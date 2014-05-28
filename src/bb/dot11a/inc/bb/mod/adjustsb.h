#ifndef BB_ADJUSTSB_H
#define BB_ADJUSTSB_H

#include <assert.h>

#include "vector128.h"
__forceinline
void ShiftRight2(SignalBlock& block)
{
    //size_t i;
    //for (i = 0; i < block.size; i++)
    //    block[i] = shift_right(block[i], 2);
    assert(block.size == 7);
    block[0] = shift_right(block[0], 2);
    block[1] = shift_right(block[1], 2);
    block[2] = shift_right(block[2], 2);
    block[3] = shift_right(block[3], 2);
    block[4] = shift_right(block[4], 2);
    block[5] = shift_right(block[5], 2);
    block[6] = shift_right(block[6], 2);
}

#endif//BB_ADJUSTSB_H
