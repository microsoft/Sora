#pragma once

#include <intalg.h>
#include <math.h>
//
// TSine: Generate a few period of sine wave
// Note: Freq is KHz - you cannot specify a freq less than 
//
DEFINE_LOCAL_CONTEXT(TSine, CF_VOID);

template<size_t FREQ, size_t START_PHASE, size_t N_PERIOD, size_t SAMPLE_RATE=40000>
class TSine {
public:
template<TSOURCE_ARGS>
class Source: public TSource<TSOURCE_PARAMS>
{
static const int NORM_ONE = 20000;
static const int PERIOD   = SAMPLE_RATE / FREQ; // 
protected:
	FP_RAD m_sphase;
	FP_RAD m_dphase;

	void __init () {
		m_sphase = START_PHASE;
		m_dphase = rad2fprad ( 2 * PI * FREQ / SAMPLE_RATE );
	}
public:
	DEFINE_OPORT(COMPLEX16, PERIOD);

public:
	REFERENCE_LOCAL_CONTEXT(TSine);

	STD_TSOURCE_CONSTRUCTOR(Source)
	{
		__init (); 
	}
	STD_TSOURCE_RESET () {}
	STD_TSOURCE_FLUSH () {}

	bool Process () {
		int i = N_PERIOD;
		FP_RAD phase = m_sphase;
		while (i) {
			COMPLEX16 * po = opin().append ();

			for ( int j =0; j<PERIOD; j++) {
				po[j].re = (short) ((NORM_ONE * ucos (phase)) >> 16); 
				po[j].im = (short) ((NORM_ONE * usin (phase)) >> 16);

				phase += m_dphase;
			}

			Next()->Process (opin());
			i--;
		}

		return false;
	}

};
};
