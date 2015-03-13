#pragma once

#include "vector128.h"

DSP_INLINE
void Pack_NT(vb *out, const vs& a, const vs& b, int nbits)
{
    if (nbits == 0)
        // The branch is only to optimize compiler code generation, ie. remove shift_right instructions
        store_nt(out, saturated_pack(a, b));
    else
        store_nt(out, saturated_pack(shift_right(a, nbits), shift_right(b, nbits)));
}

DSP_INLINE
void Copy_NT(COMPLEX8 * pcOutput, const COMPLEX16 * pcInput, unsigned int uiCount, int nbits = 6)
{
    const vs * pvInput = (vs*)pcInput;
    vb * pvOutput = (vb*)pcOutput;
    for (unsigned int i = 0; i < uiCount >> 3; i++)
    {
        Pack_NT(&pvOutput[0], pvInput[0], pvInput[1], nbits);
        pvInput += 2;
        pvOutput++;
    }
}
