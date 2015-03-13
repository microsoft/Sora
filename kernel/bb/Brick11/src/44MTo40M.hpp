#pragma once
/*
Copyright (c) Microsoft Corporation

Abstract:   Class definition for software resample from 44MHz to 40MHz sampling rate, 
            and vice versus

*/
#include <bb/bba.h>
#include "vector128.h"

class Down44to40
{
   MEM_ALIGN(16) COMPLEX16 OutStream[84];
   COMPLEX32        LastIQRight;
   int              LastIQIndex;

   int              start;
   int              length;

private:
    void Move();
    
public :

    COMPLEX16 * GetOutStream(int nIQs);
    Down44to40();
    void Resample(const SignalBlock& block);
};

#define MULTIPLE_SHIFT  7
#define MULTIPLE    (1 << MULTIPLE_SHIFT) //128

#pragma warning(disable: 4244)
static int LinearTableR[11] = 
    {1, 115, 102, 90, 77, 64, 51, 38, 26, 13, 0};

static int LinearTableL[11] = 
    {0, 0, 13, 26, 38, 51, 64, 77, 90, 102, 115};

#pragma warning(default: 4244)
    
FINL Down44to40::Down44to40(): start(0), length(0), LastIQIndex(-1)
{
    this->LastIQRight.im = 0;
    this->LastIQRight.re = 0;
}

FINL void Down44to40::Move()
{
    if (start > 0)
    {
        for (int i = 0; i < length; i++)
        {
            OutStream[i] = OutStream[i + start];
        }
        //printf("Move %d IQs from position %d to 0\n", length, start);
        start = 0;
        
    }
}

FINL COMPLEX16 * Down44to40::GetOutStream(int nIQs)
{
    if (length >= nIQs)
    {
        //printf("%d downsampled IQs available\n", nIQs);
        start += nIQs;
        length -= nIQs;
        return OutStream;
    }
    else
    {
        return NULL;
    }
}

FINL void Down44to40::Resample(const SignalBlock& block)
{
    const COMPLEX16 *pIQ = &block[0][0];
    
    int re_left, im_left = 0;
    int index11;
    
    Move();

    for (int i = 0; i < SORA_RX_SIGNAL_UNIT_COMPLEX16_NUM * SORA_RX_SIGNAL_UNIT_NUM_PER_DESC; i++)
    {
        index11 = (LastIQIndex + i + 1) % 11;
        
        if (index11 > 1)
        {
            //printf(" %d/28 (%d/11) IQ (%d,%d)*%02f coming, ", i, index11, pIQ[i].re, pIQ[i].im, (float)(LinearTableL[index11])/128);
            re_left = pIQ[i].re * LinearTableL[index11];
            im_left = pIQ[i].im * LinearTableL[index11];

            OutStream[length].re = (LastIQRight.re + re_left) >> MULTIPLE_SHIFT;
            OutStream[length].im = (LastIQRight.im + im_left) >> MULTIPLE_SHIFT;
            //printf("--> %d: (%d, %d)\n", length, OutStream[length].re, OutStream[length].im);
            length++;
            LastIQRight.re = pIQ[i].re * LinearTableR[index11];
            LastIQRight.im = pIQ[i].im * LinearTableR[index11];
            //printf(" %d/28 (%d, %d) * %02f + ", i, pIQ[i].re, pIQ[i].im, (float)LinearTableR[index11] / 128);
            
        }
        else if (index11 == 0)
        {
            OutStream[length] = pIQ[i];
            //printf(" prev discarded, %d/28 %d/11 (%d, %d) --> %d: (%d, %d)\n", i, index11, pIQ[i].re, pIQ[i].im, length, OutStream[length].re, OutStream[length].im);
            length++;

        }
        else if (index11 == 1)
        {
            LastIQRight.re = pIQ[i].re * LinearTableR[1];
            LastIQRight.im = pIQ[i].im * LinearTableR[1];
            //printf("%d/28 %d/11 (%d, %d)*%02f + ", i, index11, pIQ[i].re, pIQ[i].im, (float)LinearTableR[1] / 128);
        }
    }
    
    LastIQIndex = index11;

}
