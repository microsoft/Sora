#pragma once

#include <stdio.h> 
#include "bb/bba.h"
#include "vector128.h"
#include "lut.h"
#include "../../../../../brick/inc/bb_debug.h"

__forceinline
unsigned short 
EstimateFreqOffset(
    IN const COMPLEX16 * pc, 
    OUT short FreqFactorCos[2 * FREQ_LUT_SIZE], 
    OUT short FreqFactorSin[2 * FREQ_LUT_SIZE]
    )
{
    int iCorrRe = 0, iCorrIm = 0;
    int i;
    int iFreq;
    unsigned short usTheta;

    // Hint:
    //  It is not completely normalized. But it should be okay. 
    //
    for (i = 0; i < 64; i++)
    {
        int re1 = pc[i].re;
        int im1 = pc[i].im;
        // The most conservative option is shifting 6 bits.
        // In the worst case, iCorrRe will get 2^15 * 2^15 * 128 = 2^37.
        // The value is a trade-off between range and accuracy and determined in real system debugging
        iCorrRe += ( re1 * pc[i + 64].re + im1 * pc[i + 64].im) >> 5;
        iCorrIm += ( re1 * pc[i + 64].im - im1 * pc[i + 64].re) >> 5;
    }

#if BB_DBG
	printf ( "Sum re im <%d, %d>\n", iCorrRe, iCorrIm );
#endif
	
    iCorrRe >>= 10;
    
    if (iCorrRe == 0) {
        if ( iCorrIm > 0 ) iFreq = 2048;
        else if ( iCorrIm < 0 ) iFreq = -2048;
        else iFreq = 0;
    } else {
        iFreq = (iCorrIm / iCorrRe);
    }    
	
    
    // Hint:
    //   In this implementation the maximal 64 times theta is 63 degree=arctan(2047/1024)
    //   iFreq == 1024 * tan ( 64 * theta )
    //   theta == ATAN64 ( iFreq );
    // Note: ATAN64(x) = arctan(x/1024) / 64 / 2pi * 65536
    // Note: The maximal frequency offset is arttan(2047/1024) / 64 / 2pi * 20 MHz = 55.055 kHz, 
    //   presuming input sampled at 20MHz 
    
    // printf ("EstimateFreq  re:%d im:%d tan:%d\n", iCorrRe, iCorrIm, iFreq);
    
    if (iFreq >= 0)
    {
        // Wrap if too large to prevent buffer reading overrun
        iFreq &= (ATAN64_LUT_INPUT_SIZE - 1);
        iFreq = *ATAN64(iFreq);

    }
    else
    {
        iFreq = -iFreq;
        // Wrap if too large to prevent buffer reading overrun
        iFreq &= (ATAN64_LUT_INPUT_SIZE - 1);
        iFreq = -(*(ATAN64(iFreq))); // 65536 = 2pi
        
    }


    _dump_text ("EstimateFreq  atan:%d %8.6lf\n", iFreq, 
            - (double) (iFreq) * 2 * 3.141593 / 0xFFFF);


    // Hint: 
    //   if iFreq is negtive, it is automatically convert to 2 pi + theta
    //   note 2 pi == 65536
    //
    iFreq &= 0xFFFF; // prevenet sign extension
    
    // Inintialize FreqFactorCos/FreqFactorSin for FreqComp64()
    // This initialization pattern is a common optimization for complex number multiplication with a constant.
    usTheta = 0;
    for (i = 0; i < FREQ_LUT_SIZE * 2; i += 2)
    {
        FreqFactorCos[i]   = *(COS0xFFFF(usTheta));
        FreqFactorCos[i+1] = *(COS0xFFFF(usTheta));
        FreqFactorSin[i]   = *(SIN0xFFFF(usTheta));
        FreqFactorSin[i+1] = -(*(SIN0xFFFF(usTheta)));
        usTheta += (unsigned short)(iFreq); // Automatically overflowed
    }
    return (unsigned short)(iFreq);
}

DSP_INLINE vcs rotate(const vcs& a, const vs& cos_cos, const vs& sin_nsin)
{
    vcs m1 = (vcs)mul_high((vs&)a, cos_cos);
    vcs m2 = (vcs)mul_high((vs&)flip(a), sin_nsin);
    return add(m1, m2);
}

inline void 
FreqComp64(
    OUT vcs * pc,
    IN unsigned short Freq,
    IN const vs FreqFactorCos[32],
    IN const vs FreqFactorSin[32])
{
    if (Freq == 0) return;

    for (int i = 0; i < 16; i++)
    {
        pc[i] = rotate(pc[i], FreqFactorCos[i], FreqFactorSin[i]);
    }
}

inline void 
FreqComp128(
    OUT vcs * pc,
    IN unsigned short Freq,
    IN vs FreqFactorCos[32],
    IN vs FreqFactorSin[32])
{
    if (Freq == 0) return;

    for (int i = 0; i < 32; i++)
    {
        pc[i] = rotate(pc[i], FreqFactorCos[i], FreqFactorSin[i]);
    }
}
