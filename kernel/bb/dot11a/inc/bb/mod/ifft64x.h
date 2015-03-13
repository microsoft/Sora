#pragma once

#include "ifft_r4dif.h"

__forceinline
void IFFT64x(vcs * pcInput, vcs * pcOutput)
{
    vcs temp[128 / vcs::size];

    memset(temp, 0, sizeof(temp));
    memcpy(temp, pcInput, 32 * sizeof(COMPLEX16));
    memcpy(temp + 96 / vcs::size, pcInput + 32 / vcs::size, 32 * sizeof(COMPLEX16));

    IFFTSSEEx<128>(temp);

    int i;
    for (i = 0; i < 128; i++)
        ((COMPLEX16*)pcOutput)[i] = ((COMPLEX16*)temp) [FFTLUTMapTable<128>(i)];

    for (i = 0; i < 128 / vcs::size; i++)
        pcOutput[i] = shift_left(pcOutput[i], 2);


}

__forceinline
void IFFT64x(COMPLEX16 * pcInput, COMPLEX16 * pcOutput)
{
    IFFT64x((vcs*)pcInput, (vcs*)pcOutput);
}
