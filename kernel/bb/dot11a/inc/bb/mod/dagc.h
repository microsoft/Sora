// Functions for digital AGC(Automatic Gain Control) logic
#pragma once
#include "const.h"
#include "vector128.h"

#if 0

template<size_t N>
FINL void DigitalAGC (vcs * pSamples, int digitalAGC)
{

    if (digitalAGC > 0)
    {
        // shift right
        for (int i=0; i<N; i++)
        {
            pSamples[i] = shift_right ( pSamples[i], digitalAGC );
        }
    }
    else if (digitalAGC < 0)
    {
        // shift left
        for (int i=0; i<N; i++)
        {
            pSamples[i] = shift_left ( pSamples[i], -digitalAGC );
        }
    } // else zero
}
#endif

