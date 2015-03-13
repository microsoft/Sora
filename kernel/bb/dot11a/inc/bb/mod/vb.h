// Define classes for viterbi buffer

#pragma once
#include <assert.h>
#include "const.h"

template<size_t VB_DCSIZE>
struct VB_DCBLOCK
{
    volatile MEM_ALIGN(64) char isValid;
    MEM_ALIGN(4) unsigned char data[VB_DCSIZE];
};

template<size_t VB_DCSIZE, size_t VB_DCCOUNT>
struct VB
{
    typedef VB_DCBLOCK<VB_DCSIZE> VB_DCBLOCK;
    const static size_t BlockCount = VB_DCCOUNT;
private:
    VB_DCBLOCK * w_currentBlock;
    VB_DCBLOCK * w_itBlock;

    MEM_ALIGN(64) VB_DCBLOCK blocks[VB_DCCOUNT];
public:
    VB_DCBLOCK *BlocksBegin()
    {
        return blocks;
    }

    VB_DCBLOCK *BlocksEnd()
    {
        return blocks + VB_DCCOUNT;
    }

    void SpaceWait(size_t nBlock, volatile char *pbWorkIndicator)
    {
        VB_DCBLOCK *lastBlock = (w_currentBlock - blocks + nBlock - 1) % VB_DCCOUNT + blocks;

        while ((lastBlock->isValid & 0x1) && *pbWorkIndicator)
            _mm_pause();
    }

    unsigned char *Push()
    {
        unsigned char *ret = w_itBlock->data;

        w_itBlock++;
        if (w_itBlock == blocks + VB_DCCOUNT)
            w_itBlock = blocks;
        return ret;
    }

    void Flush()
    {
        w_currentBlock->isValid = 1;
        w_currentBlock++;
        if (w_currentBlock == blocks + VB_DCCOUNT)
            w_currentBlock = blocks;
    }

    void Clear()
    {
        unsigned int i;
        w_itBlock = blocks;
        w_currentBlock = blocks;
        for (i = 0; i < VB_DCCOUNT; i++)
        {
            blocks[i].isValid = 0;
        }
    }
};
