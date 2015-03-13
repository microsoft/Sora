#pragma once
#include "brick.h"
#include "vector128.h"

class MimoAutoCorr
{
public:
    MimoAutoCorr()
    {
        unsigned long ns = 0;
        _BitScanReverse(&ns, (vHisLength * 4));
        vShift = ns;

        //printf("normalize shift %d\n", vShift);
        vHisSample  = (vcs*)_aligned_malloc(vHisLength * sizeof(vcs), 16);
        vHisCorr    = (vci*)_aligned_malloc(vHisLength * sizeof(vci) * 2, 16);
        vHisEnergy  = (vi*)_aligned_malloc(vHisLength * sizeof(vi), 16);
        vHisSample2 = (vcs*)_aligned_malloc(vHisLength * sizeof(vcs), 16);
        vHisCorr2   = (vci*)_aligned_malloc(vHisLength * sizeof(vci) * 2, 16);
        vHisEnergy2 = (vi*)_aligned_malloc(vHisLength * sizeof(vi), 16);

        vHisIdx = 0;
        memset(vHisSample, 0, vHisLength * sizeof(vcs));
        memset(vHisCorr, 0, vHisLength * sizeof(vci) * 2);
        memset(vHisEnergy, 0, vHisLength * sizeof(vi));
        memset(vHisSample2, 0, vHisLength * sizeof(vcs));
        memset(vHisCorr2, 0, vHisLength * sizeof(vci) * 2);
        memset(vHisEnergy2, 0, vHisLength * sizeof(vi));

        set_zero(vAverageEnergySum);
        set_zero(vAverageCorrSum[0]);
        set_zero(vAverageCorrSum[1]);

        set_zero(vAverageEnergySum2);
        set_zero(vAverageCorrSum2[0]);
        set_zero(vAverageCorrSum2[1]);
    }

    ~MimoAutoCorr()
    {
        _aligned_free(vHisSample);
        _aligned_free(vHisCorr);
        _aligned_free(vHisEnergy);
        _aligned_free(vHisSample2);
        _aligned_free(vHisCorr2);
        _aligned_free(vHisEnergy2);
    }
    
    void CalcAutoCorrAndEnergy(vcs& in0, vcs& in1, vq autoCorrSqrEnergy[2], vq avgSqrEnergy[2])
    {
        // joint auto correlation
        auto_corr(in0, vAverageCorrSum[0], vAverageCorrSum[1], vHisSample, vHisCorr);
        auto_corr(in1, vAverageCorrSum2[0], vAverageCorrSum2[1], vHisSample2, vHisCorr2);

        // we get 4 auto correlation results
        autoCorrSqrEnergy[0] = SquaredNorm(add(shift_right(vAverageCorrSum[0], 1), 
            shift_right(vAverageCorrSum2[0], 1)));
        autoCorrSqrEnergy[1] = SquaredNorm(add(shift_right(vAverageCorrSum[1], 1), 
            shift_right(vAverageCorrSum2[1], 1)));

        vAverageCorrSum[0]  = permutate<1, 1>(vAverageCorrSum[1]);
        vAverageCorrSum2[0] = permutate<1, 1>(vAverageCorrSum2[1]);

        // calculate moving energy now
        vi ve1, ve2, vesum;
        moving_energy(in0, vAverageEnergySum, vHisEnergy, ve1);
        moving_energy(in1, vAverageEnergySum2, vHisEnergy2, ve2);

        vesum = add(shift_right(ve1, 1), shift_right(ve2, 1));
        mul(avgSqrEnergy[0], avgSqrEnergy[1], vesum, vesum);

        vHisIdx++;
        vHisIdx %= vHisLength;
    }

private:
    static const int vHisLength = 8;
    vi  vInputEnergy;
    vci vInputCorr[2];

    vi  vAverageEnergySum;
    vci vAverageCorrSum[2];

    vi  vAverageEnergySum2;
    vci vAverageCorrSum2[2];

    vcs* vHisSample;
    vci* vHisCorr;
    vi*  vHisEnergy;

    vcs* vHisSample2;
    vci* vHisCorr2;
    vi*  vHisEnergy2;




    int vHisIdx;
    int vShift;

    __forceinline void auto_corr(vcs& vInput, vci& vCorrSum1, vci& vCorrSum2, vcs* pvHisSample, vci* pvHisCorr)
    {
        //printf("msk %p\n", &vMulMask);
        conj_mul(vInputCorr[0], vInputCorr[1], vInput, pvHisSample[vHisIdx]);
        vInputCorr[0] = shift_right(vInputCorr[0], vShift);
        vInputCorr[1] = shift_right(vInputCorr[1], vShift);

        pvHisSample[vHisIdx] = vInput;

        vci vDeltaCorr[3];

        vDeltaCorr[0] = sub(vInputCorr[0], pvHisCorr[2 * (vHisIdx)]);
        vDeltaCorr[1] = sub(vInputCorr[1], pvHisCorr[2 * (vHisIdx) + 1]);

        pvHisCorr[2 * (vHisIdx)]     = vInputCorr[0];
        pvHisCorr[2 * (vHisIdx) + 1] = vInputCorr[1];

        vDeltaCorr[2] = concat_extract<8>(vDeltaCorr[1], vDeltaCorr[0]);

        vDeltaCorr[0] = add(vDeltaCorr[0], shift_element_left<1>(vDeltaCorr[0]));

        vCorrSum1 = add(vCorrSum1, vDeltaCorr[0]);
        vCorrSum2 = add(vCorrSum1, vDeltaCorr[1]);
        vCorrSum2 = add(vCorrSum2, vDeltaCorr[2]);
    }

    __forceinline void moving_energy(vcs& vInput, vi& vAvgEnergySum, vi* pvHisEnergy, vi& ve)
    {
        vInputEnergy = SquaredNorm(vInput);
        vInputEnergy = shift_right(vInputEnergy, vShift);

        vi vDeltaEnergy;
        vDeltaEnergy = sub(vInputEnergy, pvHisEnergy[vHisIdx]);

        pvHisEnergy[vHisIdx] = vInputEnergy;

        vDeltaEnergy = add(vDeltaEnergy, shift_element_left<1>(vDeltaEnergy));
        vDeltaEnergy = add(vDeltaEnergy, shift_element_left<2>(vDeltaEnergy));

        vAvgEnergySum  = add(vAvgEnergySum, vDeltaEnergy);

        ve = vAvgEnergySum;
        //vAvgEnergySum.v_sqr2q(vout1, vout2);

        vAvgEnergySum = permutate<3, 3, 3, 3>(vAvgEnergySum);
    }
};

DEFINE_LOCAL_CONTEXT(TAutoCorr, CF_VOID);
template<TFILTER_ARGS>
class TAutoCorr: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(COMPLEX16, 4, 2);
    DEFINE_OPORT(__int64, 4, 2);

    REFERENCE_LOCAL_CONTEXT(TAutoCorr);
    STD_TFILTER_CONSTRUCTOR(TAutoCorr) { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs &vInput1 = *(vcs *)ipin.peek(0);
            vcs &vInput2 = *(vcs *)ipin.peek(1);// don't use data from port 2 in this impl.

            vq *vop0 = (vq *)opin().write(0);// output auto correlation sqr energy
            vq *vop1 = (vq *)opin().write(1);// output average sqr energy

            core.CalcAutoCorrAndEnergy(vInput1, vInput2, vop0, vop1);
            opin().append();
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    MimoAutoCorr core;
};
