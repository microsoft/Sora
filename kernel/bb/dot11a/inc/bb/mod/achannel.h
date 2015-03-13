#ifndef BB_MOD_ACHANNEL_H
#define BB_MOD_ACHANNEL_H

#include <stdio.h>
#include "bb/bba.h"

//
// Compute factor Re and Im that will normalize it pilot to 100 * 2^13
//
#define NORM_ONE (100*8*2)
#define NORM_SHIFT (6-1)
#define CH_SHIFT   3
#define COMP_SHIFT (8+3) // COMP_SHIFT = LOG2(NORM_ONE / 100) + NORM_SHIFT - 1



static const char rgbLongPrePositive[64] = {
    0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 
    1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1
};

__forceinline
void 
EstimateChannel (
    IN const COMPLEX16 * pc, 
    OUT COMPLEX32 ChannelFactor[CHANNEL_FACTOR_SIZE])
{
    int i = 0;
    int sq;
    int re, im;


    for (i = 1; i <= 26; i++)
    {
        re = pc[i].re;
        im = pc[i].im;
        sq = re * re + im * im;

        sq >>= NORM_SHIFT;

        if (sq != 0)
        {
            if (rgbLongPrePositive[i])
                im = -im;
            else
                re = -re;

            re = (re * NORM_ONE) / sq;
            im = (im * NORM_ONE) / sq;

            ChannelFactor[i].re = re << CH_SHIFT;
            ChannelFactor[i].im = im << CH_SHIFT;
        }
    }

    for (i = 64 - 26; i < 64; i++)
    {
        re = pc[i].re;
        im = pc[i].im;
        sq = re * re + im * im;
        sq >>= NORM_SHIFT;

        if (sq != 0)
        {
            if (rgbLongPrePositive[i])
                im = -im;
            else
                re = -re;

            re = (re * NORM_ONE) / sq;
            im = (im * NORM_ONE) / sq;

            ChannelFactor[i].re = re << CH_SHIFT;
            ChannelFactor[i].im = im << CH_SHIFT;
        }
    }
}


__forceinline
void 
EstimateChannel2 (
    IN const COMPLEX16 * pc, 
    OUT COMPLEX32 ChannelFactor[CHANNEL_FACTOR_SIZE])
{
    int i = 0;
    int sq;
    int re, im;


    for (i = 1; i <= 26; i++)
    {
        re = (pc[i].re + pc[i+64].re) >> 1;
        im = (pc[i].im + pc[i+64].im) >> 1;
        sq = re * re + im * im;

        sq >>= NORM_SHIFT;

        if (sq != 0)
        {
            if (rgbLongPrePositive[i])
                im = -im;
            else
                re = -re;

            re = (re * NORM_ONE) / sq;
            im = (im * NORM_ONE) / sq;

            ChannelFactor[i].re += (re << CH_SHIFT);
            ChannelFactor[i].im += (im << CH_SHIFT);
        }
    }

    for (i = 64 - 26; i < 64; i++)
    {
        re = (pc[i].re + pc[i+64].re) >> 1;
        im = (pc[i].im + pc[i+64].im) >> 1;
        sq = re * re + im * im;
        sq >>= NORM_SHIFT;

        if (sq != 0)
        {
            if (rgbLongPrePositive[i])
                im = -im;
            else
                re = -re;

            re = (re * NORM_ONE) / sq;
            im = (im * NORM_ONE) / sq;

            ChannelFactor[i].re += (re << CH_SHIFT);
            ChannelFactor[i].im += (im << CH_SHIFT); 
        }
    }
}


//
// Hint:
//    Channel compensation normalizes the value to 100.
//

__forceinline
void 
ChannelComp(
    IN const COMPLEX32 ChannelFactor[CHANNEL_FACTOR_SIZE], 
    OUT COMPLEX16 * pc)
{
    int i;
    int re, im;
    
    for (i = 1; i <= 26; i++)
    {
        re = pc[i].re * ChannelFactor[i].re
             - pc[i].im * ChannelFactor[i].im;
        im = pc[i].re * ChannelFactor[i].im
             + pc[i].im * ChannelFactor[i].re;
        
        pc[i].re = (short)(re >> COMP_SHIFT);
        pc[i].im = (short)(im >> COMP_SHIFT);

    }
    

    for (i = 64 - 26; i < 64; i++)
    {
        re = pc[i].re * ChannelFactor[i].re
             - pc[i].im * ChannelFactor[i].im;
        im = pc[i].re * ChannelFactor[i].im
             + pc[i].im * ChannelFactor[i].re;
        
        pc[i].re = (short)(re >> COMP_SHIFT);
        pc[i].im = (short)(im >> COMP_SHIFT);

    }

}

#undef NORM_SHIFT

#endif//BB_MOD_ACHANNEL_H
