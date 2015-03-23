#pragma once

#include <stdio.h>

#include <sora.h>
#include <brick.h>
#include <fft.h>


//
// IFFT8x - 8x Oversampled IFFT and add GI - 5M
//
DEFINE_LOCAL_CONTEXT(TIFFT8x, CF_VOID);
template<TFILTER_ARGS>
class TIFFT8x: public TFilter<TFILTER_PARAMS>
{
	static const int shift_len = 2;
public:
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(COMPLEX16, 512 + 128);
	
public:
    REFERENCE_LOCAL_CONTEXT(TIFFT8x);
    STD_TFILTER_CONSTRUCTOR(TIFFT8x) { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *in  = (vcs*) ipin.peek();
            vcs *out = (vcs*) opin().append();

            oversampled_ifft (in, out+32);
			
			// add GI
			rep_memcpy <32> (out, out+128);

			// Windowing here
			COMPLEX16* pout = (COMPLEX16*) out;
			
			pout[0].re >>= 1; pout[0].im >>= 1;
			pout[1].re >>= 1; pout[1].im >>= 1;

			pout[510 + 128].re >>= 1; pout[510 + 128].im >>= 1;
			pout[511 + 128].re >>= 1; pout[511 + 128].im >>= 1;

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

	FINL
	void oversampled_ifft (vcs * pcInput, vcs * pcOutput)
	{
		vcs temp[512 / vcs::size];
		
		memset (temp, 0, sizeof(temp));
		rep_memcpy<8> (temp, pcInput );
		rep_memcpy<8> (&temp[128-8], &pcInput[8]);
		
		IFFT<512> (temp, pcOutput );

		rep_shift_right<128> (pcOutput, pcOutput, shift_len);	
	}
	
};

// #define DOUBLE_STS

#ifdef DOUBLE_STS
#define STS_N 2
#else
#define STS_N 1
#endif

//
// Defines Instanbul Wireless Training Symbols
// 10 STS + 10 STS + 2 LTS
//
DEFINE_LOCAL_CONTEXT(TTSIstanbulSrc, CF_Error);
template<TSOURCE_ARGS>
class TTSIstanbulSrc: public TSource<TSOURCE_PARAMS>
{
private:
	static const ushort sts_mod = (ushort) (1.0 * bpsk_mod_11a * 1.472);
	static const ushort lts_mod = bpsk_mod_11a;
	static const int  ts_len    = 320;

private:
	CTX_VAR_RW (ulong, error_code );     /* error code for the modulation  */	
	
private:
    struct _init_lut
    {
        void operator()(COMPLEX16 (&lut) [STS_N*ts_len+ts_len])
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
			rep_shift_right<32>((vcs*)temp, (const vcs*)temp, 4);
		    // rep<32>::vshift_right 

		    for (i = 0; i < 128; i++) {
			    lut[i] = temp[FFTLUTMapTable<128>(i)];
		    }

		    // this is an explicit overflow
		    rep_memcpy<48> ( (vcs*) (lut+32*sizeof(COMPLEX16)), (vcs*) lut );
		    // Windowing
		    lut[0].re >>= 1; lut[0].im >>= 1;
		    lut[1].re >>= 1; lut[1].im >>= 1;

		    lut[ts_len-2].re >>= 1; lut[ts_len-2].im >>= 1;
		    lut[ts_len-1].re >>= 1; lut[ts_len-1].im >>= 1;

			// second sts
#ifdef DOUBLE_STS
			memcpy ( &lut[320], lut, 320 * sizeof(COMPLEX16));
#endif

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
		    rep_shift_right<32>((vcs*)temp, (const vcs*)temp, 4);

		    for (i = 0; i < 128; i++) {
			    lut[i+STS_N*ts_len+64] = temp[FFTLUTMapTable<128>(i)];
		    }
		    rep_memcpy<32> ( (vcs*) (lut+STS_N*ts_len+64+128), (vcs*) (lut+STS_N*ts_len+64) );
		    // GI2
		    rep_memcpy<16> ( (vcs*) (lut+STS_N*ts_len), (vcs*) (lut+STS_N*ts_len+ts_len-64) );

		    // Windowing
		    lut[STS_N*ts_len].re >>= 1;   lut[STS_N*ts_len].im >>= 1;
		    lut[STS_N*ts_len+1].re >>= 1; lut[STS_N*ts_len+1].im >>= 1;

		    lut[STS_N*ts_len+ts_len-2].re >>= 1; lut[STS_N*ts_len+ts_len-2].im >>= 1;
		    lut[STS_N*ts_len+ts_len-1].re >>= 1; lut[STS_N*ts_len+ts_len-1].im >>= 1;
        }
	};
    const static_wrapper<COMPLEX16 [STS_N*ts_len+ts_len], _init_lut> m_lut;
	
public:
    DEFINE_OPORT(COMPLEX16, STS_N*ts_len+ts_len);
	
public:
    REFERENCE_LOCAL_CONTEXT(TTSIstanbulSrc);
    STD_TSOURCE_CONSTRUCTOR(TTSIstanbulSrc) 
		BIND_CONTEXT (CF_Error::error_code,	   error_code ) 
	{
	}
		
    STD_TSOURCE_RESET() {}
	
    STD_TSOURCE_FLUSH() { }

    bool Process ()
    {
       	COMPLEX16* po;
		po = opin().append();

		memcpy ( po, m_lut.get(), sizeof(COMPLEX16) * (STS_N*ts_len + ts_len));

		Next()->Process(opin());

    	return FALSE;		
    }
};

//
// Defines Instanbul Wireless Training Symbols - 10MHz
// 10 STS + 10 STS + 2 LTS
//
DEFINE_LOCAL_CONTEXT(TTSIstanbulSrc10, CF_Error);
template<TSOURCE_ARGS>
class TTSIstanbulSrc10: public TSource<TSOURCE_PARAMS>
{
private:
	static const ushort sts_mod = (ushort) (1.0 * bpsk_mod_11a * 1.472);
	static const ushort lts_mod = bpsk_mod_11a;
	static const int    fft_size  = 256;
	static const int    shift_len = 2;
	static const int    ts_len    = 640;
	static const int    lts_cp    = 128;
private:
	CTX_VAR_RW (ulong, error_code );     /* error code for the modulation  */	
	
private:
    struct _init_lut
    {
        void operator()(COMPLEX16 (&lut) [STS_N*ts_len + ts_len])
        {
		    A16 COMPLEX16 temp [256];

		    memset (temp, 0, sizeof(temp));
		    int i;
		    // short training symbol
		    temp[4].re = temp[4].im = - sts_mod ;
		    temp[8].re = temp[8].im = - sts_mod ;
		    temp[12].re = temp[12].im = sts_mod ;
		    temp[16].re = temp[16].im = sts_mod ;
		    temp[20].re = temp[20].im = sts_mod ;
		    temp[24].re = temp[24].im = sts_mod ;

		    temp[fft_size-24].re = temp[fft_size-24].im = sts_mod ;
		    temp[fft_size-20].re = temp[fft_size-20].im = - sts_mod ;
		    temp[fft_size-16].re = temp[fft_size-16].im = sts_mod ;
		    temp[fft_size-12].re = temp[fft_size-12].im = - sts_mod ;
		    temp[fft_size-8].re  = temp[fft_size-8].im  = - sts_mod ;
		    temp[fft_size-4].re  = temp[fft_size-4].im  = sts_mod ;

		    IFFTSSEEx<fft_size>((vcs*)temp);

		    rep_shift_right<fft_size/vcs::size>((vcs*)temp, (const vcs*)temp, shift_len);

		    for (i = 0; i < fft_size; i++) {
			    lut[i] = temp[FFTLUTMapTable<fft_size>(i)];
		    }

		    // this is an explicit overflow
			memcpy ( &lut[fft_size], lut, (ts_len - fft_size)*sizeof(COMPLEX16));

		    // Windowing
		    lut[0].re >>= 1; lut[0].im >>= 1;
		    lut[1].re >>= 1; lut[1].im >>= 1;

		    lut[ts_len-2].re >>= 1; lut[ts_len-2].im >>= 1;
		    lut[ts_len-1].re >>= 1; lut[ts_len-1].im >>= 1;

			// second sts
#ifdef DOUBLE_STS
			memcpy ( &lut[ts_len], lut, ts_len * sizeof(COMPLEX16));
#endif

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
				    temp[i+fft_size-64].re = lts_mod;
			    else temp[i+fft_size-64].re = -lts_mod;
			    temp[i+fft_size-64].im = 0;
		    }

		    IFFTSSEEx<fft_size>((vcs*)temp);
		    rep_shift_right<fft_size/vcs::size>((vcs*)temp, (const vcs*)temp, shift_len);

		    for (i = 0; i < fft_size; i++) {
			    lut[i+STS_N*ts_len+lts_cp] = temp[FFTLUTMapTable<fft_size>(i)];
		    }

			// Repeat LTS
			memcpy ( &lut[STS_N*ts_len + lts_cp + fft_size], &lut[STS_N*ts_len + lts_cp], fft_size * sizeof( COMPLEX16)); 
		    
		    // GI2
			memcpy ( &lut[STS_N*ts_len], &lut[STS_N*ts_len + STS_N*fft_size], lts_cp * sizeof(COMPLEX16));
		    
		    // Windowing
		    lut[STS_N*ts_len].re >>= 1;   lut[STS_N*ts_len].im >>= 1;
		    lut[STS_N*ts_len+1].re >>= 1; lut[STS_N*ts_len+1].im >>= 1;

		    lut[STS_N*ts_len+ts_len-2].re >>= 1; lut[STS_N*ts_len+ts_len-2].im >>= 1;
		    lut[STS_N*ts_len+ts_len-1].re >>= 1; lut[STS_N*ts_len+ts_len-1].im >>= 1;

        }
	};
    const static_wrapper<COMPLEX16 [STS_N*ts_len + ts_len], _init_lut> m_lut;
	
public:
    DEFINE_OPORT(COMPLEX16, STS_N*ts_len + ts_len);
	
public:
    REFERENCE_LOCAL_CONTEXT(TTSIstanbulSrc10);
    STD_TSOURCE_CONSTRUCTOR(TTSIstanbulSrc10) 
		BIND_CONTEXT (CF_Error::error_code,	   error_code ) 
	{
	}
		
    STD_TSOURCE_RESET() {}
	
    STD_TSOURCE_FLUSH() { }

    bool Process ()
    {
       	COMPLEX16* po;
		po = opin().append();
		memcpy ( po, m_lut.get(), sizeof(COMPLEX16) * (STS_N*ts_len + ts_len));
		Next()->Process(opin());
    	return FALSE;		
    }
};

//
// Defines Instanbul Wireless Training Symbols - 5MHz
// 10 STS + 10 STS + 2 LTS
//
DEFINE_LOCAL_CONTEXT(TTSIstanbulSrc5, CF_Error);
template<TSOURCE_ARGS>
class TTSIstanbulSrc5: public TSource<TSOURCE_PARAMS>
{
private:
	static const ushort sts_mod = (ushort) (1.0 * bpsk_mod_11a * 1.472);
	static const ushort lts_mod = bpsk_mod_11a;
	static const int    fft_size	= 512;
	static const int    shift_len	= 2;
	static const int    ts_len      = 1280;
	static const int    lts_cp      = 256;
private:
	CTX_VAR_RW (ulong, error_code );     /* error code for the modulation  */	
	
private:
    struct _init_lut
    {
        void operator()(COMPLEX16 (&lut) [STS_N*ts_len + ts_len]) // STS + STS + LTS
        {
		    A16 COMPLEX16 temp [fft_size];

		    memset (temp, 0, sizeof(temp));
		    int i;
		    // short training symbol
		    temp[4].re = temp[4].im = - sts_mod ;
		    temp[8].re = temp[8].im = - sts_mod ;
		    temp[12].re = temp[12].im = sts_mod ;
		    temp[16].re = temp[16].im = sts_mod ;
		    temp[20].re = temp[20].im = sts_mod ;
		    temp[24].re = temp[24].im = sts_mod ;

		    temp[fft_size-24].re = temp[fft_size-24].im =   sts_mod ;
		    temp[fft_size-20].re = temp[fft_size-20].im = - sts_mod ;
		    temp[fft_size-16].re = temp[fft_size-16].im =   sts_mod ;
		    temp[fft_size-12].re = temp[fft_size-12].im = - sts_mod ;
		    temp[fft_size-8].re  = temp[fft_size-8].im  = - sts_mod ;
		    temp[fft_size-4].re  = temp[fft_size-4].im  =   sts_mod ;

		    IFFTSSEEx<fft_size>((vcs*)temp);
		    rep_shift_right<(fft_size/vcs::size)> ((vcs*)temp, (const vcs*)temp, shift_len);

		    for (i = 0; i < fft_size; i++) {
			    lut[i] = temp[FFTLUTMapTable<fft_size>(i)];
		    }

		    // this is an explicit overflow
			memcpy ( &lut[fft_size], lut, (ts_len - fft_size) * sizeof(COMPLEX16));
			
		    // Windowing
		    lut[0].re >>= 1; lut[0].im >>= 1;
		    lut[1].re >>= 1; lut[1].im >>= 1;

		    lut[ts_len-2].re >>= 1; lut[ts_len-2].im >>= 1;
		    lut[ts_len-1].re >>= 1; lut[ts_len-1].im >>= 1;

#ifdef DOUBLE_STS
			// second sts
			memcpy ( &lut[ts_len], lut, ts_len * sizeof(COMPLEX16));
#endif

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
				    temp [i+fft_size-64].re =  lts_mod;
			    else temp[i+fft_size-64].re = -lts_mod;
			    temp[i+fft_size-64].im = 0;
		    }
					
		    IFFTSSEEx<fft_size>((vcs*)temp);
		    rep_shift_right<(fft_size/vcs::size)> ((vcs*)temp, (const vcs*)temp, shift_len);

			for (i = 0; i < fft_size; i++) {
			    lut[i+STS_N*ts_len+lts_cp] = temp[FFTLUTMapTable<fft_size>(i)];
		    }

			// Repeat LTS
			memcpy ( &lut[STS_N*ts_len + lts_cp + fft_size], &lut[STS_N*ts_len + lts_cp], fft_size * sizeof( COMPLEX16)); 
		    
		    // GI2
			memcpy ( &lut[STS_N*ts_len], &lut[STS_N*ts_len + 2*fft_size], lts_cp * sizeof(COMPLEX16));
		    
		    // Windowing
		    // Windowing
		    lut[STS_N*ts_len].re >>= 1;   lut[STS_N*ts_len].im >>= 1;
		    lut[STS_N*ts_len+1].re >>= 1; lut[STS_N*ts_len+1].im >>= 1;

		    lut[STS_N*ts_len+ts_len-2].re >>= 1; lut[STS_N*ts_len+ts_len-2].im >>= 1;
		    lut[STS_N*ts_len+ts_len-1].re >>= 1; lut[STS_N*ts_len+ts_len-1].im >>= 1;

        }
	};
    const static_wrapper<COMPLEX16 [STS_N*ts_len + ts_len], _init_lut> m_lut;
	
public:
    DEFINE_OPORT(COMPLEX16, STS_N*ts_len + ts_len);
	
public:
    REFERENCE_LOCAL_CONTEXT(TTSIstanbulSrc5);
    STD_TSOURCE_CONSTRUCTOR(TTSIstanbulSrc5) 
		BIND_CONTEXT (CF_Error::error_code,	   error_code ) 
	{
	}
		
    STD_TSOURCE_RESET() {}
	
    STD_TSOURCE_FLUSH() { }

    bool Process ()
    {
       	COMPLEX16* po;
		po = opin().append();

		memcpy ( po, m_lut.get(), sizeof(COMPLEX16) * (STS_N*ts_len + ts_len));

		Next()->Process(opin());
    	return FALSE;		
    }
};
//
// IFFT4x - 4x Oversampled IFFT and add GI - 10M
//
DEFINE_LOCAL_CONTEXT(TIFFT4x, CF_VOID);
template<TFILTER_ARGS>
class TIFFT4x: public TFilter<TFILTER_PARAMS>
{
	static const int shift_len = 2;
public:
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(COMPLEX16, 256 + 64);
	
public:
    REFERENCE_LOCAL_CONTEXT(TIFFT4x);
    STD_TFILTER_CONSTRUCTOR(TIFFT4x) { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *in  = (vcs*) ipin.peek();
            vcs *out = (vcs*) opin().append();

            oversampled_ifft (in, out+16);
			
			// add GI
			rep_memcpy<16> (out, out+64);

			// Windowing here
			COMPLEX16* pout = (COMPLEX16*) out;
			
			pout[0].re >>= 1; pout[0].im >>= 1;
			pout[1].re >>= 1; pout[1].im >>= 1;

			pout[254+64].re >>= 1; pout[254+64].im >>= 1;
			pout[255+64].re >>= 1; pout[255+64].im >>= 1;

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

	FINL
	void oversampled_ifft (vcs * pcInput, vcs * pcOutput)
	{
		vcs temp[256 / vcs::size];
		
		rep_memcpy<8> (temp, pcInput );
		rep_memcpy<8> (&temp[64-8], &pcInput[8]);
		rep_memzero<32> (&temp[8]);

		IFFT<256> (temp, pcOutput );

		rep_shift_right<64> (pcOutput, pcOutput, shift_len);	
	}
	
};

//
// TDownSample4 
// DownSample the stream by two
//
DEFINE_LOCAL_CONTEXT(TDownSample4, CF_VOID);
template<TFILTER_ARGS>
class TDownSample4 : public TFilter<TFILTER_PARAMS>
{
public: // inport and outport
    DEFINE_IPORT(COMPLEX16, 16);
    DEFINE_OPORT(COMPLEX16, 4);

public:
    REFERENCE_LOCAL_CONTEXT(TDownSample4);
    STD_TFILTER_CONSTRUCTOR(TDownSample4)
    {
    }
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}	

    BOOL_FUNC_PROCESS(ipin)
    {
        // markit!
//        COMPLEX16* output;
        while (ipin.check_read())
        {
            vcs* pi = (vcs*) ipin.peek();

//			vcs t1 = permutate<0,2,0,2>(pi[0]);
//			vcs t2 = permutate<0,2,0,2>(pi[1]);
			
			vcs t1 = interleave_low (pi[0], pi[1]);
			vcs t2 = interleave_low (pi[2], pi[3] );

			vcs& o = cast_ref<vcs> (opin().append ());
			o = (vcs)(interleave_low ((vcui&)t1, (vcui&)t2));
	
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};

//
// TDownSample8 
// DownSample the stream by two
//
DEFINE_LOCAL_CONTEXT(TDownSample8, CF_VOID);
template<TFILTER_ARGS>
class TDownSample8 : public TFilter<TFILTER_PARAMS>
{
public: // inport and outport
    DEFINE_IPORT(COMPLEX16, 32);
    DEFINE_OPORT(COMPLEX16, 4);

public:
    REFERENCE_LOCAL_CONTEXT(TDownSample8);
    STD_TFILTER_CONSTRUCTOR(TDownSample8)
    {
    }
	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}	

    BOOL_FUNC_PROCESS(ipin)
    {
        // markit!
//        COMPLEX16* output;
        while (ipin.check_read())
        {
            const COMPLEX16* pi = ipin.peek();
			COMPLEX16* po = opin().append ();
			po[0] = pi[0];
			po[1] = pi[8];
			po[2] = pi[16];
			po[3] = pi[24];
				
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};