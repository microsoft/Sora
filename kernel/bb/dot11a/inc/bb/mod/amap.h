#ifndef BB_MOD_AMAP_H
#define BB_MOD_AMAP_H

#include "lut.h"
#include "vector128.h"

DSP_INLINE
void MapBPSK_11a(char * pbInput, COMPLEX16 * pcOutput)
{
    unsigned int i;
    const COMPLEX16 * pvLUT;

    for (i = 0; i < 6; i++)
    {
        pvLUT = MAPA_BPSK((unsigned char)(pbInput[i]));
        memcpy(pcOutput, pvLUT, sizeof(COMPLEX16) * 8);
        pcOutput += 8;
    }
}

__forceinline
void MapQPSK_11a(char * pbInput, COMPLEX16 * pcOutput)
{
    unsigned int i;
    const COMPLEX16 * pcLUT;

    for (i = 0; i < 12; i++)
    {
        pcLUT = MAPA_QPSK((unsigned char)(pbInput[i]));
        pcOutput[0] = pcLUT[0];
        pcOutput[1] = pcLUT[1];
        pcOutput[2] = pcLUT[2];
        pcOutput[3] = pcLUT[3];
        pcOutput += 4;
    }
}

__forceinline
void Map16QAM_11a(char * pbInput, COMPLEX16 * pcOutput)
{
    unsigned int i;
    const COMPLEX16 * pcLUT;

    for (i = 0; i < 24; i++)
    {
        pcLUT = MAPA_16QAM((unsigned char)(pbInput[i]));
        pcOutput[0] = pcLUT[0];
        pcOutput[1] = pcLUT[1];
        pcOutput += 2;
    }
}

__forceinline
void Map64QAM_11a(char * pbInput, COMPLEX16 * pcOutput)
{
    unsigned int i;
    unsigned int uiInput;
    const COMPLEX16 * pcLUT;

    for (i = 0; i < 12; i++)
    {
        uiInput = (*((unsigned int *)(pbInput)));

        pcLUT = MAPA_64QAM(uiInput & 0xFFF);
        pcOutput[0] = pcLUT[0];
        pcOutput[1] = pcLUT[1];

        pcLUT = MAPA_64QAM((uiInput >> 12) & 0xFFF);
        pcOutput[2] = pcLUT[0];
        pcOutput[3] = pcLUT[1];

        pcOutput += 4;
        pbInput += 3;
    }
}

#endif//BB_MOD_AMAP_H
