#pragma once

#include "const.h"
#include "sora.h"

FINL BOOL ParseSignal(
        unsigned int uiSignal, 
        unsigned short * pusLength,
        unsigned char * pbRate,
        unsigned int * puiDBPS)
{
    unsigned int uiParity;
    // signal rate look up table
    static const unsigned int g11a_rguiDBPSLookUp[16] = {
        /* R1-4 */
        /* 0000 */ 0,
        /* 1000 */ 0,
        /* 0100 */ 0,
        /* 1100 */ 0,
        /* 0010 */ 0,
        /* 1010 */ 0,
        /* 0110 */ 0,
        /* 1110 */ 0,

        /* 0001 */ 192,
        /* 1001 */ 96,
        /* 0101 */ 48,
        /* 1101 */ 24,
        /* 0011 */ 216,
        /* 1011 */ 144,
        /* 0111 */ 72,
        /* 1111 */ 36,
    };


    uiSignal &= 0xFFFFFF;
    if (uiSignal & 0xFC0010) // all these bits should be always zero
        return FALSE;
    
    uiParity = (uiSignal >> 16) ^ (uiSignal);
    uiParity = (uiParity >> 8) ^ (uiParity);
    uiParity = (uiParity >> 4) ^ (uiParity);
    uiParity = (uiParity >> 2) ^ (uiParity);
    uiParity = (uiParity >> 1) ^ (uiParity);
    if (uiParity & 0x1)
        return FALSE;

    (*pbRate) = uiSignal & 0xF;
    if (!((*pbRate) & 0x8))
        return FALSE;

    (*pusLength) = (uiSignal >> 5) & 0xFFF;
    (*puiDBPS) = g11a_rguiDBPSLookUp[*pbRate];

    return TRUE;
}
