#pragma once
#include "const.h"

#define LB1_MAX     2 * 1024

// Linear FIFO, ie. all items in the FIFO are stored in sequential memory
template<class TYPE, size_t FIELD_SIZE = LB1_MAX/sizeof(TYPE)>
class SoraLinearFifo1
{
    TYPE data[FIELD_SIZE];

    // reader and writer
    volatile unsigned int w_count;
    volatile unsigned int r_count;

public:
    /*
    SoraLinearFifo()
        : w_count(0), r_count(0)
    {
    }
    */

    FINL bool RCheck(size_t n)
    {
        return w_count - r_count >= n;
    }

    FINL void Pop(size_t n)
    {
        r_count += n;
        if (r_count == w_count)
            r_count = w_count = 0;
        if (r_count > FIELD_SIZE / 2)
        {
            memcpy(data + r_count - FIELD_SIZE / 2, data + r_count, (w_count - r_count) * sizeof(TYPE));
            r_count -= FIELD_SIZE / 2;
            w_count -= FIELD_SIZE / 2;
        }
    }

    FINL TYPE *Peek()
    {
        return data + r_count;
    }

    FINL TYPE *Push(size_t n)
    {
        TYPE *ret = data + w_count;
        w_count += n;
        return ret;
    }

    FINL void Clear()
    {
        w_count = r_count = 0;
    }
};
