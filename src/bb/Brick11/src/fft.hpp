#pragma once

#include <fft.h>
//
// IFFTx - Oversampled IFFT and add GI 
//
DEFINE_LOCAL_CONTEXT(TIFFTx, CF_VOID);
template<TFILTER_ARGS>
class TIFFTx: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(COMPLEX16, 128 + 32);
	
public:
    REFERENCE_LOCAL_CONTEXT(TIFFTx);
    STD_TFILTER_CONSTRUCTOR(TIFFTx) { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *in  = (vcs*) ipin.peek();
            vcs *out = (vcs*) opin().append();

            oversampled_ifft (in, out+8);
			
			// add GI
			rep<8>::vmemcpy (out, out+32);

			// Windowing here
			COMPLEX16* pout = (COMPLEX16*) out;
			pout[0].re >>= 1; pout[0].im >>= 1;
			pout[1].re >>= 1; pout[1].im >>= 1;

			pout[158].re >>= 1; pout[158].im >>= 1;
			pout[159].re >>= 1; pout[159].im >>= 1;

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

	FINL
	void oversampled_ifft (vcs * pcInput, vcs * pcOutput)
	{
		vcs temp[128 / vcs::size];
		
		rep<8>::vmemcpy   (temp, pcInput );
		rep<8>::vmemcpy   (&temp[24], &pcInput[8]);
		rep<16>::vmemzero (&temp[8]);

		IFFT<128> (temp, pcOutput );

		rep<32>::vshift_right (pcOutput, 4);	
	}
	
};

DEFINE_LOCAL_CONTEXT(TIFFTxOnly, CF_VOID);
template<TFILTER_ARGS>
class TIFFTxOnly: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(COMPLEX16, 128);
	
public:
    REFERENCE_LOCAL_CONTEXT(TIFFTxOnly);
    STD_TFILTER_CONSTRUCTOR(TIFFTxOnly) { }
    STD_TFILTER_RESET() { }
    STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *in  = (vcs*) ipin.peek();
            vcs *out = (vcs*) opin().append();

            oversampled_ifft (in, out);
			
            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

	FINL
	void oversampled_ifft (vcs * pcInput, vcs * pcOutput)
	{
		vcs temp[128 / vcs::size];
		
		rep<8>::vmemcpy   (temp, pcInput );
		rep<8>::vmemcpy   (&temp[24], &pcInput[8]);
		rep<16>::vmemzero (&temp[8]);

		IFFT<128> (temp, pcOutput );
	}
};

//
// FFTx - 
//
DEFINE_LOCAL_CONTEXT(TFFT64, CF_VOID);
template<TFILTER_ARGS>
class TFFT64: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(COMPLEX16, 64);
	
public:
    REFERENCE_LOCAL_CONTEXT(TFFT64);
    STD_TFILTER_CONSTRUCTOR(TFFT64) { }
    STD_TFILTER_RESET() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *in  = (vcs*) ipin.peek();
            vcs *out = (vcs*) opin().append();

			FFT<64>(in, out);

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};

//
// FFTx - 
//
DEFINE_LOCAL_CONTEXT(TIFFT128, CF_VOID);
template<TFILTER_ARGS>
class TIFFT128: public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(COMPLEX16, 128);
    DEFINE_OPORT(COMPLEX16, 128);
	
public:
    REFERENCE_LOCAL_CONTEXT(TIFFT128);
    STD_TFILTER_CONSTRUCTOR(TIFFT128) { }
    STD_TFILTER_RESET() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            vcs *in  = (vcs*) ipin.peek();
            vcs *out = (vcs*) opin().append();

			IFFT<128>(in, out);

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }
};
