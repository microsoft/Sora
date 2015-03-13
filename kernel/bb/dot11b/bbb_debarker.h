/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_debarker.h

Abstract: Despread barker macro.

History: 
          7/7/2009: Modified by senxiang.
--*/
#pragma once
#include "complex_ext.h"

// Note: make OFFSET template parameter instead of function parameter, in order to optimize speed
FINL COMPLEX32 DESPREAD_BARKER(COMPLEX16 pcInputBase[11])
{
    const int c[11] = {
        1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0,
    };

    COMPLEX32 cResult;
    cResult.re = cResult.im = 0;
    c[0]? cResult += pcInputBase[0] : cResult -= pcInputBase[0];
    c[1]? cResult += pcInputBase[1] : cResult -= pcInputBase[1];
    c[2]? cResult += pcInputBase[2] : cResult -= pcInputBase[2];
    c[3]? cResult += pcInputBase[3] : cResult -= pcInputBase[3];
    c[4]? cResult += pcInputBase[4] : cResult -= pcInputBase[4];
    c[5]? cResult += pcInputBase[5] : cResult -= pcInputBase[5];
    c[6]? cResult += pcInputBase[6] : cResult -= pcInputBase[6];
    c[7]? cResult += pcInputBase[7] : cResult -= pcInputBase[7];
    c[8]? cResult += pcInputBase[8] : cResult -= pcInputBase[8];
    c[9]? cResult += pcInputBase[9] : cResult -= pcInputBase[9];
    c[10]? cResult += pcInputBase[10] : cResult -= pcInputBase[10];
    return cResult;
}
