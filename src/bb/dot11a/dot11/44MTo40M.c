#include "44MTo40M.h"

#define MULTIPLE_SHIFT  7
#define MULTIPLE    (1 << MULTIPLE_SHIFT) //128


#pragma warning(disable: 4244)
static int LinearTableR[11] = 
    {1, 115, 102, 90, 77, 64, 51, 38, 26, 13, 0};

static int LinearTableL[11] = 
    {0, 0, 13, 26, 38, 51, 64, 77, 90, 102, 115};

#pragma warning(default: 4244)
    
CDown44MTo40M::CDown44MTo40M(): start(0), length(0), LastIQIndex(-1)
{
    this->LastIQRight.im = 0;
    this->LastIQRight.re = 0;
}

void CDown44MTo40M::Move()
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

COMPLEX16 * CDown44MTo40M::GetOutStream(int nIQs)
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

void CDown44MTo40M::Resample(const SignalBlock& block)
{
    const COMPLEX16 *pIQ = &block[0][0];
    
    int re_left, im_left, j = 0;
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

#define UP_MULTIPLE_SHIFT   8
static int UpLinearTableL[10] = {   23/*1/11*/, 233/*10/11*/, 209/*9/11*/, 186/*8/11*/, 
                                    163/*7/11*/, 140/*6/11*/, 116/*5/11*/, 93/*4/11*/, 
                                    70/*3/11*/, 47/*2/11*/};
                                /*, 10/11, 9/11, 8/11, 7/11, 6/11, 5/11, 4/11, 3/11, 2/11*/
static int UpLinearTableR[10] = {  23,   47,   70,  93,   116,  140,  163, 186, 209, 233};
                                /*1/11, 2/11, 3/11, 4/11, 5/11, 6/11, 7/11, 8/11, 9/11, 10/11*/

ULONG CUp40MTo44M::Resample(
        OUT COMPLEX8 *pIQBuf44M, 
        IN ULONG n44MBufIQNum, 
        IN COMPLEX8 *pIQBuf40M, 
        IN ULONG n40BufIQNum)
{
    ULONG i;
    ULONG index10;
    
    int re_right;
    int im_right;
    int outLength = 0;

    if (11 * (1 + n40BufIQNum / 10) > n44MBufIQNum || n40BufIQNum == 0) return 0;
    
    //printf("40M[0](%d, %d) --> ", pIQBuf40M[0].re, pIQBuf40M[0].im); 
    pIQBuf44M[outLength++] = pIQBuf40M[0];
    //printf("44M[0](%d, %d) \n", pIQBuf44M[0].re, pIQBuf44M[0].im); 
    re_right = pIQBuf40M[0].re * UpLinearTableR[0];
    im_right = pIQBuf40M[0].im * UpLinearTableR[0];
    //printf("40M[0](%d, %d) * %02f + ", pIQBuf40M[0].re, pIQBuf40M[0].im, (float)UpLinearTableR[0] /256);

    for (i = 1; i < n40BufIQNum; i++)
    {
        index10 = i % 10; //generate 11 from 10
        pIQBuf44M[outLength].re = 
                (re_right + pIQBuf40M[i].re * UpLinearTableL[index10]) >> UP_MULTIPLE_SHIFT;
        pIQBuf44M[outLength].im = 
                (im_right + pIQBuf40M[i].im * UpLinearTableL[index10]) >> UP_MULTIPLE_SHIFT;
        //printf("40M[%d](%d, %d) * %02f = 44M[%d](%d, %d)\n", i, pIQBuf40M[i].re, pIQBuf40M[i].im, (float)UpLinearTableL[index10]/256, outLength, pIQBuf44M[outLength].re, pIQBuf44M[outLength].im);
        outLength++;    
        re_right = pIQBuf40M[i].re * UpLinearTableR[index10];
        im_right = pIQBuf40M[i].im * UpLinearTableR[index10];
        if (index10 == 0)
        {
            pIQBuf44M[outLength++] = pIQBuf40M[i];
            //printf("40M[%d](%d, %d) --> ", i, pIQBuf40M[i].re, pIQBuf40M[i].im); 
            //printf("44M[%d](%d, %d) \n", outLength-1, pIQBuf44M[outLength-1].re, pIQBuf44M[outLength-1].im); 
        }
        //printf("40M[%d](%d, %d) * %02f + ", i, pIQBuf40M[i].re, pIQBuf40M[i].im, (float)UpLinearTableR[index10] /256);
        
    }
    return outLength;
}

