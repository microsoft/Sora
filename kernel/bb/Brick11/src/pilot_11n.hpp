#pragma once
#include "brick.h"
#include "complex_ext.h"
#include "dsp_math.h"
#include "_b_dot11_pilot.h"

DEFINE_LOCAL_CONTEXT(T11nAddPilot, CF_VOID);
template<int iss>
class T11nAddPilot
{
public:
template<TFILTER_ARGS>
class Filter : public TFilter<TFILTER_PARAMS>
{
private:
    const COMPLEX16 bpsk_one;

    int pilot_index;
    dot11_ofdm_pilot _pilot_tracker;
    __int16  _pilots[4];

    void _init () {
        pilot_index = 0;	
    }
public:
    DEFINE_IPORT(COMPLEX16, 52);
    DEFINE_OPORT(COMPLEX16, 64);

public:
    REFERENCE_LOCAL_CONTEXT(Filter);
    STD_TFILTER_CONSTRUCTOR(Filter)
        , bpsk_one(complex_init<COMPLEX16>(30339, 0))
    {
        // The output symbols contain data subcarrriers, pilot subcarriers, and zero subcarriers.
        // Here we prepare the output pin queue with zero buffer, to make symbol composition easier.
        opin().zerobuf();

        _init ();
    }
    STD_TFILTER_RESET() { _init (); }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        if (!ipin.check_read()) return true;

        while (ipin.check_read())
        {
            const COMPLEX16 *in  = ipin.peek();
            COMPLEX16 *out = opin().append();

            _pilot_tracker(iss, pilot_index, _pilots);

		    ulong i;
		    for (i = 64 - 28; i < 64; i++)
		    {
			    if (i == 64 - 7 || i == 64 - 21)
				    continue;
			    out[i] = *in; in ++;
		    }
		
		    for (i = 1; i <= 28; i++)
		    {
			    if (i == 7 || i == 21)	continue;
			    out[i] = *in; in ++;
		    }

            out[64 - 21]  =  bpsk_one * _pilots[0];
            out[64 - 7]   =  bpsk_one * _pilots[1];
            out[7]         = bpsk_one * _pilots[2];
            out[21]        = bpsk_one * _pilots[3];

            pilot_index++;

            ipin.pop();
            Next()->Process(opin());
        }
        // Note: for 802.11n BPSK mapper, the input is not the whole bytes, so mapper output should be discarded at the end of each symbol
        ipin.clear();
        return true;
    }
}; };

__forceinline short dot11_pilot_tracking(COMPLEX16* pc)
{
    static const dsp_math& dm = dsp_math::singleton();
    short th1 = dm.atan(pc[64 - 21].re, pc[64 - 21].im);
    short th2 = dm.atan(pc[64 -  7].re, pc[64 - 7].im);
    short th3 = dm.atan(pc[ 7].re, pc[ 7].im);
    short th4 = dm.atan(pc[21].re, pc[21].im);

    short ThisTheta = (th1 + th2 + th3 + th4) >> 2;

    return ThisTheta;
}

DEFINE_LOCAL_CONTEXT(TPilotTrack_11n, CF_FreqOffset_11n);
template<TFILTER_ARGS>
class TPilotTrack_11n: public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW(vs, vfo_theta_i);

public:
    const static int BURST = 16 * vcs::size;
    DEFINE_IPORT(COMPLEX16, BURST, 2);
    DEFINE_OPORT(COMPLEX16, BURST, 2);

public:
    REFERENCE_LOCAL_CONTEXT(TPilotTrack_11n);
    STD_TFILTER_CONSTRUCTOR(TPilotTrack_11n)
        BIND_CONTEXT(CF_FreqOffset_11n::vfo_theta_i, vfo_theta_i)
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

            short theta1 = dot11_pilot_tracking(ipc1);
            short theta2 = dot11_pilot_tracking(ipc2);
            short theta  = ( (theta1 + theta2) >> 1 );

            vs v_theta;
            set_all(v_theta, theta);

#if 0
            v_theta.v_zero();
#endif
            vfo_theta_i = add(vfo_theta_i, v_theta);

            Next()->Process(ipin);
        }
        return true;
    }
};
