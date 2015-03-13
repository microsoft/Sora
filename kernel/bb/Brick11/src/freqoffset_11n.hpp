#pragma once
#include "brick.h"
#include "vector128.h"
#include "dsp_math.h"
#include "ieee80211facade.hpp"
#include "intalg.h"

__forceinline short v_estimate_i(vcs* pvcs, int vcount, int vdist)
{
    vci vsum;
    set_zero(vsum);
    unsigned long nshift = 0;
    _BitScanReverse(&nshift, vcount);
    nshift += 2; // represented in vector, need add log2(4)

    for (int i = 0; i < vcount; i++)
    {
        vcs& va = (vcs&)pvcs[i];
        vcs& vb = (vcs&)pvcs[i + vdist];
        vci vc;
        vci vd;

        conj_mul(vc, vd, va, vb);
        vc = shift_right(vc, nshift);
        vd = shift_right(vd, nshift);
        vsum = add(vc, vsum);
        vsum = add(vd, vsum);
    }
    vsum = hadd(vsum);

    //printf(" vsum =[%d, %d]\n", vsum[0].re, vsum[0].im);

    static const dsp_math& dm = dsp_math::singleton();
    short sdeltaf = dm.atan(vsum[0].re, vsum[0].im);
    _BitScanReverse(&nshift, vdist);
    nshift += 2;
    sdeltaf >>= nshift;

    return sdeltaf;
}

__forceinline short v_estimate_i(vcs* pvcs1, vcs* pvcs2, int vcount, int vdist)
{
    vci vsum;
    set_zero(vsum);
    unsigned long nshift = 0;
    _BitScanReverse(&nshift, vcount);
    nshift += 3; // represented in vector, need add log2(4) + 1

    for (int i = 0; i < vcount; i++)
    {
        vcs& va = (vcs&)pvcs1[i];
        vcs& vb = (vcs&)pvcs1[i + vdist];

        vcs& vaa = (vcs&)pvcs2[i];
        vcs& vbb = (vcs&)pvcs2[i + vdist];

        vci vc;
        vci vd;

        conj_mul(vc, vd, va, vb);
        vc = shift_right(vc, nshift);
        vd = shift_right(vd, nshift);
        vsum = add(vc, vsum);
        vsum = add(vd, vsum);

        conj_mul(vc, vd, vaa, vbb);
        vc = shift_right(vc, nshift);
        vd = shift_right(vd, nshift);
        vsum = add(vc, vsum);
        vsum = add(vd, vsum);
    }
    vsum = hadd(vsum);

    //printf(" vsum =[%d, %d]\n", vsum[0].re, vsum[0].im);

    static const dsp_math& dm = dsp_math::singleton();
    short sdeltaf = dm.atan(vsum[0].re, vsum[0].im);
    _BitScanReverse(&nshift, vdist);
    nshift += 2;
    sdeltaf >>= nshift;

    return sdeltaf;
}

DEFINE_LOCAL_CONTEXT(TFreqEstimator_11n, CF_FreqOffset_11n, CF_CFOffset);
template<TFILTER_ARGS>
class TFreqEstimator_11n: public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW(vs, vfo_delta_i);
    CTX_VAR_RW(vs, vfo_step_i);
    CTX_VAR_RW(vs, vfo_theta_i);
    CTX_VAR_RW(FP_RAD, CFO_est);

public:
    const static int vEstimateLength =  16;// 16 vcs => 64 complex16
    const static int vEstimateDistance = 16;// 16 vcs => 64 complex16
    const static int BURST = (vEstimateLength + vEstimateDistance) * vcs::size;
    DEFINE_IPORT(COMPLEX16, BURST, 2);
    DEFINE_OPORT(COMPLEX16, BURST, 2);

public:
    REFERENCE_LOCAL_CONTEXT(TFreqEstimator_11n);
    STD_TFILTER_CONSTRUCTOR(TFreqEstimator_11n)
        BIND_CONTEXT(CF_FreqOffset_11n::vfo_delta_i, vfo_delta_i)
        BIND_CONTEXT(CF_FreqOffset_11n::vfo_step_i,  vfo_step_i)
        BIND_CONTEXT(CF_FreqOffset_11n::vfo_theta_i, vfo_theta_i)
        BIND_CONTEXT(CF_CFOffset::CFO_est,           CFO_est)
    { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *ip1 = (vcs *)ipin.peek(0);
            vcs *ip2 = (vcs *)ipin.peek(1);

#if 0
            short delta1 = v_estimate_i(ip1, vEstimateLength, vEstimateDistance);
            short delta2 = v_estimate_i(ip2, vEstimateLength, vEstimateDistance);
            short delta  = (delta1 >> 1) + (delta2 >> 1);
            //short delta = delta1;
            PlotText("[log]", "CFO1=%d, CFO2=%d, AvgCFO=%d\n", delta1, delta2, delta);
#endif

            // joint CFO estimation
            short delta = v_estimate_i(ip1, ip2, vEstimateLength, vEstimateDistance);
            CFO_est = delta;

            float fcfohz = (2.0f * 3.14f * delta / 65535.0f) * 1000.0f * (180.0f / 3.14f) / 4.0f;
            //printf(" CFO: %d, %.3f KHz\n", delta, fcfohz);
#if enable_dbgplot
            PlotText("[log]", "AvgCFO=%d  -=>  %.3f KHz\n", delta, fcfohz);
#endif

            vs vcfodelta;
            set_all(vcfodelta, delta);

            vfo_step_i    = shift_left(vcfodelta, 3);

            vcfodelta         = add(vcfodelta, shift_element_left<1>(vcfodelta));
            vcfodelta         = add(vcfodelta, shift_element_left<2>(vcfodelta));
            vcfodelta         = add(vcfodelta, shift_element_left<4>(vcfodelta));
            vcfodelta         = shift_element_left<1>(vcfodelta);

            //log("%s: cfo=%d\n", name(), vfo_delta_i[1]);

            vfo_delta_i    = vcfodelta;

            set_zero(vfo_theta_i);

            // Passthrough to downstream
            repex<COMPLEX16, BURST>::vmemcpy(opin().write(0), (const COMPLEX16 *)ip1);
            repex<COMPLEX16, BURST>::vmemcpy(opin().write(1), (const COMPLEX16 *)ip2);
            opin().append();
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};

__forceinline void v_make_coeff_i(vcs* pvcs, int vcount, vs &vdeltaf, const vs &vstep, const vs &vthetaf)
{
    static const dsp_math& dm = dsp_math::singleton();
    vs _vdeltaf = vdeltaf;
    vs vsrad;
    vcs vout;
    short v;
    int j = 0;
    for (int i = 0; i < (vcount >> 1); i++)
    {
        vsrad       = sub(_vdeltaf, vthetaf);

        v           = extract<0>(vsrad); vout = insert<0>(vout, dm.sincos( v ) );
        //assert(abs(ucos(v) - dm.sincos( v ).re) < 5);
        v           = extract<1>(vsrad); vout = insert<1>(vout, dm.sincos( v ) );
        v           = extract<2>(vsrad); vout = insert<2>(vout, dm.sincos( v ) );
        v           = extract<3>(vsrad); vout = insert<3>(vout, dm.sincos( v ) );

        pvcs[j]     = vout;

        v           = extract<4>(vsrad); vout = insert<0>(vout, dm.sincos( v ) );
        v           = extract<5>(vsrad); vout = insert<1>(vout, dm.sincos( v ) );
        v           = extract<6>(vsrad); vout = insert<2>(vout, dm.sincos( v ) );
        v           = extract<7>(vsrad); vout = insert<3>(vout, dm.sincos( v ) );

        pvcs[j + 1] = vout;

        j          += 2;
        _vdeltaf    = add(_vdeltaf, vstep);
    }
    vdeltaf = _vdeltaf;
}

__forceinline void v_compensate_i(vcs* pvcsin, vcs* pvcscoeff, vcs* pvcsout, int vcount)
{
    vci vciout1, vciout2;

    for (int i = 0; i < vcount; i++)
    {
        vcs &vin   = (vcs &)pvcsin[i];
        vcs &vcof  = (vcs &)pvcscoeff[i];

        mul(vciout1, vciout2, vin, vcof);

        vciout1 = shift_right(vciout1, 15);
        vciout2 = shift_right(vciout2, 15);

        vcs &vout  = (vcs &)pvcsout[i];
        vout        = saturated_pack(vciout1, vciout2);
    }
}

DEFINE_LOCAL_CONTEXT(TFreqComp_11n, CF_FreqOffset_11n);
template<TFILTER_ARGS>
class TFreqComp_11n: public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW(vs, vfo_delta_i);
    CTX_VAR_RO(vs, vfo_step_i);
    CTX_VAR_RO(vs, vfo_theta_i);

public:
    const static int BURST = 8;
    DEFINE_IPORT(COMPLEX16, BURST, 2);
    DEFINE_OPORT(COMPLEX16, BURST, 2);

public:
    REFERENCE_LOCAL_CONTEXT(TFreqComp_11n);
    STD_TFILTER_CONSTRUCTOR(TFreqComp_11n)
        BIND_CONTEXT(CF_FreqOffset_11n::vfo_delta_i, vfo_delta_i)
        BIND_CONTEXT(CF_FreqOffset_11n::vfo_step_i,  vfo_step_i)
        BIND_CONTEXT(CF_FreqOffset_11n::vfo_theta_i, vfo_theta_i)
    { _reset(); }
    STD_TFILTER_RESET() { _reset(); }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *ip1 = (vcs *)ipin.peek(0);
            vcs *op1 = (vcs *)opin().write(0);

            vcs *ip2 = (vcs *)ipin.peek(1);
            vcs *op2 = (vcs *)opin().write(1);

#if 0
            for (int i = 0; i < *vCompensateLength; i++)
            {
                op1[i] = ip1[i];
                op2[i] = ip2[i];
            }
#else
            //log("%s: n=%d,cfo=%d", name(), n, (*vfo_delta_i)[1]);
#endif

            v_make_coeff_i(vfo_coeff, 2, vfo_delta_i, vfo_step_i, vfo_theta_i);
            v_compensate_i(ip1, vfo_coeff, op1, 2);
            v_compensate_i(ip2, vfo_coeff, op2, 2);

            opin().append();
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
private:
    vcs vfo_coeff[2];

    FINL void _reset()
    {
        for (int i = 0; i < 2; i++)
        {
            set_zero(vfo_coeff[i]);
        }
    }
};
