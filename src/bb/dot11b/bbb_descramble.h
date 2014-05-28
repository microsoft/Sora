/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_descramble.h

Abstract: descramble macro.

History: 
          7/7/2009: Modified by senxiang.
--*/

#ifndef _BB_DESCRAMBLE_H
#define _BB_DESCRAMBLE_H

extern unsigned char LUT_BBBDESCRAMBLE[256][128];

#define DESCRAMBLE(bOutput, bThis, bLast) \
{ \
    (bOutput) = LUT_BBBDESCRAMBLE[bThis][bLast >> 1]; \
} \

#endif // _BB_DESCRAMBLE_H
