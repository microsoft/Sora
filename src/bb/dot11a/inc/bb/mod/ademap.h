#pragma once

#include <stdio.h>
#include "vector128.h"

extern const unsigned char g11a_rgbDemapBPSK[256];
extern const unsigned char g11a_rgbDemap16QAM2[256];
extern const unsigned char g11a_rgbDemap64QAM2[256];
extern const unsigned char g11a_rgbDemap64QAM3[256];

DSP_INLINE1 void DemapLimit_11a(vcs * pc)
{
    static const vcs::data_type rgbDemapMin = {
        { -128, -128 }, { -128, -128 }, { -128, -128 }, { -128, -128 },
    };
    static const vcs::data_type rgbDemapMax = {
        { 127, 127 }, { 127, 127 }, { 127, 127 }, { 127, 127 },
    };

    for (int i = 0; i < 16; i++)
    {
        pc[i] = smin(smax(pc[i], rgbDemapMin), rgbDemapMax);
    }
}

DSP_INLINE
void DemapBPSK_11a(COMPLEX16 * pc, unsigned char * pbOutput)
{
    int i;
    int j = 0;
    
    for (i = 64 - 26; i < 64; i++)
    {
        if (i == 64 - 21 || i == 64 - 7)
            continue;
        pbOutput[0] = g11a_rgbDemapBPSK[(unsigned char)(pc[i].re)];
        pbOutput++;
    }

    for (i = 1; i <= 26; i++)
    {
        if (i == 7 || i == 21)
            continue;
        
        pbOutput[0] = g11a_rgbDemapBPSK[(unsigned char)(pc[i].re)];
        pbOutput++;
    }
}

DSP_INLINE
void DemapQPSK_11a(COMPLEX16 * pc, unsigned char * pbOutput)
{
    int i;
    int j = 0;
    
    for (i = 64 - 26; i < 64; i++)
    {
        if (i == 64 - 21 || i == 64 - 7)
            continue;
        pbOutput[0] = g11a_rgbDemapBPSK[(unsigned char)(pc[i].re)];
        pbOutput[1] = g11a_rgbDemapBPSK[(unsigned char)(pc[i].im)];
        pbOutput += 2;
    }

    for (i = 1; i <= 26; i++)
    {
        if (i == 7 || i == 21)
            continue;
        pbOutput[0] = g11a_rgbDemapBPSK[(unsigned char)(pc[i].re)];
        pbOutput[1] = g11a_rgbDemapBPSK[(unsigned char)(pc[i].im)];
        pbOutput += 2;
    }
}

DSP_INLINE
void Demap16QAM_11a(COMPLEX16 * pc, unsigned char * pbOutput)
{
    int i;
    unsigned char re, im;
    
    for (i = 64 - 26; i < 64; i++)
    {
        if (i == 64 - 21 || i == 64 - 7)
            continue;
        re = (unsigned char)(pc[i].re);
        im = (unsigned char)(pc[i].im);
        pbOutput[0] = g11a_rgbDemapBPSK[re];
        pbOutput[1] = g11a_rgbDemap16QAM2[re];
        pbOutput[2] = g11a_rgbDemapBPSK[im];
        pbOutput[3] = g11a_rgbDemap16QAM2[im];
        pbOutput += 4;
    }

    for (i = 1; i <= 26; i++)
    {
        if (i == 7 || i == 21)
            continue;
        re = (unsigned char)(pc[i].re);
        im = (unsigned char)(pc[i].im);
        pbOutput[0] = g11a_rgbDemapBPSK[re];
        pbOutput[1] = g11a_rgbDemap16QAM2[re];
        pbOutput[2] = g11a_rgbDemapBPSK[im];
        pbOutput[3] = g11a_rgbDemap16QAM2[im];
        pbOutput += 4;
    }
}

DSP_INLINE
void Demap64QAM_11a(COMPLEX16 * pc, unsigned char * pbOutput)
{
    int i;
    unsigned char re, im;
    
    for (i = 64 - 26; i < 64; i++)
    {
        if (i == 64 - 21 || i == 64 - 7)
            continue;
        re = (unsigned char)(pc[i].re);
        im = (unsigned char)(pc[i].im);
        pbOutput[0] = g11a_rgbDemapBPSK[re];
        pbOutput[1] = g11a_rgbDemap64QAM2[re];
        pbOutput[2] = g11a_rgbDemap64QAM3[re];
        pbOutput[3] = g11a_rgbDemapBPSK[im];
        pbOutput[4] = g11a_rgbDemap64QAM2[im];
        pbOutput[5] = g11a_rgbDemap64QAM3[im];
        pbOutput += 6;
    }

    for (i = 1; i <= 26; i++)
    {
        if (i == 7 || i == 21)
            continue;
        re = (unsigned char)(pc[i].re);
        im = (unsigned char)(pc[i].im);
        pbOutput[0] = g11a_rgbDemapBPSK[re];
        pbOutput[1] = g11a_rgbDemap64QAM2[re];
        pbOutput[2] = g11a_rgbDemap64QAM3[re];
        pbOutput[3] = g11a_rgbDemapBPSK[im];
        pbOutput[4] = g11a_rgbDemap64QAM2[im];
        pbOutput[5] = g11a_rgbDemap64QAM3[im];
        pbOutput += 6;
    }
}

DSP_INLINE
void Demap_11a(COMPLEX16 * pc, unsigned char * pbOutput, char bRate)
{

// dump constellation
#if BB_DBG
   int i;
   for (i = 64 - 26; i < 64; i++)
   {
        if ( i == 64 -7 || i == 64 - 21 ) continue;
        fprintf ( stderr, " %d %d\n", pc[i].re, pc[i].im, stderr);
   }

   for (i = 1; i <= 26; i++)
   {
        if ( i == 7 || i == 21 ) continue;
        fprintf (stderr, " %d %d\n", pc[i].re, pc[i].im );
   }
#endif


    DemapLimit_11a((vcs*)pc);
    
    switch (bRate & 0x3)
    {
        case 0x3:
            DemapBPSK_11a(pc, pbOutput);
            break;
        case 0x2:
            DemapQPSK_11a(pc, pbOutput);
            break;
        case 0x1:
            Demap16QAM_11a(pc, pbOutput);
#ifdef _DBG_PLOT_
			PlotDots ("BB:Constel", &pc[1], 63);
#endif 
            break;
        case 0x0:
            Demap64QAM_11a(pc, pbOutput);
#ifdef _DBG_PLOT_
			PlotDots ("BB:Constel", &pc[1], 63);
#endif 
			
            break;
    }
}
