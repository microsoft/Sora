#pragma once
#include <sora.h>
#include <soratime.h>
#include <dspcomm.h>

#include "ieee80211facade.hpp"

/***********************************************
	TEnergyDetect
************************************************/
DEFINE_LOCAL_CONTEXT(TEnergyDetect, CF_11CCA, CF_Error);
template<TFILTER_ARGS>
class TEnergyDetect : public TFilter<TFILTER_PARAMS>
{
    static const uint win_len = 8;	                // 8*4  samples, roughly 1us
    static const uint min_sensing_samples = 32;     // 32*4 samples, 3 us
	static const uint max_sensing_samples = 100;    // 100*4 samples, roughly 10us 
	CTX_VAR_RO (uint, 				cca_pwr_threshold );
	CTX_VAR_RW (uint, 				cca_pwr_reading );

	CTX_VAR_RW (CF_11CCA::CCAState, cca_state );
	CTX_VAR_RW (ulong, error_code );

	uint average_energy_;
	uint window_[win_len];
	uint idx_;
	uint count_;

public:
	DEFINE_IPORT(COMPLEX16, 4);
    DEFINE_OPORT(COMPLEX16, 4);

protected:
	FINL void __init () {
		average_energy_ = 0;
		memset ( window_, 0, sizeof(window_));
		idx_   = 0;
		count_ = 0;
	}
	
public:
	REFERENCE_LOCAL_CONTEXT(TEnergyDetect );
	
	STD_TFILTER_CONSTRUCTOR(TEnergyDetect )
		BIND_CONTEXT(CF_11CCA::cca_pwr_threshold, cca_pwr_threshold)
		BIND_CONTEXT(CF_11CCA::cca_pwr_reading,   cca_pwr_reading)
		BIND_CONTEXT(CF_11CCA::cca_state, 		  cca_state)
		BIND_CONTEXT(CF_Error::error_code, 		  error_code)		
	{ __init (); }

	STD_TFILTER_RESET() { 
		__init ();
	}
	
	STD_TFILTER_FLUSH() { }

	BOOL_FUNC_PROCESS (ipin)
	{
		while (ipin.check_read())
		{
			const vcs& pi = cast_ref<vcs>(ipin.peek ());

			// Computer the power average
			// Average over 32 samples
			vi power = hadd (shift_right (SquaredNorm (pi), 5));
			uint ave_power = (uint) power[0];
			average_energy_ = average_energy_ - window_[idx_] + ave_power;
			window_[idx_] = ave_power; 
			idx_ ++; if ( idx_ >= win_len ) idx_ = 0;

			count_ ++;
			if ( count_ >= min_sensing_samples ) {
				if ( count_ >= max_sensing_samples ) {
					error_code = E_ERROR_CS_TIMEOUT;
					ipin.clear();
					return 0;
				}

				if ( average_energy_ >= cca_pwr_threshold ) {
    				cca_pwr_reading = average_energy_;
					cca_state = CF_11CCA::power_detected;
                    RaiseEvent(OnPowerDetected)();
				} 
			} 

			// Energy gating - only low power samples pass
			if ( cca_state != CF_11CCA::power_detected ) {
				vcs& po = cast_ref<vcs>(opin().append());
				po = pi;
				Next()->Process(opin());
			}	

			ipin.pop();

		}
		return true;
	}
};


/*****************************************************
	TCCA11a - Carrier sensing using auto-correlation
******************************************************/
DEFINE_LOCAL_CONTEXT(TCCA11a, CF_11CCA, CF_CFOffset, CF_Error);
template<TFILTER_ARGS>
class TCCA11a : public TFilter<TFILTER_PARAMS>
{
private:
typedef enum { no_energy, high_energy} CCA_STATE;

static const uint ac_norm_shift = 5;
static const uint NORM_SHIFT = 4;

// carrier sensing slot timing
static const uint max_sense_count = 84; // 80/20 = 4us

private:	
	// Context variables	
	CTX_VAR_RO (uint, cca_pwr_threshold );
	CTX_VAR_RW (uint, cca_pwr_reading );
	CTX_VAR_RW (CF_11CCA::CCAState, cca_state );
	CTX_VAR_RW (FP_RAD, CFO_est );
	CTX_VAR_RW (ulong,  error_code );

protected:
	// accumulators
	CMovingWindow<vcs, 4> sample_his;
    CAccumulator <int, 4> ac_re_his;
    CAccumulator <int, 4> ac_im_his;
	
	// norm0 of the auto correlation
    CAccumulator <int, 4> ac_norm_his; 
    CAccumulator <int, 4> energy_his;
	CAccumulator <int, 4> energy_sqr_his;

    // counter for autocorrelation high energy samples
    uint auto_count;

	// counter for the number of samples sensed
	uint sense_count;	

	// counter for the samples in high energy state
	uint high_count;

	// local sync state 
	CCA_STATE sync_state;

	// variables for cross-correlation
    static_wrapper<A16 COMPLEX16 [16][16]> sts_corr_pattern;

	// the max value in cross-correlation
	int peak_corr;	

	// the index in sample of the peak
	int peak_index;
		
public:
	DEFINE_IPORT(COMPLEX16, 4);
	DEFINE_OPORT(COMPLEX16, 4);

protected:
	FINL int GetAutoCorrelation ( vcs& input, int& corr_re, int& corr_im ) {
		vi re, im;
		
		// compute the auto correlation
		conj_mul ( re, im, input, sample_his.First() );

		vi sum_r = hadd (re >> NORM_SHIFT );
		vi sum_i = hadd (im >> NORM_SHIFT );
		
		ac_re_his << sum_r[0];
		ac_im_his << sum_i[0];

		corr_re = ac_re_his.Register(); 
		corr_im	= ac_im_his.Register(); 

		// NORM0
		int sum = (abs(corr_re) + abs(corr_im));

		ac_norm_his << sum;
		
		return ac_norm_his.Register();
	}

	FINL int GetEnergy ( vcs& input ) {
		vi e  = SquaredNorm (input); 
		vi sum  = hadd (e >> NORM_SHIFT );
        energy_his << sum[0];
		return energy_his.Register();
	}

	FINL
	int GetCrossCorrelation ( vcs* sarray, int k, vcs* pattern )
	{
		vi re[4], im[4];
		vi sum_re, sum_im;

		conj_mul ( re[0], im[0], pattern[0], sarray[k] ); k = (k+1) & 0x3;
		conj_mul ( re[1], im[1], pattern[1], sarray[k] ); k = (k+1) & 0x3;				
		conj_mul ( re[2], im[2], pattern[2], sarray[k] ); k = (k+1) & 0x3;				
		conj_mul ( re[3], im[3], pattern[3], sarray[k] ); 

		sum_re = hadd (re[0] + re[1] + re[2] + re[3]);
		sum_im = hadd (im[0] + im[1] + im[2] + im[3]);
		
		int corr = abs(sum_re[0]) + abs(sum_im[0]);
		return corr;
	}
		
	FINL bool establish_sync () {
		int sindex = sample_his.GetIndex ();
		vcs* sarray = (vcs*) sample_his;

		// clear the peak record
		peak_corr = 0;
		
		int sum_corr = 0;
		for (int i=0; i<16; i++) {
			vcs* pattern = (vcs*)sts_corr_pattern[i];
			int corr = GetCrossCorrelation ( sarray, sindex, pattern );
			if ( corr > peak_corr ) {
				peak_corr = corr;
				peak_index = i;
			}
			sum_corr += corr;
		}

		_dump_text ("establish sync: %d %d - sum %d\n", peak_corr, peak_index, sum_corr );
		
		// a peak should be at least twice higher than noise correlation
		return ( peak_corr > (sum_corr >> 3));

	}
		
	FINL bool check_sync () {
		int sindex = sample_his.GetIndex ();
		vcs* sarray = (vcs*) sample_his;

		vcs* pattern = (vcs*)sts_corr_pattern[peak_index];

		int corr = GetCrossCorrelation ( sarray, sindex, pattern );
		
 		_dump_text ( "check peak -- %d\n", corr );		
 		
		// check if it is a peak
		// a later peak should not be lower than the first peak by half
		if ( corr < (peak_corr >> 1)) {
			return false;
		} else {
			if ( corr > peak_corr ) {
				peak_corr = corr;
			}	
			return true;
		}
	}
		
protected:
	FINL void __init_corr_pattern () {
		// build up cross-correlation pattern
		A16 COMPLEX16 temp[64];
		Generate80211aSTS<64> (temp);
		
		// cyclic shift
		for (int i=0; i<16; i++) {
			memcpy (sts_corr_pattern[i], &temp[i], sizeof(COMPLEX16)*16);
		}	
	}
		
	FINL void __init () {
		sync_state = no_energy;

        auto_count = 0;
		sense_count = 0;
		high_count = 0;
		
		sample_his.Clear();
		ac_re_his.Clear();
		ac_im_his.Clear();
		ac_norm_his.Clear(); 
		energy_his.Clear();
		energy_sqr_his.Clear ();

		peak_index = 0;
		peak_corr  = 0;
	}
		
public:
	REFERENCE_LOCAL_CONTEXT(TCCA11a );
	
	STD_TFILTER_CONSTRUCTOR(TCCA11a )
		BIND_CONTEXT(CF_11CCA::cca_pwr_threshold, cca_pwr_threshold)
		BIND_CONTEXT(CF_11CCA::cca_pwr_reading,   cca_pwr_reading)
		BIND_CONTEXT(CF_11CCA::cca_state,		  cca_state)
		BIND_CONTEXT(CF_CFOffset::CFO_est,		  CFO_est)
		BIND_CONTEXT(CF_Error::error_code,		  error_code)	
	{ 
		__init_corr_pattern();
	  	__init (); 
	}
	
	STD_TFILTER_RESET() { 
		__init ();
	}
		
	STD_TFILTER_FLUSH() { }

	BOOL_FUNC_PROCESS (ipin)
	{

		while (ipin.check_read())
		{
			const vcs& pi = cast_ref<const vcs>(ipin.peek ());
			int corr_r, corr_i;

			if ( sync_state == no_energy ) 
			{ 
				// auto-correlation senssing
				int iAutoCorr = GetAutoCorrelation ( pi>>2, corr_r, corr_i );
				int iEnergy   = GetEnergy ( pi>>2 );
				
				// save sample in the accumulator
				sample_his << (pi>>2);
				
				sense_count += 4;
				
				if (
					 (iEnergy > (int)cca_pwr_threshold) && 
					 iAutoCorr >= (iEnergy - (iEnergy >> 3)))
				{
                    auto_count++;

					// any case we are sensing high, we reset the sense counter
					sense_count = 0;

					// if remaining in plateau for 4*4=16 samples
                    if (auto_count >= 4) 
                    {
					    _dump_text ( "power detected!\n" );
						
					    // use cross-correlation to establish sync
					    if ( establish_sync () ) {
						    sync_state = high_energy;
						
						    cca_pwr_reading = iEnergy;
					
						    // adjust the peak_index
						    high_count = 0;
						
						    if ( peak_index > 3 ) {
							    high_count = peak_index / 4;
							    peak_index = peak_index & 0x03;
						    }
						
						    // coarse CFO estimation
						    // Kun: disabled
						    /*						
						    FP_RAD theta = uatan2 ( corr_i, corr_r );
						    CFO_est = (theta >> 4); 
						    printf ( "coarse freq offset (%lf) %lfKHz\n", 
								    fprad2rad(CFO_est),
								    20000 * fprad2rad(CFO_est) / 2 / 3.141593 );
						    */		
					    } 
                    }
				} 
/*				
				else if ( sense_count >= max_sense_count && auto_count == 0)
				{
					// CS timeout
					error_code = E_ERROR_CS_TIMEOUT;
					ipin.clear();
					return true;
				}
*/
                else
                {
                    auto_count = 0;
                }
			} else if ( sync_state == high_energy ) {
				sample_his << (pi>>2);

				high_count ++;
					
				if ( high_count % 4 == 0 ) {
					if ( !check_sync ()) {
						// cross-correlation peak lost
						// if we already have enough peaks found, we mark
						// it as the end of STS
						if ( high_count > 8 ) {
							cca_state = CF_11CCA::power_detected;
							RaiseEvent(OnPowerDetected)();

							_dump_text ( "CCA done!\n" );
						} else {
							sync_state = no_energy;
							sense_count = 0;
						}
					}
				}	
			} 


			// Energy gating - only low power samples pass
			if ( sync_state == no_energy ) {
				vcs& po = cast_ref<vcs>(opin().append());
				po = pi;
				Next()->Process(opin());
			}	

			ipin.pop();
		}

		// now we have processed all data inqueue
		if ( sense_count >= max_sense_count && sync_state == no_energy ) {
			// CS time out
			error_code = E_ERROR_CS_TIMEOUT;
		}
	
		return true;
	}
};
