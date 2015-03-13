#pragma once
#include "sora.h"
#include "dspcomm.h"
#include "ieee80211facade.hpp"

/***********************************************
	TSymTiming - 4 TO 1 decimation
************************************************/
DEFINE_LOCAL_CONTEXT(TSymTiming, CF_VOID);
template<TFILTER_ARGS>
class TSymTiming : public TFilter<TFILTER_PARAMS>
{
protected:
	int m_index; // the index of the sample to choose
    int m_frag;

	COMPLEX16 dsamples_[8]; // up to 8 decimated samples
	FINL void _init () {
		m_frag  = 0;
		m_index = 2;
	}

public:
	// define pin traits
	// input
	DEFINE_IPORT (COMPLEX16, 28); // a whole signal block
	// output
    DEFINE_OPORT (COMPLEX16, 1);  // output downsample sample one-by-one  

public:
    REFERENCE_LOCAL_CONTEXT(TSymTiming);
    STD_TFILTER_CONSTRUCTOR(TSymTiming)
    {
    	_init ();
    }
	STD_TFILTER_RESET() {
		_init ();
	}
	STD_TFILTER_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin)
    {
    	// Marker::TSymTiming
        while(ipin.check_read())
        {
            COMPLEX16* pInput = (COMPLEX16*)ipin.peek();

			Decimation   ( pInput ); // 4x decimation

			// N.B.
			// Here, we implement a simple non-data aided timing 
			// estimation algorithm a.k.a. early-late samping alg.
			// 
			// timing error is sampled and adjusted every signal block
			// or 28 samples
			//
			AdjustTiming ( pInput ); 

            ipin.pop();

        }
        return true;
    }

protected:
	FINL 
	void Decimation ( COMPLEX16* pInput ) {
		int idx = m_index;
		while ( idx < 28 ) {
			COMPLEX16& out = *opin().append();
			if ( idx < 0 ) {
				out = pInput[0];
				m_index += 4;
			} else {
				out = pInput[idx];
			}
			idx += 4;
			Next()->Process ( opin() );
		}
		if ( m_index >= 4) {
			m_index =0;
		}

		/*
		int idx = 0; int symcnt = 0;
		if ( m_index < 0 ) {
			// we have missed one symbol in last round
			//*opin().append() = *pY;
			//Next()->Process ( opin() );
			dsamples_[idx] = *pY;
			symcnt ++;
			m_index += 4;
		} 
		
		pY += m_index;

		int cnt = 7;
		if ( m_index >= 4 ) {
			m_index = 0;
			m_frag = -m_frag;
			// m_frag = 0;
			cnt --;
		}

		symcnt += cnt;
		for (int i=0; i<cnt; i++ ) {
//			*opin().append() = *pY;
//			Next()->Process ( opin() );
			pY += 4;
		}
		*/
        
	}

	FINL void AdjustTiming ( COMPLEX16* pInput ) 
	{
		// early-late detector
		vcs* pv = reinterpret_cast<vcs*> (pInput);
		vi   energy[7];
		vi   sum;
		set_zero (sum);
		rep<7>::vshift_right (pv, 3);
		rep<7>::vsqrnorm (energy, pv);
		rep<7>::vsum     (sum, energy );

        assert(m_index >= 0 && m_index < 4);
		int early_index = (m_index==0)?3:(m_index-1);
		int late_index  = (m_index==3)?0:(m_index+1);

/*		printf ( "timing: index %d ( e %d i %d l %d)\n", 
			m_index, 
			sum[early_index],
			sum[m_index],
			sum[late_index] );
*/		
		if ( sum[early_index] < sum[late_index] ) {
			if ( sum[m_index] < sum[early_index] ) {
				// quick jump
				m_index ++;
				m_frag = 0;
			} else
			if ( sum[m_index] < sum[late_index] ) {
				// later is better	
				m_frag ++;
			}
		} else {
			if ( sum[m_index] < sum[late_index] ) {
				// quick jump
				m_index --;
				m_frag = 0;
			} else
			if ( sum[m_index] < sum[early_index] ) {			
				// early is better
				m_frag --;
			}	
		}

		if ( m_frag >= 4 ) {
			m_index ++; m_frag = -3;
		} else if ( m_frag <= -4 ) { 
			m_index --; m_frag = 3;
		}
        assert (m_index >= -1 && m_index < 5);

//		printf ( "timing : m_index %d\n", m_index );
	
	}
};

/***********************************************
	TBarkerSync
************************************************/

DEFINE_LOCAL_CONTEXT(TBarkerSync, CF_Error);
template<TFILTER_ARGS>
class TBarkerSync : public TFilter<TFILTER_PARAMS>
{
static const int barker_len = 11;
private:
	CTX_VAR_RW(ulong, error_code );
	
protected:
	typedef enum {
		NO_PEAK_FOUND = 0,
		PEAK_FOUND, 
		PEAK_VALID, 
		PEAK_VALIDED,
		BARKER_SYNCED,		
	} SYNC_STATE ;
	
    SYNC_STATE m_sync_flag;

	int m_last_peak_cnt;	// # of samples since last peak found
	int m_max;
	int search_count;

	// partial sum of Barker despread
	COMPLEX16 m_partial_sum[11];	

	FINL void _init () {
		m_sync_flag 	= NO_PEAK_FOUND; // no peak is found
		m_last_peak_cnt = -1;             
		m_max = 0;
	 	search_count = 0;		
		memset ( m_partial_sum, 0, sizeof(m_partial_sum));
	}
public:
	// inport and outport
	DEFINE_IPORT (COMPLEX16, 1);  // read a sample each time
    DEFINE_OPORT (COMPLEX16, 1);  // output one sample after alignment

public:	
    REFERENCE_LOCAL_CONTEXT(TBarkerSync);
    STD_TFILTER_CONSTRUCTOR(TBarkerSync)
        BIND_CONTEXT(CF_Error::error_code, error_code)
    {
    	_init ();
    }
	STD_TFILTER_RESET () {
		_init ();
    }
	STD_TFILTER_FLUSH () {}
	
    BOOL_FUNC_PROCESS(ipin)
    {
    	// Marker::BarkerSync
        while(ipin.check_read())
        {
            const COMPLEX16& input = *ipin.peek();

			if (m_sync_flag == BARKER_SYNCED) {
				// already synced - just pass the sample downstream
				*opin().append() = input;
				Next()->Process (opin());
			} else {
				search_count ++;
				if ( search_count >= (barker_len * 4)) {
					error_code = E_ERROR_SYNC_TIMEOUT;
					ipin.pop ();
					return false;
				}
				
				// otherwise - sync on barker code
				int corr = UpdateBarkerCorrelation ( input );
				if ( m_sync_flag == NO_PEAK_FOUND ) {
					// peak searching phase
					if ( corr > m_max ) {
						m_max = corr;
						m_last_peak_cnt = 1;
					} else {
						m_last_peak_cnt ++;
						if ( m_last_peak_cnt == barker_len ) {
							m_sync_flag = PEAK_FOUND;
						}
					}
				} else if ( m_sync_flag == PEAK_FOUND) {
					m_max = corr / 2;
					m_last_peak_cnt = 1;
					m_sync_flag = PEAK_VALID;
				} else if ( m_sync_flag == PEAK_VALID) {
					// validating phase
					if ( corr > m_max ) {
						// validation failed
						// reason - a peak found in the middle
						m_max = corr;
						m_last_peak_cnt = 0;
						m_sync_flag = NO_PEAK_FOUND;
					} else {
						m_last_peak_cnt ++;
						if ( m_last_peak_cnt == barker_len ) {
							// validated - synchronized
							m_sync_flag = PEAK_VALIDED;
						}
					}
				} else {
					// just skip one more symbol
					m_sync_flag = BARKER_SYNCED;
				}
			}	
			
			ipin.pop();
        }
        return true;
    }

public:
	//	Barker11[] = { 1, -1, 1, 1, -1, 1, 1, 1, -1, -1, -1 };

	FINL 
	int UpdateBarkerCorrelation ( COMPLEX16 ss ) {
		ss = ss >> 4; 
		COMPLEX16 o;
		o = m_partial_sum[0] - ss;
		m_partial_sum[0] = m_partial_sum[1] - ss;
		m_partial_sum[1] = m_partial_sum[2] - ss;			
		m_partial_sum[2] = m_partial_sum[3] + ss;
		m_partial_sum[3] = m_partial_sum[4] + ss;
		m_partial_sum[4] = m_partial_sum[5] + ss;
		m_partial_sum[5] = m_partial_sum[6] - ss;
		m_partial_sum[6] = m_partial_sum[7] + ss;
		m_partial_sum[7] = m_partial_sum[8] + ss;
		m_partial_sum[8] = m_partial_sum[9] - ss;
		m_partial_sum[9] = ss;

		return norm2(o);		
	}
};






/***********************************************************************/
class CF_TimeRec
{
	FACADE_FIELD(int, flag_part); // the fragment part of timing
	FACADE_FIELD(int, int_part);  // the integer part of timing
};

//
// Timing recovery
// 
DEFINE_LOCAL_CONTEXT(TTimingRec, CF_TimeRec);
template<TFILTER_ARGS>
class TTimingRec : public TFilter<TFILTER_PARAMS>
{
protected:
	// binding variables
    int &m_frag;
	int &m_index;

	
	FINL void _init () {
		m_frag  = 0;
		m_index = 0;
	}

public:
	// define pin traits
	// input
	DEFINE_IPORT (SignalBlock, 1); // a whole signal block
	// output
    DEFINE_OPORT (COMPLEX16, 1);  // output downsample sample one-by-one  

public:
    REFERENCE_LOCAL_CONTEXT(TTimingRec);
    STD_TFILTER_CONSTRUCTOR(TTimingRec)
        BIND_CONTEXT(CF_TimeRec::flag_part, m_frag)
        BIND_CONTEXT(CF_TimeRec::int_part , m_index)
    {
    	_init ();
    }
	STD_TFILTER_RESET() {
		_init ();
	}
	STD_TFILTER_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            COMPLEX16* pInput = (COMPLEX16*)ipin.peek();

			Decimation   ( pInput ); // 4x decimation

			// N.B.
			// Here, we implement a simple non-data aided timing 
			// estimation algorithm
			// 
			// timing error is sampled and adjusted every signal block
			// or 28 samples
			//
			AdjustTiming ( pInput ); 

            ipin.pop();

        }
        return true;
    }

protected:
	FINL void Decimation ( COMPLEX16* pInput ) {
		COMPLEX16* pY = pInput;
		if ( m_index < 0 ) {
			// we have missed one symbol in last round
			*opin().append() = *pY;
//printf ( "downsamped <%d, %d>\n", pY->re, pY->im );
			Next()->Process ( opin() );

			m_index += 4;
		} 
		
		pY += m_index;

		int cnt = 7;
		if ( m_index >= 4 ) {
			m_index = 0;
			m_frag = -m_frag;
			// m_frag = 0;
			cnt --;
		}

		for (int i=0; i<cnt; i++ ) {
			*opin().append() = *pY;
//printf ( "downsamped <%d, %d>\n", pY->re, pY->im );			
			Next()->Process ( opin() );
			pY += 4;
		}

	}

	FINL void AdjustTiming ( COMPLEX16* pInput ) 
	{
		int idx = m_index + 4;
		int x0, x1;
		if ( m_frag > 0 ) {
			x0 = norm2 ( pInput[idx] );
			x1 = norm2 ( pInput[idx+1] );

		} else {
			x0 = norm2 ( pInput[idx-1] );
			x1 = norm2 ( pInput[idx] );
		}

		
		if ( x0 < x1 ) {
			m_frag ++;
			if ( m_frag >= 8 ) {
				m_index ++;
				m_frag = -7;
			}
		} else {
			m_frag --;
			if ( m_frag <= -8 ) {
				m_index --;
				m_frag = 7;
			}
		
		}
		
//		printf ( "deci index %d.%d\n", m_index, m_frag );
	}
};

//
// Barker Synchronizer
// This Brick tries to lock onto a Barker spreaded symbols
// 
#define NO_PEAK_FOUND 0
#define PEAK_FOUND_1  1
#define PEAK_FOUND_2  2
#define BARKER_SYNCED 3

class CF_BarkerSyncer {
	FACADE_FIELD (int,sync_flag_);
public:
    FINL void Reset()
    {
		sync_flag_()     = NO_PEAK_FOUND;  // not synced
    }
};
