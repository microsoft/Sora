#include "common.h"

//struct COMPLEX16 {
//    short re, im;
//};

static const unsigned char g11a_rgbDemapBPSK[256] = {
    // positive
    4, 5, 6, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,

    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,

    // negative
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 2, 3,
};

static const unsigned char g11a_rgbDemap16QAM2[256] = {
    // positive
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  6, 5, 4, 3,

    2, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    // negative
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 2,

    3, 4, 5, 6,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
};

static const unsigned char g11a_rgbDemap64QAM2[256] = {
    // positive
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 6,  5, 4, 3, 2,

    1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    // negative
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,

    2, 3, 4, 5,  6, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
};

static const unsigned char g11a_rgbDemap64QAM3[256] = {
    // positive
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 2, 3, 4,
    5, 6, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,

    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 6, 5, 4,  3, 2, 1, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

    // negative
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 1, 2, 3,  4, 5, 6, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,

    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
    7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 6, 5,
    4, 3, 2, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
};

static void 
demapBPSK_11a(COMPLEX16 * pc, unsigned char * pbOutput)
{
    int i;
    
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

static void 
demapQPSK_11a(COMPLEX16 * pc, unsigned char * pbOutput)
{
    int i;
    
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

static void 
demap16QAM_11a(COMPLEX16 * pc, unsigned char * pbOutput)
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

static void 
demap64QAM_11a(COMPLEX16 * pc, unsigned char * pbOutput)
{
    int i;
    unsigned char re, im;
	double m;
    
    for (i = 64 - 26; i < 64; i++)
    {
        m = getSoftBitModifier (i);
		
        if (i == 64 - 21 || i == 64 - 7)
            continue;
        re = (unsigned char)(pc[i].re);
        im = (unsigned char)(pc[i].im);
		
        pbOutput[0] = (unsigned char) (m * g11a_rgbDemapBPSK[re] * 2 + 1) /2 ;
        pbOutput[1] = (unsigned char) (m * g11a_rgbDemap64QAM2[re]*2+1 )/2;
        pbOutput[2] = (unsigned char) (m * g11a_rgbDemap64QAM3[re]*2+1 )/2;
        pbOutput[3] = (unsigned char) (m * g11a_rgbDemapBPSK[im]  *2+1 )/2;
        pbOutput[4] = (unsigned char) (m * g11a_rgbDemap64QAM2[im] *2+1 )/2;
        pbOutput[5] = (unsigned char) (m * g11a_rgbDemap64QAM3[im] *2+1 )/2;
        /*
        pbOutput[0] = g11a_rgbDemapBPSK[re] ;
        pbOutput[1] = g11a_rgbDemap64QAM2[re];
        pbOutput[2] = g11a_rgbDemap64QAM3[re];
        pbOutput[3] = g11a_rgbDemapBPSK[im];
        pbOutput[4] = g11a_rgbDemap64QAM2[im] ;
        pbOutput[5] = g11a_rgbDemap64QAM3[im] ;
		*/
        pbOutput += 6;
    }

    for (i = 1; i <= 26; i++)
    {
        m = getSoftBitModifier (i);
        if (i == 7 || i == 21)
            continue;
        re = (unsigned char)(pc[i].re);
        im = (unsigned char)(pc[i].im);

        pbOutput[0] = (unsigned char) (m * g11a_rgbDemapBPSK[re] *2+1 )/2;
        pbOutput[1] = (unsigned char) (m * g11a_rgbDemap64QAM2[re] *2+1 )/2;
        pbOutput[2] = (unsigned char) (m * g11a_rgbDemap64QAM3[re] *2+1 )/2;
        pbOutput[3] = (unsigned char) (m * g11a_rgbDemapBPSK[im] *2+1 )/2;
        pbOutput[4] = (unsigned char) (m * g11a_rgbDemap64QAM2[im] *2+1 )/2;
        pbOutput[5] = (unsigned char) (m * g11a_rgbDemap64QAM3[im] *2+1 )/2;

/*
        pbOutput[0] = g11a_rgbDemapBPSK[re] ;
        pbOutput[1] = g11a_rgbDemap64QAM2[re] ;
        pbOutput[2] = g11a_rgbDemap64QAM3[re] ;
        pbOutput[3] = g11a_rgbDemapBPSK[im] ;
        pbOutput[4] = g11a_rgbDemap64QAM2[im] ;
        pbOutput[5] = g11a_rgbDemap64QAM3[im] ;
*/
        pbOutput += 6;
    }
}

void
demap(const Complex * pc, unsigned char * poutput, int rate)
{
    COMPLEX16 cache[64];
    for (int i = 0; i < 64; i++)
    {
        cache[i].re = (short)(pc[i].re * 100);
        cache[i].im = (short)(pc[i].im * 100);
        if (cache[i].re > 127)
            cache[i].re = 127;
        if (cache[i].re < -128)
            cache[i].re = -128;
        if (cache[i].im > 127)
            cache[i].im = 127;
        if (cache[i].re < -128)
            cache[i].re = -128;
    }

    memset(poutput, -1, 289);

    switch (rate)
    {
        case 6: case 9:
            demapBPSK_11a(cache, poutput);
            break;
        case 12: case 18:
            demapQPSK_11a(cache, poutput);
            break;
        case 24: case 36:
            demap16QAM_11a(cache, poutput);
            break;
        case 48: case 54:
            demap64QAM_11a(cache, poutput);
            break;
        default:
            dprintf("ERROR: unknown rate in demap()");
            break;
    }
}


