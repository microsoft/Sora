#pragma once
#include "brick.h"
#include "vector128.h"
#include "ieee80211facade.hpp"

__declspec(selectany) A16 vui::data_type _80211_LLTFMask[16] =
{
    //0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1,
    { 0x0000FFFF, 0xFFFF0000, 0x0000FFFF, 0x0000FFFF },
    { 0xFFFF0000, 0xFFFF0000, 0x0000FFFF, 0xFFFF0000 },
    { 0x0000FFFF, 0xFFFF0000, 0x0000FFFF, 0x0000FFFF },
    { 0x0000FFFF, 0x0000FFFF, 0x0000FFFF, 0xFFFF0000 },

    //1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    { 0xFFFF0000, 0x0000FFFF, 0x0000FFFF, 0xFFFF0000 },
    { 0x0000FFFF, 0xFFFF0000, 0x0000FFFF, 0xFFFF0000 },
    { 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0x0000FFFF },
    { 0x0000FFFF, 0x0000FFFF, 0x0000FFFF, 0x0000FFFF },

    //0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1,
    { 0x0000FFFF, 0x0000FFFF, 0x0000FFFF, 0x0000FFFF },
    { 0x0000FFFF, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000 },
    { 0x0000FFFF, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000 },
    { 0x0000FFFF, 0xFFFF0000, 0x0000FFFF, 0xFFFF0000 },

    //1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1
    { 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000 },
    { 0xFFFF0000, 0x0000FFFF, 0x0000FFFF, 0xFFFF0000 },
    { 0xFFFF0000, 0x0000FFFF, 0xFFFF0000, 0x0000FFFF },
    { 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000 }
};

__forceinline void v_siso_channel_estimation_64(__in vcs* pcsin, __in vcs* pvmask, __out vcs* pcschannel)
{
    vi  vsqr, vsqrtemp;
    vi  vciinput1, vciinput2;
    vcs vout;  
    int  v;
    int  i;

    for (i = 0; i < 7; i++)
    {
        vcs& vin = (vcs&)pcsin[i];
        int sqr;
        vsqr = SquaredNorm(vin);

        unpack(vciinput1, vciinput2, vin);

        vciinput1 = shift_left(vciinput1, 16);
        vciinput2 = shift_left(vciinput2, 16);

        vsqrtemp = shift_right(vsqr, 1);

        vciinput1 = add(vciinput1, vsqrtemp);
        vciinput2 = add(vciinput2, vsqrtemp);

        sqr = extract<0>(vsqr);
        if (sqr == 0) sqr = 1;
        v = extract<0>(vciinput1) / sqr;
        vciinput1 = insert<0>( vciinput1, v );

        v = extract<1>(vciinput1) / sqr;
        vciinput1 = insert<1>( vciinput1, v );

        sqr = extract<1>(vsqr);
        if (sqr == 0) sqr = 1;
        v = extract<2>(vciinput1) / sqr;
        vciinput1 = insert<2>( vciinput1, v );

        v = extract<3>(vciinput1) / sqr;    
        vciinput1 = insert<3>( vciinput1, v );

        sqr = extract<2>(vsqr);
        if (sqr == 0) sqr = 1;
        v = extract<0>(vciinput2) / sqr;
        vciinput2 = insert<0>( vciinput2, v );

        v = extract<1>(vciinput2) / sqr;
        vciinput2 = insert<1>( vciinput2, v );

        sqr = extract<3>(vsqr);
        if (sqr == 0) sqr = 1;
        v = extract<2>(vciinput2) / sqr;
        vciinput2 = insert<2>( vciinput2, v );

        v = extract<3>(vciinput2) / sqr;
        vciinput2 = insert<3>( vciinput2, v );

        vout = saturated_pack((vci&)vciinput1, (vci&)vciinput2);

        vout = xor(vout, pvmask[i]);
        vout = sub(vout, pvmask[i]);

        pcschannel[i] = vout;
    }

    for (i = 9; i < 16; i++)
    {
        vcs& vin = (vcs&)pcsin[i];
        int sqr;
        vsqr = SquaredNorm(vin);

        unpack(vciinput1, vciinput2, vin);

        vciinput1 = shift_left(vciinput1, 16);
        vciinput2 = shift_left(vciinput2, 16);

        vsqrtemp = shift_right(vsqr, 1);

        vciinput1 = add(vciinput1, vsqrtemp);
        vciinput2 = add(vciinput2, vsqrtemp);

        sqr = extract<0>(vsqr);
        if (sqr == 0) sqr = 1;
        v = extract<0>(vciinput1) / sqr;
        vciinput1 = insert<0>( vciinput1, v );

        v = extract<1>(vciinput1) / sqr;
        vciinput1 = insert<1>( vciinput1, v );

        sqr = extract<1>(vsqr);
        if (sqr == 0) sqr = 1;
        v = extract<2>(vciinput1) / sqr;
        vciinput1 = insert<2>( vciinput1, v );

        v = extract<3>(vciinput1) / sqr;
        vciinput1 = insert<3>( vciinput1, v );

        sqr = extract<2>(vsqr);
        if (sqr == 0) sqr = 1;
        v = extract<0>(vciinput2) / sqr;
        vciinput2 = insert<0>( vciinput2, v );

        v = extract<1>(vciinput2) / sqr;
        vciinput2 = insert<1>( vciinput2, v );

        sqr = extract<3>(vsqr);
        if (sqr == 0) sqr = 1;
        v = extract<2>(vciinput2) / sqr;
        vciinput2 = insert<2>( vciinput2, v );

        v = extract<3>(vciinput2) / sqr;
        vciinput2 = insert<3>( vciinput2, v );

        vout = saturated_pack((vci&)vciinput1, (vci&)vciinput2);

        vout = xor(vout, pvmask[i]);
        vout = sub(vout, pvmask[i]);

        pcschannel[i] = vout;
    }
}

DEFINE_LOCAL_CONTEXT(TSisoChannelEst, CF_Channel_11n, CF_11nSymState);
template<TSINK_ARGS>
class TSisoChannelEst: public TSink<TSINK_PARAMS>
{
    CTX_VAR_RW(Dot11aChannelCoefficient, dot11a_siso_channel_1);
    CTX_VAR_RW(Dot11aChannelCoefficient, dot11a_siso_channel_2);
    CTX_VAR_RW(ulong,  symbol_type );

public:
    const static int BURST = 32 * vcs::size;
    DEFINE_IPORT(COMPLEX16, BURST, 2);

public:
    REFERENCE_LOCAL_CONTEXT(TSisoChannelEst);
    STD_TSINK_CONSTRUCTOR(TSisoChannelEst)
        BIND_CONTEXT(CF_Channel_11n::dot11a_siso_channel_1, dot11a_siso_channel_1)
        BIND_CONTEXT(CF_Channel_11n::dot11a_siso_channel_2, dot11a_siso_channel_2)
        BIND_CONTEXT(CF_11nSymState::symbol_type,  symbol_type )
    { }
    STD_TSINK_RESET() { }
    STD_TSINK_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *ip1 = (vcs *)ipin.peek(0);
            vcs *ip2 = (vcs *)ipin.peek(1);

            Dot11aChannelCoefficient& ch1  = dot11a_siso_channel_1;
            vcs *pvmask = (vcs*)&_80211_LLTFMask[0];

            v_siso_channel_estimation_64(ip1, pvmask, (vcs*)&siso_channel_1[0]);
            v_siso_channel_estimation_64(ip1 + 16, pvmask, (vcs*)&siso_channel_2[0]);

            for (int i = 0; i < 16; i++)
            {
                vcs r = add(siso_channel_1[i], siso_channel_2[i]);
                ch1[i]  = shift_right(r, 1);
            }


            Dot11aChannelCoefficient& ch2  = dot11a_siso_channel_2;

            v_siso_channel_estimation_64(ip2, pvmask, (vcs*)&siso_channel_1[0]);
            v_siso_channel_estimation_64(ip2 + 16, pvmask, (vcs*)&siso_channel_2[0]);
            for (int i = 0; i < 16; i++)
            {
                vcs r = add(siso_channel_1[i], siso_channel_2[i]);
                ch2[i]  = shift_right(r, 1);
            }


            ipin.pop();

            symbol_type = CF_11nSymState::SYMBOL_SIG;
        }
        return true;
    }

private:
    Dot11aChannelCoefficient siso_channel_1;
    Dot11aChannelCoefficient siso_channel_2;
};

DEFINE_LOCAL_CONTEXT(TSisoChannelComp, CF_Channel_11n);
template<TFILTER_ARGS>
class TSisoChannelComp: public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW(Dot11aChannelCoefficient, dot11a_siso_channel_1);
    CTX_VAR_RW(Dot11aChannelCoefficient, dot11a_siso_channel_2);

public:
    const static int BURST = 16 * vcs::size;
    DEFINE_IPORT(COMPLEX16, BURST, 2);
    DEFINE_OPORT(COMPLEX16, BURST, 2);

public:
    REFERENCE_LOCAL_CONTEXT(TSisoChannelComp);
    STD_TFILTER_CONSTRUCTOR(TSisoChannelComp)
        BIND_CONTEXT(CF_Channel_11n::dot11a_siso_channel_1, dot11a_siso_channel_1)
        BIND_CONTEXT(CF_Channel_11n::dot11a_siso_channel_2, dot11a_siso_channel_2)
    { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *ip1 = (vcs *)ipin.peek(0);
            vcs *ip2 = (vcs *)ipin.peek(1);
            vcs *op1 = (vcs *)opin().write(0);
            vcs *op2 = (vcs *)opin().write(1);

#if 0
            for (int i = 0; i < 16; i++)
            {
                op1[i] = ip1[i];
                op2[i] = ip2[i];
            }
#else
            vci vciout1, vciout2;

            Dot11aChannelCoefficient& channel_factor1 = dot11a_siso_channel_1;

            for (int i = 0; i < 16; i++)
            {
                vcs &vin   = (vcs &)ip1[i];
                vcs &vcof  = (vcs &)channel_factor1[i];
                vcs &vout  = (vcs &)op1[i];

                mul(vciout1, vciout2, vin, vcof);

                vciout1 = shift_right(vciout1, 9);
                vciout2 = shift_right(vciout2, 9);

                vout        = saturated_pack(vciout1, vciout2);
            }

            Dot11aChannelCoefficient& channel_factor2 = dot11a_siso_channel_2;

            for (int i = 0; i < 16; i++)
            {
                vcs &vin   = (vcs &)ip2[i];
                vcs &vcof  = (vcs &)channel_factor2[i];
                vcs &vout  = (vcs &)op2[i];

                mul(vciout1, vciout2, vin, vcof);

                vciout1 = shift_right(vciout1, 9);
                vciout2 = shift_right(vciout2, 9);

                vout        = saturated_pack(vciout1, vciout2);
            }
#endif
            ipin.pop();
            opin().append();
            Next()->Process(opin());
        }
        return true;
    }
};

#pragma region dot11n HT LTF Mask
SELECTANY A16 vui::data_type _80211n_HTLTFMask[16] =
{
    //0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1,
    { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF },
    { 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 },
    { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF },
    { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 },

    //1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    { 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 },
    { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000 },
    { 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF },
    { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },

    //0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1,
    { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
    { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 },
    { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000 },

    //1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
    { 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 },
    { 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF },
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000 }
};
#pragma endregion dot11n HT LTF Mask

__forceinline vci v_convert2ci_lo(vcq &a, vcq &b){ vci c; c = (vci&)_mm_shuffle_ps((__m128&)a, (__m128&)b, _MM_SHUFFLE(2, 0, 2, 0)); return c; }


DEFINE_LOCAL_CONTEXT(TMimoChannelEst, CF_ChannelMimo, CF_11nSymState);
template<TSINK_ARGS>
class TMimoChannelEst: public TSink<TSINK_PARAMS>
{
    CTX_VAR_RW(MIMO_2x2_H, dot11n_2x2_channel_inv);
    CTX_VAR_RW(MIMO_2x2_H, dot11n_2x2_channel);
    CTX_VAR_RW(ulong,  symbol_type );

public:
    const static int BURST = 32 * vcs::size;
    DEFINE_IPORT(COMPLEX16, BURST, 2);

public:
    REFERENCE_LOCAL_CONTEXT(TMimoChannelEst);
    STD_TSINK_CONSTRUCTOR(TMimoChannelEst)
        BIND_CONTEXT(CF_ChannelMimo::dot11n_2x2_channel_inv, dot11n_2x2_channel_inv)
        BIND_CONTEXT(CF_ChannelMimo::dot11n_2x2_channel,     dot11n_2x2_channel)
        BIND_CONTEXT(CF_11nSymState::symbol_type,            symbol_type )
    { }
    STD_TSINK_RESET() { }
    STD_TSINK_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vci vtemp[4];

            vci vstar[2];
            vq  vstarsqr[2];

            vci vcih1, vcih2;

            vcs vNegMask;
            set_all_bits(vNegMask);

            const COMPLEX16* pHTLTFMask = (const COMPLEX16*)&_80211n_HTLTFMask[0][0];

            COMPLEX16 *ipc1 = ipin.peek(0);
            COMPLEX16 *ipc2 = ipin.peek(1);

            MIMO_2x2_H& mimo_channel_2x2_inv = dot11n_2x2_channel_inv;

            int i;

            for (i = 0; i < 64; i += 4)
            {
                vcs& vhtmsk = (vcs&)pHTLTFMask[i];

                vcs& v11    = (vcs&)ipc1[i];
                vcs& v12    = (vcs&)ipc1[i + 64];
                vcs& v21    = (vcs&)ipc2[i];
                vcs& v22    = (vcs&)ipc2[i + 64];

                vcs& vh11   = (vcs&)dot11n_2x2_channel[0][i];
                vcs& vh12   = (vcs&)dot11n_2x2_channel[0][i + 64];
                vcs& vh21   = (vcs&)dot11n_2x2_channel[1][i];
                vcs& vh22   = (vcs&)dot11n_2x2_channel[1][i + 64];

                vcs& vtemph = (vcs&)vtemp[0];

                vtemph = saturated_sub(v11, v12);
                vtemph = shift_right(vtemph, 1);
                vtemph = xor(vtemph, vhtmsk);
                vh11   = sub(vtemph, vhtmsk);

                vtemph = saturated_add(v11, v12); 
                vtemph = shift_right(vtemph, 1);
                vtemph = xor(vtemph, vhtmsk);
                vh12   = sub(vtemph, vhtmsk);

                vtemph = saturated_sub(v21, v22);
                vtemph = shift_right(vtemph, 1);
                vtemph = xor(vtemph, vhtmsk);
                vh21   = sub(vtemph, vhtmsk);

                vtemph = saturated_add(v21, v22); 
                vtemph = shift_right(vtemph, 1);
                vtemph = xor(vtemph, vhtmsk);
                vh22   = sub(vtemph, vhtmsk);
            }

            for (i = 0; i < 64; i += 4)
            {
                vcs& vh11   = (vcs&)dot11n_2x2_channel[0][i];
                vcs& vh12   = (vcs&)dot11n_2x2_channel[0][i + 64];
                vcs& vh21   = (vcs&)dot11n_2x2_channel[1][i];
                vcs& vh22   = (vcs&)dot11n_2x2_channel[1][i + 64];

                mul(vtemp[0], vtemp[1], vh11, vh22);
                mul(vtemp[2], vtemp[3], vh12, vh21);

                vstar[0] = sub(vtemp[0], vtemp[2]);
                vstar[1] = sub(vtemp[1], vtemp[3]);

                vstarsqr[0] = SquaredNorm(vstar[0]);
                vstarsqr[1] = SquaredNorm(vstar[1]);

                // Add 1 to avoid divided by 0 exception
                vstarsqr[0] = sub(vstarsqr[0], (vq&)vNegMask);
                vstarsqr[1] = sub(vstarsqr[1], (vq&)vNegMask);
                //
                vcs &vinvh11 = (vcs&)mimo_channel_2x2_inv[0][i];
                vcs &vinvh12 = (vcs&)mimo_channel_2x2_inv[0][i + 64];
                vcs &vinvh21 = (vcs&)mimo_channel_2x2_inv[1][i];
                vcs &vinvh22 = (vcs&)mimo_channel_2x2_inv[1][i + 64];

                vcq &vres1 = (vcq&)vtemp[0];
                vcq &vres2 = (vcq&)vtemp[1];
                vcq &vres3 = (vcq&)vtemp[2];
                vcq &vres4 = (vcq&)vtemp[3];

                // - invh11
                unpack(vcih1, vcih2, vh22);

                conj_mul(vres1, vres2, vcih1, vstar[0]);
                conj_mul(vres3, vres4, vcih2, vstar[1]);

                vres1 = shift_left(vres1, 16);
                vres2 = shift_left(vres2, 16);
                vres3 = shift_left(vres3, 16);
                vres4 = shift_left(vres4, 16);

                vres1[0].re /= vstarsqr[0][0]; vres1[0].im /= vstarsqr[0][0];
                vres2[0].re /= vstarsqr[0][1]; vres2[0].im /= vstarsqr[0][1];
                vres3[0].re /= vstarsqr[1][0]; vres3[0].im /= vstarsqr[1][0];
                vres4[0].re /= vstarsqr[1][1]; vres4[0].im /= vstarsqr[1][1];

                ((vci&)vres1) = v_convert2ci_lo(vres1, vres2);
                ((vci&)vres3) = v_convert2ci_lo(vres3, vres4);

                vcs &vout11 = (vcs&)vres2;
                vout11       = saturated_pack((vci&)vres1, (vci&)vres3);
                vinvh11      = vout11;

                // - invh12
                unpack(vcih1, vcih2, vh12);

                conj_mul(vres1, vres2, vcih1, vstar[0]);
                conj_mul(vres3, vres4, vcih2, vstar[1]);

                vres1 = shift_left(vres1, 16);
                vres2 = shift_left(vres2, 16);
                vres3 = shift_left(vres3, 16);
                vres4 = shift_left(vres4, 16);

                vres1[0].re /= vstarsqr[0][0]; vres1[0].im /= vstarsqr[0][0];
                vres2[0].re /= vstarsqr[0][1]; vres2[0].im /= vstarsqr[0][1];
                vres3[0].re /= vstarsqr[1][0]; vres3[0].im /= vstarsqr[1][0];
                vres4[0].re /= vstarsqr[1][1]; vres4[0].im /= vstarsqr[1][1];

                ((vci&)vres1) = v_convert2ci_lo(vres1, vres2);
                ((vci&)vres3) = v_convert2ci_lo(vres3, vres4);

                vcs &vout12 = (vcs&)vres2;
                vout12       = saturated_pack((vci&)vres1, (vci&)vres3);
                vout12       = xor(vout12, vNegMask);
                vout12       = sub((vcs&)vout12, (vcs&)vNegMask);
                vinvh12      = vout12;

                // - invh21
                unpack(vcih1, vcih2, vh21);

                conj_mul(vres1, vres2, vcih1, vstar[0]);
                conj_mul(vres3, vres4, vcih2, vstar[1]);

                vres1 = shift_left(vres1, 16);
                vres2 = shift_left(vres2, 16);
                vres3 = shift_left(vres3, 16);
                vres4 = shift_left(vres4, 16);

                vres1[0].re /= vstarsqr[0][0]; vres1[0].im /= vstarsqr[0][0];
                vres2[0].re /= vstarsqr[0][1]; vres2[0].im /= vstarsqr[0][1];
                vres3[0].re /= vstarsqr[1][0]; vres3[0].im /= vstarsqr[1][0];
                vres4[0].re /= vstarsqr[1][1]; vres4[0].im /= vstarsqr[1][1];

                ((vci&)vres1) = v_convert2ci_lo(vres1, vres2);
                ((vci&)vres3) = v_convert2ci_lo(vres3, vres4);

                vcs &vout21 = (vcs&)vres2;
                vout21       = saturated_pack((vci&)vres1, (vci&)vres3);
                vout21       = xor(vout21, vNegMask);
                vout21       = sub((vcs&)vout21, (vcs&)vNegMask);
                vinvh21      = vout21;

                // - invh22
                unpack(vcih1, vcih2, vh11);

                conj_mul(vres1, vres2, vcih1, vstar[0]);
                conj_mul(vres3, vres4, vcih2, vstar[1]);

                vres1 = shift_left(vres1, 16);
                vres2 = shift_left(vres2, 16);
                vres3 = shift_left(vres3, 16);
                vres4 = shift_left(vres4, 16);

                vres1[0].re /= vstarsqr[0][0]; vres1[0].im /= vstarsqr[0][0];
                vres2[0].re /= vstarsqr[0][1]; vres2[0].im /= vstarsqr[0][1];
                vres3[0].re /= vstarsqr[1][0]; vres3[0].im /= vstarsqr[1][0];
                vres4[0].re /= vstarsqr[1][1]; vres4[0].im /= vstarsqr[1][1];

                ((vci&)vres1) = v_convert2ci_lo(vres1, vres2);
                ((vci&)vres3) = v_convert2ci_lo(vres3, vres4);

                vcs &vout22 = (vcs&)vres2;
                vout22       = saturated_pack((vci&)vres1, (vci&)vres3);
                vinvh22      = vout22;
            }
            ipin.pop();
            symbol_type = CF_11nSymState::SYMBOL_OFDM_DATA;
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(TMimoChannelComp, CF_ChannelMimo);
template<TFILTER_ARGS>
class TMimoChannelComp: public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RO(MIMO_2x2_H, dot11n_2x2_channel_inv);

public:
    const static int BURST = 16 * vcs::size;
    DEFINE_IPORT(COMPLEX16, BURST, 2);
    DEFINE_OPORT(COMPLEX16, BURST, 2);

public:
    REFERENCE_LOCAL_CONTEXT(TMimoChannelComp);
    STD_TFILTER_CONSTRUCTOR(TMimoChannelComp)
        BIND_CONTEXT(CF_ChannelMimo::dot11n_2x2_channel_inv, dot11n_2x2_channel_inv)
    { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            COMPLEX16 *ipc1 = ipin.peek(0);
            COMPLEX16 *ipc2 = ipin.peek(1);
            COMPLEX16 *opc1 = opin().write(0);
            COMPLEX16 *opc2 = opin().write(1);

            const MIMO_2x2_H& mimo_channel_2x2 = dot11n_2x2_channel_inv;

            vci vcomp1[2], vcomp2[2];

            for (int i = 0; i < 64; i += 4)
            {
                const vcs &vinvh11 = (const vcs&)mimo_channel_2x2[0][i];
                const vcs &vinvh12 = (const vcs&)mimo_channel_2x2[0][i + 64];
                const vcs &vinvh21 = (const vcs&)mimo_channel_2x2[1][i];
                const vcs &vinvh22 = (const vcs&)mimo_channel_2x2[1][i + 64];
                const vcs &y1      = (const vcs&)ipc1[i];
                const vcs &y2      = (const vcs&)ipc2[i];
                vcs& x1      = (vcs&)opc1[i];
                vcs& x2      = (vcs&)opc2[i];

                mul(vcomp1[0], vcomp1[1], vinvh11, y1);
                mul(vcomp2[0], vcomp2[1], vinvh12, y2);

                vcomp1[0] = add(vcomp1[0], vcomp2[0]);
                vcomp1[1] = add(vcomp1[1], vcomp2[1]);

                vcomp1[0] = shift_right(vcomp1[0], 9);
                vcomp1[1] = shift_right(vcomp1[1], 9);

                x1 = saturated_pack(vcomp1[0], vcomp1[1]);

                mul(vcomp1[0], vcomp1[1], vinvh21, y1);
                mul(vcomp2[0], vcomp2[1], vinvh22, y2);

                vcomp1[0] = add(vcomp1[0], vcomp2[0]);
                vcomp1[1] = add(vcomp1[1], vcomp2[1]);

                vcomp1[0] = shift_right(vcomp1[0], 9);
                vcomp1[1] = shift_right(vcomp1[1], 9);

                x2 = saturated_pack(vcomp1[0], vcomp1[1]);
            }

#if enable_dbgplot
            PlotDots("RX 1", opc1, 64);
            PlotDots("RX 2", opc2, 64);
#endif

            ipin.pop();
            opin().append();
            Next()->Process(opin());
        }
        return true;
    }
};
