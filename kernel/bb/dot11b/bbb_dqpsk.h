/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_dqpsk.h

Abstract: 802.11b Differential Quadrant Phase Shift Keying (DQPSK) demap bit macro. 

History: 
          7/7/2009: Modified by senxiang.
--*/

#ifndef _BBB_DQPSK_H
#define _BBB_DQPSK_H

#define DEMAP_BIT_DQPSK(bOutput, uiPosition, cLast, cThis) \
{ \
    COMPLEX32 cTemp; \
    cTemp.re = (cLast).re * (cThis).re + (cLast).im * (cThis).im; \
    cTemp.im = (cLast).re * (cThis).im - (cLast).im * (cThis).re; \
    (bOutput) |= (unsigned long)(cTemp.re + cTemp.im) >> 31 << (uiPosition); \
    (bOutput) |= (unsigned long)(cTemp.re - cTemp.im) >> 31 << ((uiPosition) + 1); \
} \

#endif//_BBB_DQPSK_H

