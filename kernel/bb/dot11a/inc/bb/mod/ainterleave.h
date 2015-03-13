#ifndef BB_MOD_AINTERLEAVE_H
#define BB_MOD_AINTERLEAVE_H

#include "lut.h" 

__forceinline
void InterleaveBPSK(const char * pbInput, char * pbOutput)
{
    const unsigned short * pusInterleaveMask
        = INTERLEAVE_6M((unsigned char)(pbInput[0]));
    unsigned int j;

    *(unsigned short *)(pbOutput) = pusInterleaveMask[0];
    *(unsigned short *)(pbOutput + 2) = pusInterleaveMask[1];
    *(unsigned short *)(pbOutput + 4) = pusInterleaveMask[2];

    for (j = 1; j < 6; j++)
    {
        pusInterleaveMask 
            = INTERLEAVE_6M((j << 8) + (unsigned char)(pbInput[j]));

        *(unsigned short *)(pbOutput) |= pusInterleaveMask[0];
        *(unsigned short *)(pbOutput + 2) |= pusInterleaveMask[1];
        *(unsigned short *)(pbOutput + 4) |= pusInterleaveMask[2];
    }
}

__forceinline
void InterleaveQPSK(const char * pbInput, char * pbOutput)
{
    unsigned int * puiOutput = (unsigned int *) pbOutput;
    const unsigned int * puiInterleaveMask
        = INTERLEAVE_12M((unsigned char)(pbInput[0]));
    unsigned int j;

    puiOutput[0] = puiInterleaveMask[0];
    puiOutput[1] = puiInterleaveMask[1];
    puiOutput[2] = puiInterleaveMask[2];

    for (j = 1; j < 12; j++)
    {
        puiInterleaveMask = INTERLEAVE_12M((j << 8) 
                + (unsigned char)(pbInput[j]));

        puiOutput[0] |= puiInterleaveMask[0];
        puiOutput[1] |= puiInterleaveMask[1];
        puiOutput[2] |= puiInterleaveMask[2];
    }
}

__forceinline
void Interleave16QAM(const char * pbInput, char * pbOutput)
{
    unsigned int * puiOutput = (unsigned int *) pbOutput;
    const unsigned int * puiInterleaveMask 
        = INTERLEAVE_24M((unsigned char)(pbInput[0]));
    unsigned int j;

    puiOutput[0] = puiInterleaveMask[0];
    puiOutput[1] = puiInterleaveMask[1];
    puiOutput[2] = puiInterleaveMask[2];
    puiOutput[3] = puiInterleaveMask[3];
    puiOutput[4] = puiInterleaveMask[4];
    puiOutput[5] = puiInterleaveMask[5];

    for (j = 1; j < 24; j++)
    {
        puiInterleaveMask = INTERLEAVE_24M((j << 8) 
                + (unsigned char)(pbInput[j]));

        puiOutput[0] |= puiInterleaveMask[0];
        puiOutput[1] |= puiInterleaveMask[1];
        puiOutput[2] |= puiInterleaveMask[2];
        puiOutput[3] |= puiInterleaveMask[3];
        puiOutput[4] |= puiInterleaveMask[4];
        puiOutput[5] |= puiInterleaveMask[5];
    }
}

__forceinline
void Interleave64QAM(const char * pbInput, char * pbOutput)
{
    unsigned int * puiOutput = (unsigned int *) pbOutput;
    const unsigned int * puiInterleaveMask 
        = INTERLEAVE_48M((unsigned char)(pbInput[0]));
    unsigned int j;

    puiOutput[0] = puiInterleaveMask[0];
    puiOutput[1] = puiInterleaveMask[1];
    puiOutput[2] = puiInterleaveMask[2];
    puiOutput[3] = puiInterleaveMask[3];
    puiOutput[4] = puiInterleaveMask[4];
    puiOutput[5] = puiInterleaveMask[5];
    puiOutput[6] = puiInterleaveMask[6];
    puiOutput[7] = puiInterleaveMask[7];
    puiOutput[8] = puiInterleaveMask[8];

    for (j = 1; j < 36; j++)
    {
        puiInterleaveMask = INTERLEAVE_48M((j << 8) 
                + (unsigned char)(pbInput[j]));

        puiOutput[0] |= puiInterleaveMask[0];
        puiOutput[1] |= puiInterleaveMask[1];
        puiOutput[2] |= puiInterleaveMask[2];
        puiOutput[3] |= puiInterleaveMask[3];
        puiOutput[4] |= puiInterleaveMask[4];
        puiOutput[5] |= puiInterleaveMask[5];
        puiOutput[6] |= puiInterleaveMask[6];
        puiOutput[7] |= puiInterleaveMask[7];
        puiOutput[8] |= puiInterleaveMask[8];
    }
}

#endif//BB_MOD_AINTERLEAVE_H
