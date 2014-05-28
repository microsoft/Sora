/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_dbpsk.h

Abstract: 802.11b Differential Binary Phase Shift Keying (DBPSK) demap bit macro. 

History: 
          7/7/2009: Modified by senxiang.
--*/

#pragma once

// Demap DBPSK symbol and store the bit into one bit field of the output byte
// Parameters:
//   bOutput: output byte
//   uiPosition: the bit field index in the output byte
//   cLast: previous DBPSK symbol, a complex number used as reference for demapping
//   cThis: current DBPSK symbol, also a complex number
#define DEMAP_BIT_DBPSK(bOutput, uiPosition, cLast, cThis) \
{\
    bOutput |= (unsigned char)\
        ((unsigned long)((cLast).re * (cThis).re + (cLast).im * (cThis).im) >> 31) \
        << (uiPosition); \
}
