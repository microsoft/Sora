#pragma once

#include <fft.h>

//
// This brick can be optimized and removed
//

DEFINE_LOCAL_CONTEXT(TTS11aSrc, CF_Error);
template<TSOURCE_ARGS>
class TTS11aSrc: public TSource<TSOURCE_PARAMS>
{
private:
	static const ushort sts_mod = (ushort) (1.0 * bpsk_mod_11a * 1.472);
	static const ushort lts_mod = bpsk_mod_11a;

private:
	CTX_VAR_RW (ulong, error_code );     /* error code for the modulation  */	
	
private:
    struct _init_lut
    {
        void operator()(COMPLEX16 (&lut) [640])
        {
		    A16 COMPLEX16 temp[128];

		    memset (temp, 0, sizeof(temp));
		    int i;
		    // short training symbol
		    temp[4].re = temp[4].im = - sts_mod ;
		    temp[8].re = temp[8].im = - sts_mod ;
		    temp[12].re = temp[12].im = sts_mod ;
		    temp[16].re = temp[16].im = sts_mod ;
		    temp[20].re = temp[20].im = sts_mod ;
		    temp[24].re = temp[24].im = sts_mod ;

		    temp[104].re = temp[104].im = sts_mod ;
		    temp[108].re = temp[108].im = - sts_mod ;
		    temp[112].re = temp[112].im = sts_mod ;
		    temp[116].re = temp[116].im = - sts_mod ;
		    temp[120].re = temp[120].im = - sts_mod ;
		    temp[124].re = temp[124].im = sts_mod ;

		    IFFTSSEEx<128>((vcs*)temp);
		    rep<32>::vshift_right ((vcs*)temp, 4);

		    for (i = 0; i < 128; i++) {
			    lut[i] = temp[FFTLUTMapTable<128>(i)];
		    }

		    // this is an explicit overflow
		    rep<48>::vmemcpy ( (vcs*) (lut+32*sizeof(COMPLEX16)), (vcs*) lut );
		    // Windowing
		    lut[0].re >>= 1; lut[0].im >>= 1;
		    lut[1].re >>= 1; lut[1].im >>= 1;

		    lut[318].re >>= 1; lut[318].im >>= 1;
		    lut[319].re >>= 1; lut[319].im >>= 1;

		    // long training symbol
		    memset (temp, 0, sizeof(temp));

		    for (i=1; i<=26; i++ ) {
			    if (LTS_Positive_table[i]) 
				    temp[i].re = lts_mod;
			    else temp[i].re = -lts_mod;
			    temp[i].im = 0;
		    }

		    for (i=64-26; i<64; i++ ) {
			    if (LTS_Positive_table[i]) 
				    temp[i+64].re = lts_mod;
			    else temp[i+64].re = -lts_mod;
			    temp[i+64].im = 0;
		    }

		
		    IFFTSSEEx<128>((vcs*)temp);
		    rep<32>::vshift_right ((vcs*)temp, 4);

		    for (i = 0; i < 128; i++) {
			    lut[i+320+64] = temp[FFTLUTMapTable<128>(i)];
		    }
		    rep<32>::vmemcpy ( (vcs*) (lut+320+64+128), (vcs*) (lut+320+64) );
		    // GI2
		    rep<16>::vmemcpy ( (vcs*) (lut+320), (vcs*) (lut+640-64) );
		    // Windowing
		    lut[320].re >>= 1; lut[320].im >>= 1;
		    lut[321].re >>= 1; lut[321].im >>= 1;

		    lut[638].re >>= 1; lut[638].im >>= 1;
		    lut[639].re >>= 1; lut[639].im >>= 1;

    /*
		    int lcnt = 8;
		    for (i=0; i<640; i++) {
			    if (lcnt == 0 ) {
				    lcnt = 8; printf ( "\n" );
			    }
			    printf ( "<%d, %d> ", lut[i].re, lut[i].im );
			    lcnt --;
		    }
		    printf ( "\n" );
     */		
        }
	};
    const static_wrapper<COMPLEX16 [640], _init_lut> m_lut;
	
public:
    DEFINE_OPORT(COMPLEX16, 640);
	
public:
    REFERENCE_LOCAL_CONTEXT(TTS11aSrc);
    STD_TSOURCE_CONSTRUCTOR(TTS11aSrc) 
		BIND_CONTEXT (CF_Error::error_code,	   error_code ) 
	{
	}
		
    STD_TSOURCE_RESET() {}
	
    STD_TSOURCE_FLUSH() { }

    bool Process ()
    {
       	COMPLEX16* po;
		po = opin().append();
		rep<160>::vmemcpy ( (vcs*) po, (vcs*) m_lut.get() );

		Next()->Process(opin());

    	return FALSE;		
    }
};
