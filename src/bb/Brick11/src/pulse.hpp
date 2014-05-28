#pragma once
#include <math.h>

#include "vector128.h"

#undef PI
#define PI 3.141593


FINL vcs FIRIncremental (vcs & vi, vcs* pTemparray, 
						 vs * Fir_Taps, int tap_number)
{
    register vs * pTap = (vs*) Fir_Taps;
    vcs tt[4];
    register vcs * pTemp = pTemparray;
    register vcs * pTT   = tt;
    for ( int i=0; i<4; i ++ ) {
        * pTT = mul_low ( (vs&) vi, *pTap );
        * pTT = add (* pTT, * pTemp );
        pTap ++; pTemp ++; pTT ++;
    }
    vcs ret = hadd4 (tt[0], tt[1], tt[2], tt[3] );

    // calc all temp results
    vcs* pTemp1 = pTemparray;
    for (int j = 4; j < tap_number; j++)
    {
        tt[0]   = mul_low(*pTap, (vs) vi);
        *pTemp1 = add(tt[0], *pTemp);
        pTemp ++; pTap ++; pTemp1++;
    }

    return ret;	
}

// 
// Pulse shaper - Raised Cosine Filter
// With 4x upsampling
// 
// N.B. All input should be normalized to 1 and -1
//
DEFINE_LOCAL_CONTEXT(TPulseShaper, CF_VOID);
template<TFILTER_ARGS>
class TPulseShaper : public TFilter<TFILTER_PARAMS>
{
// N.B. power_level should be slightly less than the maximal value
// to prevent overflow
static const int power_level = 80; 
static const int tap_span = 9;
static const int tap_cnt = 2*tap_span+1; 
protected:
	// must extended by 3 and initialize them to zero
	vcs m_temps[tap_cnt+3]; 
	vcs m_input;

	FINL void _init () {
		memset (m_temps, 0, sizeof(m_temps) );
	}

    struct _init_lut
    {
        void operator()(short (&lut)[tap_cnt+3][8])
        {
		    memset ( lut, 0, sizeof(lut));

		    for ( int i=-tap_span; i<=tap_span; i++) {
			    // compute square-root-raised-cosine
			    double x;
                if (i == 1 || i == -1)
                    x = 1;
                else
                    x = 4 * cos(PI*i/2) / PI / (1 - i*i);

			    // normalized to the power level
			    short xx = (short) (x*power_level + .5); 

			    lut[i+tap_span][0] = lut[i+tap_span][1] =
			    lut[i+tap_span+1][2] = lut[i+tap_span+1][3] =
			    lut[i+tap_span+2][4] = lut[i+tap_span+2][5] =
			    lut[i+tap_span+3][6] = lut[i+tap_span+3][7] = xx;
		    }
    #if 0
		    for ( int i=0; i<tap_cnt+3; i++) {
			    for ( int j=0; j<8; j++) {
				    printf ( "%d, ",  lut[i][j] );
			    }
			    printf ( "\n" );
		    }
    #endif
        }
	};
    const static_wrapper<short [tap_cnt+3][8], _init_lut> m_taps;
	
public:
	// inport & outport
	DEFINE_IPORT (COMPLEX8,  1);
    DEFINE_OPORT (COMPLEX16, 4);

public:	
    REFERENCE_LOCAL_CONTEXT(TPulseShaper);
    STD_TFILTER_CONSTRUCTOR(TPulseShaper)
    {
    	_init ();
    }
	STD_TFILTER_RESET()
    {
    	_init();
    }
	STD_TFILTER_FLUSH() {
		for (int i=0; i<tap_cnt; i++) {
			*(vcs*) opin().append() = m_temps[i];
			Next()->Process(opin());
		}
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
			COMPLEX8 si = *ipin.peek();

			set_zero (m_input);
			m_input[0].re = si.re; m_input[0].im = si.im;
			ipin.pop ();
            // printf ( "sample %d %d\n", m_input[0].re, m_input[0].im );

			vcs * po = (vcs*)opin().append();
			
			*po = FIRIncremental (m_input, m_temps,
					  (vs*) m_taps.get(), 
					  tap_cnt+3);

			Next()->Process(opin());
           
        }

        return true;
    }
};

// 
// Matched filter - Raised Cosine Filter
// 
DEFINE_LOCAL_CONTEXT(TMatchFilter, CF_VOID);
template<TFILTER_ARGS>
class TMatchFilter : public TFilter<TFILTER_PARAMS>
{
static const int tap_span = 9;
static const int tap_cnt = 2*tap_span+1; // 33
protected:

	vcs m_temps[tap_cnt+3];
	vcs m_input;

	FINL void _init () {
		memset (m_temps, 0, sizeof(m_temps) );
	}
	
    struct _init_lut
    {
        void operator()(short (&lut)[tap_cnt+3][8])
        {
		    memset ( lut, 0, sizeof(lut));
		
		    for ( int i=-tap_span; i<=tap_span; i++) {
			    // compute raised cosine
			    double x;
			    double a = 1;
			    int T=4;
			    if (i==0) x = 1;
			    else if ( 1-4.0*a*a*i*i/T/T < 0.0001) 
				    x = PI / 4 * sin (PI*1.0*i/T) / i / PI *T;
			    else x = T*sin(PI*1.0*i/T) / PI /i*cos(a*PI*i/T)/(1-4.0*a*a*i*i/T/T);

			    short xx = (short) (x*160);

			    lut[i+tap_span][0] = lut[i+tap_span][1] =
			    lut[i+tap_span+1][2] = lut[i+tap_span+1][3] =
			    lut[i+tap_span+2][4] = lut[i+tap_span+2][5] =
			    lut[i+tap_span+3][6] = lut[i+tap_span+3][7] = xx;
		    }
    #if 0
		    for ( int i=0; i<tap_cnt+3; i++) {
			    for ( int j=0; j<8; j++) {
				    printf ( "%d, ",  lut[i][j] );
			    }
			    printf ( "\n" );
		    }
    #endif
        }
	};
    const static_wrapper<short [tap_cnt+3][8], _init_lut> m_lut;

public:
	// inport & outport
	DEFINE_IPORT (COMPLEX16, 4);
    DEFINE_OPORT (COMPLEX16, 4);

	
    REFERENCE_LOCAL_CONTEXT(TMatchFilter);
    STD_TFILTER_CONSTRUCTOR(TMatchFilter)
    {
    	_init ();
    }
	
	STD_TFILTER_RESET()
    {
    	_init();
    }

	STD_TFILTER_FLUSH() {
		vcs padding;
		setzero (padding);
		for (int i=0; i<16; i++) {
			vcs * po = (vcs*)opin().append();
			
			*po = FIRIncremental (m_input, m_temps,
					  (vs*) m_taps, 
					  tap_cnt+3);
			Next()->Process(opin());
		}
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
			m_input = *((vcs*)ipin.peek());
			ipin.pop ();

			vcs * po = (vcs*)opin().append();
			
			*po = FIRIncremental (m_input, m_temps,
					  (vs*) m_taps, 
					  tap_cnt+3 );
			*po = shift_right (*po, 8);
			
			*po = m_input;
			Next()->Process(opin());           
        }

        return true;
    }
};

// 
// Quick pulse shaper - Raised Cosine Filter
// With 4x upsampling
// This algorithm is quick as it removes the unecessary compuation of 
// samples with zero
// 
// N.B. All input should be normalized to 1 and -1
//
static const int tap_left  = -11;
static const int tap_right = 8;
static const int tap_cnt = (tap_right-tap_left+1) / 4; 

DEFINE_LOCAL_CONTEXT(TQuickPulseShaper, CF_VOID);
template<TFILTER_ARGS>
class TQuickPulseShaper : public TFilter<TFILTER_PARAMS>
{
// N.B. power_level should be slightly less than the maximal value
// to prevent overflow
static const int power_level = 80;

protected:
    struct _init_taps
    {
        void operator()(short (&lut)[tap_cnt][8])
        {
		    memset ( lut, 0, sizeof(lut));
		
		    int ii = 8;
		    for ( int j=0; j<5; j++ ) {
			    // rows
			    for ( int k=0; k<4; k++ ) {
				    // column
				    int i = ii-k;
				
			        // compute square-root-raised-cosine
			        double x;
                    if (i == 1 || i == -1)
                        x = 1;
                    else
                        x = 4 * cos(PI*i/2) / PI / (1 - i*i);

			        // normalized to the power level
			        short xx = (short) (x*power_level + .5); 

				    lut[j][2*k] = lut[j][2*k+1] = xx;
			    }
			    ii-=4;
		    }
#if 0		
		    for ( int i=0; i<tap_cnt+3; i++) {
			    for ( int j=0; j<8; j++) {
				    printf ( "%d, ",  lut[i][j] );
			    }
			    printf ( "\n" );
		    }
#endif		
        }
    };
	// taps - duplicated for vector processing
    const static_wrapper<short [tap_cnt][8], _init_taps> m_taps;

	// must extended by 3 and initialize them to zero
	vcs m_temps[tap_cnt]; 
	vcs m_input;

	FINL void _init () {
		memset (m_temps, 0, sizeof(m_temps) );
	}

public:
	// inport & outport
	DEFINE_IPORT (COMPLEX8,  1);
    DEFINE_OPORT (COMPLEX16, 4);

public:	
    REFERENCE_LOCAL_CONTEXT(TQuickPulseShaper);
    STD_TFILTER_CONSTRUCTOR(TQuickPulseShaper)
    {
    	_init ();
    }
	STD_TFILTER_RESET()
    {
    	_init();
    }
	STD_TFILTER_FLUSH() {
		for (int i=0; i<tap_cnt; i++) {
//			*(vcs*) opin().append() = m_temps[i];
//			Next()->Process(opin());

			COMPLEX16 ss;
			ss.re = ss.im = 0;
			set_all (m_input, ss);
			
		
			vcs * po = (vcs*)opin().append();
		
			*po = QuickShape ( m_input, m_temps,
								(const vs*) m_taps.get(), tap_cnt);
			Next()->Process(opin());
		}
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
			COMPLEX8 si = *ipin.peek();
			COMPLEX16 ss;
			ss.re = si.re; ss.im = si.im;
			set_all (m_input, ss);
			ipin.pop ();

			vcs * po = (vcs*)opin().append();
			
			*po = QuickShape ( m_input, m_temps,
					  			(const vs*) m_taps.get(), tap_cnt);
			Next()->Process(opin());
           
        }

        return true;
    }

	
	FINL vcs QuickShape ( vcs & vi, vcs* stores, 
						  const vs * taps, int tap_number)
	{
		vs tt;		
		tt = mul_low ( (vs&)vi, taps[0]); 
		vcs result = (vcs)add (tt, (vs)stores[0] );

		for ( int i=0; i<tap_number-1; i ++ ) {
			tt 		  = mul_low ( (vs&) vi, taps[i+1] );
			stores[i] = add ( stores[i+1], (vcs)tt);
		}
	
		return result; 
	}
};
