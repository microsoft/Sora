#pragma once
#include <assert.h>
#include "vector128.h"
#include "complex_ext.h"
#include "bb/bba.h"

#define DCHIS_MASK          (DCHIS_LEN - 1)
#define DCHIS_SHIFT         4

DSP_INLINE1 void ResetDC(PBB11A_RX_CONTEXT pRxContextA)
{
    int i;
    for (i = 0; i < DCHIS_LEN; i++)
    {
        pRxContextA->dcHistory[i].re = 0;
        pRxContextA->dcHistory[i].im = 0;
    }

    pRxContextA->dcSum.re = 0;
    pRxContextA->dcSum.im = 0;

    pRxContextA->dcHistoryCounter = 0;
    pRxContextA->dcLearningCounter = 0;

    set_zero((vcs&)pRxContextA->dcEstimated);
}

DSP_INLINE1 void EstimateDC(PBB11A_RX_CONTEXT pRxContextA, const SignalBlock& inBlock)
{
    if (pRxContextA->dcLearningCounter++ >= DCHIS_LEN)
    {
        if (pRxContextA->dcLearningCounter != DCHIS_LEN + 7) return;
        pRxContextA->dcLearningCounter = DCHIS_LEN;
    } 

    // rewrite the DC Estimation 
    // taking care of overflow as well as AGC
    // taking sample by the first 4 Vectors

    vcs d1, d2, d3;

    // update DC
    d1 = add ( inBlock[0], inBlock[1] );
    d2 = add ( inBlock[2], inBlock[3] );
    d1 = shift_right ( d1, 4 );
    d2 = shift_right ( d2, 4 );
    d3 = add ( d1, d2 );
    vcs sum = hadd ( d3 );

    pRxContextA->dcSum += sum[0];
    pRxContextA->dcSum -= pRxContextA->dcHistory[pRxContextA->dcHistoryCounter];

    pRxContextA->dcHistory[pRxContextA->dcHistoryCounter] = sum[0];
    pRxContextA->dcHistoryCounter++;
    pRxContextA->dcHistoryCounter &= DCHIS_MASK;

    COMPLEX32 dcSumShift = pRxContextA->dcSum >> (DCHIS_SHIFT);
    COMPLEX16 t = { (short)dcSumShift.re, (short)dcSumShift.im };
        
	set_all((vcs&)pRxContextA->dcEstimated, t);

}
