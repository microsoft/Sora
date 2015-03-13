#pragma once
#include "brick.h"
#include "autocorr.hpp"

DEFINE_LOCAL_CONTEXT(TCCA11n, CF_Error, CF_11CCA, CF_TimeStamps);
template< TSINK_ARGS >
class TCCA11n : public TSink<TSINK_PARAMS>
{
	CTX_VAR_RW (uint, 				                cca_pwr_reading );
	CTX_VAR_RW(CF_11CCA::CCAState,                  cca_state );
    CTX_VAR_RW(ulong,                               error_code );
    CTX_VAR_RW (SoraPerformanceCounter,             decoding_data_stopwatch);
public:
    DEFINE_IPORT(COMPLEX16, 4, 2);
    REFERENCE_LOCAL_CONTEXT(TCCA11n);
    STD_TSINK_CONSTRUCTOR(TCCA11n)
		BIND_CONTEXT(CF_11CCA::cca_pwr_reading,     cca_pwr_reading)
		BIND_CONTEXT(CF_11CCA::cca_state,		    cca_state)
        BIND_CONTEXT(CF_Error::error_code,          error_code)
        BIND_CONTEXT (CF_TimeStamps::decoding_data_stopwatch, decoding_data_stopwatch)
    { _init(); }
    STD_TSINK_RESET() { _reset(); }
    STD_TSINK_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        vcs &vInput1 = *(vcs *)ipin.peek(0);
        vcs &vInput2 = *(vcs *)ipin.peek(1);// don't use data from port 2 in this impl.

        vq vop0[2];// output auto correlation sqr energy
        vq vop1[2];// output average sqr energy

        core.CalcAutoCorrAndEnergy(vInput1, vInput2, vop0, vop1);

        __int64 *acorr = &vop0[0][0];
        __int64 *energy = &vop1[0][0];
        size_t   _nin  = 4;

#if 0
        for (size_t i = 0; i < _nin; i++)
        {
            printf("%I64d, %I64d\n", acorr[i], energy[i]);
        }
#endif

        for (size_t i = 0; i < _nin; i++)
        {
            itracept++;

            __int64 eb = energy[i] / (his_moving_energy[his_index] + 1);
            int eb32 = (int)(eb);

            // for debug only, make it easy to be seen on screen
            eb32 *= 10;

            if (!peak_found)
            {
                sense_count += 1;

                if ( eb > 5 && acorr[i] > (energy[i] >> 1)/*  && (e > 200000)*/ )
                {
					// any case we are sensing high, we reset the sense counter
					sense_count = 0;

                    peak_count++;
                    //if ( peak_count > 3)
                    {
                        float fsnr = 5 * log10((float)eb) - 1.0f;
                        //printf(" Frame detected, LSTF, SNR %.3f dB\n", fsnr);
                        //printf("peak-->%d\n", itracept);
                        //cout << "peak ->" << endl;

                        peak_found = true;
                    }
                }
                else
                {
                    peak_count = 0;
                }
            }
            else
            {
                //log("%I64d\t%I64d\n", _ip0[i], _ip1[i]);
                if ( acorr[i] < (energy[i] >> 3) )
                {
                    if (peak_count > 96 && peak_count < 160)
                    {
                        peak_found = false;
                        peak_count = 0;
        				cca_pwr_reading = uint(his_moving_energy[his_index] >> 32);
                        cca_state = CF_11CCA::power_detected;
                        decoding_data_stopwatch.Restart();
						RaiseEvent(OnPowerDetected)();

                        //cout << "peak <-" << endl;
                        //getchar();
                        break;
                    }
                    else
                    {
                        // it's a fake preamble
                        peak_found = false;
                        peak_count = 0;
                        //break;
                    }
                }
                else
                {
                    peak_count++;
                    if ( peak_count > 160 )
                    {
                        //log("Too many peaks..................\n");
                        //getchar();
                        peak_found = false;
                        peak_count = 0;
                    }
                }
            }
            his_moving_energy[his_index++] = energy[i];
            his_index %= 64;
        }

		// now we have processed all data inqueue
		if ( sense_count >= max_sense_count && cca_state == CF_11CCA::power_clear ) {
			// carrier sense timeout
			error_code = E_ERROR_CS_TIMEOUT;
		}

        ipin.clear ();
        return true;
    }

private:
    MimoAutoCorr core;

    // carrier sensing slot timing
    static const uint max_sense_count = 84; // 84/20 = 4.2 us

    int peak_up_shift;
    int peak_down_shift;
    bool peak_found;
    int peak_count;


    __int64 his_moving_energy[64];
    int his_index;

    int itracept;

	// counter for the number of samples sensed for carrier sense timeout
	uint sense_count;

    FINL void _init()
    {
        peak_up_shift = 1;
        peak_down_shift = 3;

        for (int i = 0; i < 64; i++) his_moving_energy[i] = LLONG_MAX; // Prevent initial fake trigger
        his_index = 0;
        itracept = 0;

        _reset();
    }

    FINL void _reset()
    {
        sense_count = 0;
        peak_found = false;
        peak_count = 0;
    }
};
