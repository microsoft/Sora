#ifndef _BB_MOD_FETCHDT_H
#define _BB_MOD_FETCHDT_H

#include <bb/bba.h>
#include "vector128.h"
#include "soradsp.h"
#include "44MTo40M.h"
DSP_INLINE void Mem2VcsL(const SignalBlock& block40M, vcs& v0, vcs& v1, vcs& v2, vcs& v3)
{
    const static vcs::data_type m128FirstHalfMask  = { ~0, ~0, ~0, ~0,  0,  0,  0,  0 };
    const static vcs::data_type m128SecondHalfMask = {  0,  0,  0,  0, ~0, ~0, ~0, ~0 };

    vcs vFirstHalfMask = m128FirstHalfMask;
    vcs vSecondHalfMask = m128SecondHalfMask;

    v0 = permutate<0x08>(block40M[0]);
    v0 = and(v0, vFirstHalfMask);               // get two from 4 samples, saved in low xmm0 QWORD
    vcs vTemp = permutate<0x80>(block40M[1]);
    vTemp = and(vTemp, vSecondHalfMask);        // get two from 4 samples, saved in high xmm7 QWORD
    v0 = xor(v0, vTemp);                        // save down sampled 4 samples in xmm0

    v1 = permutate<0x08>(block40M[2]);
    v1 = and(v1, vFirstHalfMask);
    vTemp = permutate<0x80>(block40M[3]);
    vTemp = and(vTemp, vSecondHalfMask);
    v1 = xor(v1, vTemp);

    v2 = permutate<0x08>(block40M[4]);
    v2 = and(v2, vFirstHalfMask);
    vTemp = permutate<0x80>(block40M[5]);
    vTemp = and(vTemp, vSecondHalfMask);
    v2 = xor(v2, vTemp);

    v3 = permutate<0x08>(block40M[6]);
    v3 = and(v3, vFirstHalfMask);
}

DSP_INLINE void Mem2VcsH(const SignalBlock& block40M, vcs& v3, vcs& v4, vcs& v5, vcs& v6)
{
    const static vcs::data_type m128FirstHalfMask  = { ~0, ~0, ~0, ~0,  0,  0,  0,  0 };
    const static vcs::data_type m128SecondHalfMask = {  0,  0,  0,  0, ~0, ~0, ~0, ~0 };

    vcs vFirstHalfMask = m128FirstHalfMask;
    vcs vSecondHalfMask = m128SecondHalfMask;

    vcs vTemp = permutate<0x80>(block40M[0]);
    vTemp = and(vTemp, vSecondHalfMask);
    v3 = xor(v3, vTemp);

    v4 = permutate<0x08>(block40M[1]);
    v4 = and(v4, vFirstHalfMask);
    vTemp = permutate<0x80>(block40M[2]);
    vTemp = and(vTemp, vSecondHalfMask);
    v4 = xor(v4, vTemp);

    v5 = permutate<0x08>(block40M[3]);
    v5 = and(v5, vFirstHalfMask);
    vTemp = permutate<0x80>(block40M[4]);
    vTemp = and(vTemp, vSecondHalfMask);
    v5 = xor(v5, vTemp);

    v6 = permutate<0x08>(block40M[5]);
    v6 = and(v6, vFirstHalfMask);
    vTemp = permutate<0x80>(block40M[6]);
    vTemp = and(vTemp, vSecondHalfMask);
    v6 = xor(v6, vTemp);
}

__forceinline
HRESULT FetchDMADataTouchDownSampled40(
        PSORA_RADIO_RX_STREAM pRxStream, 
        FLAG * pbTouched,
        SignalBlock& block)
{
    HRESULT hr;
    SignalBlock block40M;

    hr = SoraRadioReadRxStream(pRxStream, pbTouched, block40M);
    if (FAILED(hr))
        return hr; // hardware error

    // Downsample from 40MHz to 20MHz
    // Mem2VcsL and Mem2VcsH work together to downsample sequential 2 signal block into one signal block
    Mem2VcsL(block40M, block[0], block[1], block[2], block[3]);
    
    hr = SoraRadioReadRxStream(pRxStream, pbTouched, block40M);
    if (FAILED(hr))
        return hr; // hardware error

    Mem2VcsH(block40M, block[3], block[4], block[5], block[6]);
    return hr;
}

__forceinline
HRESULT FetchDMADataTouchDownSampled44(
        PSORA_RADIO_RX_STREAM pRxStream, 
        FLAG * pbTouched,
        SignalBlock& block)
{
    static CDown44MTo40M Resampler;

    // Downsample from 44MHz to 40MHz
    HRESULT hr ;
    SignalBlock block44M;
    COMPLEX16 *p40MStream;
    while((p40MStream = Resampler.GetOutStream(56)) == NULL)
    {
        hr = SoraRadioReadRxStream(pRxStream, pbTouched, block44M);
        if (FAILED(hr))
            return hr; // hardware error

        Resampler.Resample(block44M);
    }
    
    // Downsample from 40MHz to 20MHz
    // Mem2VcsL and Mem2VcsH work together to downsample sequential 2 signal block into one signal block
    Mem2VcsL(*(SignalBlock*)p40MStream , block[0], block[1], block[2], block[3]);
    Mem2VcsH(*((SignalBlock*)p40MStream + 1), block[3], block[4], block[5], block[6]);
    return hr;
}

#endif
