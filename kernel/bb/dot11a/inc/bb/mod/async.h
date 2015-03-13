#pragma once

#include <stdio.h>
#include "func.h"
#include "vector128.h"

// Standard short preamble sync factor
extern __declspec(align(16)) const short g11a_stdSPSF[16 * 64];

// Get correlation between two signal blocks
// pa refers to 4 vcs and pb refers to 8 vcs
DSP_INLINE1 vi GetCrossCorrelation(const vcs* pa, const vcs* pb)
{
    // corr re
    vi re[4], im[4];
    re[0] = shift_right (pairwise_muladd((vs&)pa[0], (vs&)pb[0]), 5);
    re[1] = shift_right (pairwise_muladd((vs&)pa[1], (vs&)pb[1]), 5);
    re[2] = shift_right (pairwise_muladd((vs&)pa[2], (vs&)pb[2]), 5);
    re[3] = shift_right (pairwise_muladd((vs&)pa[3], (vs&)pb[3]), 5);

    vi s1 = add(re[0], re[1]);
    vi s2 = add(re[2], re[3]);
    vi sum_re = add(s1, s2 );
    sum_re = hadd(sum_re);

    // corr im
    im[0] = shift_right ( pairwise_muladd((vs&)pa[0], (vs&)pb[4]), 5 );
    im[1] = shift_right ( pairwise_muladd((vs&)pa[1], (vs&)pb[5]), 5 );
    im[2] = shift_right ( pairwise_muladd((vs&)pa[2], (vs&)pb[6]), 5 );
    im[3] = shift_right ( pairwise_muladd((vs&)pa[3], (vs&)pb[7]), 5 );

    s1 = add(im[0], im[1]);
    s2 = add(im[2], im[3]);
    vi sum_im = add(s1, s2);
    
    sum_im = hadd(sum_im);

    return add(abs(sum_re), abs(sum_im));
}

// Establish Synchronization
DSP_INLINE1 int EstablishSync(vi *pcorrRe, const vcs *pc, int *puiMax)
{
    vcs* pstdSPSF = (vcs*)g11a_stdSPSF;
    for (int i = 0; i < 16; i++)
    {
        pcorrRe[i] = GetCrossCorrelation(pc, pstdSPSF);
        pstdSPSF += 8;
    }

    unsigned int uiMax = pcorrRe[0][0];
    unsigned int uiSum = uiMax;
    unsigned int iMaxInd = 0;

    for (int i = 1; i < 16; i++)
    {
        unsigned int uiThis = pcorrRe[i][0];
        if (uiThis > uiMax)
        {
            uiMax = uiThis;
            iMaxInd = i;
        }
        uiSum += uiThis; 
    }

    if ((uiSum >> 3) > uiMax) 
    {
        // Note: if all noise, uiSum is 16 times of uiMax
        return -1;  // error, no peak
    }
        
    *puiMax = uiMax >> 1;
    return iMaxInd;
}

const int gAGCLut[17] = 
{
    -7, -6, -5, -4, -3,
    -2, -1,  0,  1,
     2,  3,  4,  2,
     2,  2,  2,  2,
};

DSP_INLINE1 void SetDigitalAGC (int corrRe, int& digitalAGC) 
{
    // Hint:
    //  pcorrRe is an estimation of ave I/Q modular (?)
    //
    int s = corrRe >> 16;
    digitalAGC = gAGCLut[FindLeftMostSetPosition(s)]; // Kun: add average later

    //printf ( "DAGC: corr %d AGC %d\n", corrRe, digitalAGC );
}

// Check Synchronization
DSP_INLINE1 bool CheckSync(vi* pcorrRe, vcs * pc, int pos, unsigned int uiMax, int& digitalAGC)
{
    vcs* pstdSPSF = (vcs*)g11a_stdSPSF + (pos << 3);

    pcorrRe[0] = GetCrossCorrelation(pc, pstdSPSF);

    int corrRe = pcorrRe[0][0];
    if ( corrRe < 0 ) corrRe = - corrRe;

//    if ((unsigned int)pcorrRe[0][0] > uiMax)
    if ( (unsigned int) corrRe > uiMax ) 
    {
        // Kun: disable DigitalAGC
		digitalAGC = 0;
        // SetDigitalAGC ( corrRe, digitalAGC );
        return 1;
    }

    return 0;
}
