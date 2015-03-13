/*++
Copyright (c) Microsoft Corporation

Module Name: soradsp.h

Abstract: Sora digital signal processing functions.

--*/
#pragma once

#include <stdlib.h>
#include "const.h"
#include "soratypes.h"
#include "sora.h"

typedef struct __COMPLEXM128
{
    SAMPLE  Samples[M128_COMPLEX16_NUM];
}COMPLEXM128, *PCOMPLEXM128;

#ifdef INLINE_ASM

#define SORA_ABS_M128_COMPLEX16(xmm, tempxmm)               \
    {__asm pxor   tempxmm, tempxmm }    /* tempxmm <= 0 */  \
    {__asm psubw  tempxmm, xmm }        /* tempxmm <= tempxmm - xmm, ie. tempxmm <= -xmm, word-wise */ \
    {__asm pmaxsw xmm, tempxmm }        /* xmm <= Max (xmm, tempxmm), ie. xmm <= |xmm|, word-wise   */ \

// Duplicate one complex number 4 times
// Note: xmm0 is undefined after the call. 
SORA_EXTERN_C
FINL void  SoraDupComplex16ToComplexM128 (IN const COMPLEX16 *pSrc, OUT COMPLEXM128 *pDst)
{
    // *pDst <- [*pSrc ... *pSrc] (ie. 4 times duplication)
    *(s_int32*)pDst = *(s_int32*)pSrc;
    __asm
    {
        mov esi, pDst;
        pshufd xmm0, [esi], 0x00;
        movdqa [esi], xmm0;
    } 
}

// Stores a XMM Signal Block (in xmm0 ~ xmm6) to a memory location.
// Parameters:
//   pSampleBuf: the memory location for the Signal Block.
// Note: 
//   Before called, xmm0 ~ xmm6 contains a block of signal samples. 
SORA_EXTERN_C
FINL void SoraStoreXMMSignalBlock(OUT SORA_SAMPLE_BLOCK *pSampleBuf)
{
    // Load xmm regs to memory
    // reg: xmm0 ~ xmm6
    // memory: pSampleBuf[0 ~ 6F]
    __asm
    {
        mov     esi, pSampleBuf;
        movdqa  [esi], xmm0;
        movdqa  [esi + 0x10], xmm1;
        movdqa  [esi + 0x20], xmm2;
        movdqa  [esi + 0x30], xmm3;
        movdqa  [esi + 0x40], xmm4;
        movdqa  [esi + 0x50], xmm5;
        movdqa  [esi + 0x60], xmm6;
    }
}

// Loads a XMM Signal Block (xmm0 ~ xmm6) from sample block buffer.
// Parameters:
//   pSampleBuf: sample block buffer.
// Note: 
//   After called, xmm0 ~ xmm6 contains a block of signal samples.
//   Access unshared memory (software access only)
SORA_EXTERN_C
FINL void SoraLoadXMMSignalBlock(IN SORA_SAMPLE_BLOCK *pSampleBuf)
{
    __asm 
    {
        mov    esi , pSampleBuf;
        movdqa xmm0, [esi];
        pshufd xmm1, [esi+0x10], MSP;
        movdqa xmm2, [esi+0x20];
        pshufd xmm3, [esi+0x30], MSP;
        movdqa xmm4, [esi+0x40];
        pshufd xmm5, [esi+0x50], MSP;
        movdqa xmm6, [esi+0x60];
    }
}

// Substracts a XMM Signal Block (xmm0~xmm6) with a constant COMPLEXM128. 
SORA_EXTERN_C
FINL void SoraRemoveDcXMMSignalBlock(PCOMPLEXM128 pOperand)
{
    // [xmm0~xmm6] -= *ppOperand (word-wise)
    __asm
    {
        mov esi, pOperand;
        pshufd  xmm7, [esi], MSP;
        psubw   xmm0, xmm7;
        psubw   xmm1, xmm7;
        psubw   xmm2, xmm7;
        psubw   xmm3, xmm7;
        psubw   xmm4, xmm7;
        psubw   xmm5, xmm7;
        psubw   xmm6, xmm7;
    }
}

// Check all COMPLEX16 items in XMM0~XMM6 registries, and find the smallest square to cover them all
// Parameters:
//   pLeast: The 1/2 of the edge width of the sqaure 
SORA_EXTERN_C
FINL void SoraEstimateEnergyScope(OUT s_int16 *pLeast)
{
    __asm
    {
        pshufd      xmm7, xmm0, MSP;
        pmaxsw      xmm7, xmm1;
        pmaxsw      xmm7, xmm2;
        pmaxsw      xmm7, xmm3;
        pmaxsw      xmm7, xmm4;
        pmaxsw      xmm7, xmm5;
        pmaxsw      xmm7, xmm6;

        pminsw      xmm0, xmm1;
        pminsw      xmm0, xmm2;
        pminsw      xmm0, xmm3;
        pminsw      xmm0, xmm4;
        pminsw      xmm0, xmm5;
        pminsw      xmm0, xmm6;

        pxor        xmm1, xmm1;
        psubw       xmm1, xmm0;
        pmaxsw      xmm1, xmm7;         // xmm1 <= Max (|xmm0| ~ |xmm7|), word-wise

        pshufd      xmm0, xmm1, 0x4E;
        pmaxsw      xmm0, xmm1;
        pshufd      xmm1, xmm0, 0xB1;
        pmaxsw      xmm0, xmm1;         // xmm0 double word <= Max (xmm1 double words)

        punpcklwd   xmm0, xmm0;
        pshufd      xmm1, xmm0, 0xB1;
        pmaxsw      xmm0, xmm1;         // xmm0 word <= Max (xmm0 words)

        pextrw      eax, xmm0, 0;       // eax <= Max (xmm0 word)

        mov         esi, pLeast;
        mov         WORD PTR [esi], ax; // [pLeast] <= eax
        // this is for energy drop judging
    }
}

// Same as SoraEstimateEnergyScope with more readable implementation,
// used for validation only, not used in real hardware
SORA_EXTERN_C
FINL int SoraEstimateEnergyScope_Shadow(SORA_SAMPLE_BLOCK *block)
{
    int max = 0;
    int i, j;
    for(j = 0; j < SORA_RX_SIGNAL_UNIT_COMPLEX16_NUM; j++)
    {
        for(i = 0; i < SORA_RX_SIGNAL_UNIT_NUM_PER_DESC; i++)
        {
            int x = abs(block->SampleUnit[i].Samples[j].re);
            if (x > max) max = x;
            x = abs(block->SampleUnit[i].Samples[j].im);
            if (x > max) max = x;
        }
    }
    return max;
}

// Calculates average of [xmm0, xmm0, xmm1 ~ xmm6] double-word-wise, and store it in XMM0
SORA_EXTERN_C
FINL void __CalDCOffsetToXmm0()
{
    __asm
    {
        psraw xmm0, 2;
        psraw xmm1, 3;
        psraw xmm2, 3;
        psraw xmm3, 3;
        psraw xmm4, 3;
        psraw xmm5, 3;
        psraw xmm6, 3;

        paddw xmm0, xmm1;
        paddw xmm0, xmm2;
        paddw xmm0, xmm3;
        paddw xmm0, xmm4;
        paddw xmm0, xmm5;
        paddw xmm0, xmm6;

        psraw xmm0, 2;

        pshufd xmm4, xmm0, 0x4E;
        paddw xmm0, xmm4;
        pshufd xmm4, xmm0, 0xB1;
        paddw xmm0, xmm4;
    }
}

// Calculates Direct-Current(DC) offset from a XMM sample block xmm0 ~ xmm6
// Parameters:
//   pDcOffsetM128: pointer to 128-bit output of DC offset.
// Note:
//   After it, xmm0~xmm6 sample block is destroyed.
SORA_EXTERN_C
FINL void SoraCalcDCfromXMMSignalBlock (OUT COMPLEX16 *pDcOffsetM128)
{
    __CalDCOffsetToXmm0();
    __asm
    {
        mov esi, pDcOffsetM128;
        movdqa [esi], xmm0;
    }
}

// Updates Direct-Current(DC) offset from new a XMM sample block xmm0 ~ xmm6
// Parameters:
//   pDcOffsetM128: pointer to 128-bit input and output of DC offset.
// Note:
//   After it, xmm0~xmm6 sample block is destroyed.
SORA_EXTERN_C
FINL void SoraUpdateDCfromXMMSignalBlock (IN OUT COMPLEX16 *pDcOffsetM128)
{
    // *pDcOffsetM128 <- *pDcOffsetM128 * 15/16 + DC(xmm0~xmm6) / 16
    __CalDCOffsetToXmm0();
    __asm
    {
        mov esi, pDcOffsetM128;
        pshufd xmm2, [esi], MSP;
        pshufd xmm7, xmm2, MSP;
        psraw xmm7, 4;
        psubw xmm2, xmm7;
        psraw xmm0, 4;
        paddw xmm0, xmm2;
        movdqa [esi], xmm0;
    }
}

// Decreases energy of sample block saved in xmm0~xmm6 linearly.
// Parameters:
//   shift: decrease level. Result sample will be equal to original sample >> shift.
// Note:
//   Sample block must be saved in xmm0~xmm6 before it is called. Bad parameter 
//   will cause unknown error.
SORA_EXTERN_C
FINL void SoraRightShiftXMMSignalBlock(unsigned int shift)
{
    ASSERT(shift < 16);     // ref: sdr_phy_rx.c __GetDagcShiftRightBits()
    __asm
    {
        mov esi, shift;
        xorps xmm7, xmm7;
        pinsrw xmm7, esi, 0;
        psraw xmm0, xmm7;
        psraw xmm1, xmm7;
        psraw xmm2, xmm7;
        psraw xmm3, xmm7;
        psraw xmm4, xmm7;
        psraw xmm5, xmm7;
        psraw xmm6, xmm7;
    }
}

// Increases energy of sample block saved in xmm0~xmm6 linearly.
// Parameters:
//   shift: increase level. Result sample will be equal to original sample << shift.
// Note:
//   Sample block must be saved in xmm0~xmm6 before it is called. Bad parameter 
//   will cause unknown error.
SORA_EXTERN_C
FINL void SoraLeftShiftXMMSignalBlock(unsigned int shift)
{
    ASSERT(shift < 16);     // ref: sdr_phy_rx.c __GetDagcShiftRightBits()
    __asm
    {
        mov esi, shift;
        xorps xmm7, xmm7;
        pinsrw xmm7, esi, 0;
        psllw xmm0, xmm7;
        psllw xmm1, xmm7;
        psllw xmm2, xmm7;
        psllw xmm3, xmm7;
        psllw xmm4, xmm7;
        psllw xmm5, xmm7;
        psllw xmm6, xmm7;
    }
}

// Calculates 1/16 of sum of 1-norm of complex numbers in one SignalBlock
SORA_EXTERN_C
FINL void SoraPackNorm1XMMSignalBlock(unsigned short *pDownSampleNorm)
{
    __asm
    {
        pxor    xmm7, xmm7;
        psubw   xmm7, xmm0;
        pmaxsw  xmm0, xmm7;         // xmm0 <- |xmm0|, word-wise

        pxor    xmm7, xmm7;
        psubw   xmm7, xmm1;
        pmaxsw  xmm1, xmm7;         // xmm1 <- |xmm1|, word-wise

        pxor    xmm7, xmm7;
        psubw   xmm7, xmm2;
        pmaxsw  xmm2, xmm7;         // xmm2 <- |xmm2|, word-wise

        pxor    xmm7, xmm7;
        psubw   xmm7, xmm3;
        pmaxsw  xmm3, xmm7;         // xmm3 <- |xmm3|, word-wise

        pxor    xmm7, xmm7;
        psubw   xmm7, xmm4;
        pmaxsw  xmm4, xmm7;         // xmm4 <- |xmm4|, word-wise

        pxor    xmm7, xmm7;
        psubw   xmm7, xmm5;
        pmaxsw  xmm5, xmm7;         // xmm5 <- |xmm5|, word-wise

        pxor    xmm7, xmm7;
        psubw   xmm7, xmm6;
        pmaxsw  xmm6, xmm7;         // xmm6 <- |xmm6|, word-wise

        pavgw xmm6, xmm5;
        pavgw xmm4, xmm3;
        pavgw xmm2, xmm1;

        pavgw xmm4, xmm6;
        psrlw xmm4, 1;              // xmm4 <- (xmm3 +...+ xmm6) / 8

        psrlw xmm2, 2;              // xmm2 <- (xmm1 + xmm2) / 8
        psrlw xmm0, 3;              // xmm0 <-  xmm0 / 8

        paddw xmm0, xmm2;
        paddw xmm0, xmm4;           // xmm0 += xmm2 + xmm4

        pshufhw xmm1, xmm0, 0xB1;
        pshuflw xmm1, xmm1, 0xB1;   // xmm1 <- Swap I/Q of xmm0

        pavgw xmm0, xmm1;           // xmm0 <- Average I/Q of xmm0

        mov esi, pDownSampleNorm;
        movdqa [esi], xmm0;
    }
}

// ONLY used in assertion for SoraPackNorm1XMMSignalBlock()
SORA_EXTERN_C
FINL BOOLEAN SoraPackNorm1XMMSignalBlock_Check(const SORA_SAMPLE_BLOCK *block, const unsigned short *pDownSampleNorm)
{
    int abs1, abs2, s1, s2, s3, s4, j;
    for(j = 0; j < SORA_RX_SIGNAL_UNIT_COMPLEX16_NUM; j++)
    {
        // Real part
        abs1 = abs(block->SampleUnit[3].Samples[j].re);
        abs2 = abs(block->SampleUnit[4].Samples[j].re);
        s1 = (abs1 + abs2 + 1) / 2;
        abs1 = abs(block->SampleUnit[5].Samples[j].re);
        abs2 = abs(block->SampleUnit[6].Samples[j].re);
        s2 = (abs1 + abs2 + 1) / 2;
        s3 = (s1 + s2 + 1) / 4;

        abs1 = abs(block->SampleUnit[1].Samples[j].re);
        abs2 = abs(block->SampleUnit[2].Samples[j].re);
        s1 = (abs1 + abs2 + 1) / 2;
        s3 += s1 / 4;

        abs1 = abs(block->SampleUnit[0].Samples[j].re);
        s3 += abs1 / 8;

        // Image part
        abs1 = abs(block->SampleUnit[3].Samples[j].im);
        abs2 = abs(block->SampleUnit[4].Samples[j].im);
        s1 = (abs1 + abs2 + 1) / 2;
        abs1 = abs(block->SampleUnit[5].Samples[j].im);
        abs2 = abs(block->SampleUnit[6].Samples[j].im);
        s2 = (abs1 + abs2 + 1) / 2;
        s4 = (s1 + s2 + 1) / 4;

        abs1 = abs(block->SampleUnit[1].Samples[j].im);
        abs2 = abs(block->SampleUnit[2].Samples[j].im);
        s1 = (abs1 + abs2 + 1) / 2;
        s4 += s1 / 4;

        abs1 = abs(block->SampleUnit[0].Samples[j].im);
        s4 += abs1 / 8;

        // All
        if (pDownSampleNorm[j*2] != (s3 + s4 + 1) / 2)
            return FALSE;
    }
    return TRUE;
}

// Calculates 1/4 of sum of 1-norm of complex numbers in one SignalBlock
SORA_EXTERN_C
FINL void SoraGetNormFromXMMSignalBlock(ULONG DescSum[4])
{
    __asm
    {
        // Mirror complex numbers in xmm0 ~ xmm6 to the first quadrant
        SORA_ABS_M128_COMPLEX16(xmm0, xmm7);
        SORA_ABS_M128_COMPLEX16(xmm1, xmm7);
        SORA_ABS_M128_COMPLEX16(xmm2, xmm7);
        SORA_ABS_M128_COMPLEX16(xmm3, xmm7);
        SORA_ABS_M128_COMPLEX16(xmm4, xmm7);
        SORA_ABS_M128_COMPLEX16(xmm5, xmm7);
        SORA_ABS_M128_COMPLEX16(xmm6, xmm7);

        pavgw xmm0, xmm1;       // xmm0 <- (xmm0 + xmm1) / 2
        pavgw xmm2, xmm3;       // xmm2 <- (xmm2 + xmm3) / 2
        pavgw xmm4, xmm5;       // xmm4 <- (xmm4 + xmm5) / 2

        pxor xmm7, xmm7;        // xmm7 = 0
        psrlw xmm6, 1;          // xmm6 /= 2

        pshufd xmm1, xmm0, MSP; // xmm1 <- xmm0
        pshufd xmm3, xmm2, MSP; // xmm3 <- xmm2
        pshufd xmm5, xmm4, MSP; // xmm5 <- xmm4

        punpcklwd xmm0, xmm7;
        punpckhwd xmm1, xmm7;
        paddd xmm1, xmm0;

        punpcklwd xmm2, xmm7;
        punpckhwd xmm3, xmm7;
        paddd xmm1, xmm2;
        paddd xmm1, xmm3;

        punpcklwd xmm4, xmm7;
        punpckhwd xmm5, xmm7;
        paddd xmm1, xmm4;
        paddd xmm1, xmm5;

        pshufd xmm5, xmm6, MSP; // Duplicate xmm6 into xmm5
        punpcklwd xmm5, xmm7;
        punpckhwd xmm6, xmm7;
        paddd xmm1, xmm5;
        paddd xmm1, xmm6;       // xmm1 <- (I_odd, Q_odd, I_even, Q_even), double-word-wise

        pshufd xmm0, xmm1, 0x4E;
        paddd xmm1, xmm0;
        pshufd xmm0, xmm1, 0xB1;
        paddd xmm0, xmm1;       // xmm0 <- I_all + Q_all

        // Now xmm0 have 4 double word, each is a sum
        psrld xmm0, 1;          // xmm0 /= 2
        mov esi, DescSum;
        movdqa [esi], xmm0;     // DescSum <- xmm0
    }
}

// Note: shadow function for SoraGetNormFromXMMSignalBlock(), only used in
// pure software validation of that function
SORA_EXTERN_C
FINL ULONG SoraGetNormFromXMMSignalBlock_Shadow()
{
    A16 SORA_SAMPLE_BLOCK block;
    unsigned int sum = 0;
    int i, j;

    SoraStoreXMMSignalBlock(&block);
    for(j = 0; j < SORA_RX_SIGNAL_UNIT_COMPLEX16_NUM; j++)
    {
        for(i = 0; i < SORA_RX_SIGNAL_UNIT_NUM_PER_DESC; i += 2)
        {
            if (i + 1 < SORA_RX_SIGNAL_UNIT_NUM_PER_DESC)
            {
                sum += (abs(block.SampleUnit[i].Samples[j].re) + abs(block.SampleUnit[i+1].Samples[j].re) + 1) / 2;
                sum += (abs(block.SampleUnit[i].Samples[j].im) + abs(block.SampleUnit[i+1].Samples[j].im) + 1) / 2;
            }
            else
            {
                sum += abs(block.SampleUnit[i].Samples[j].re) / 2;
                sum += abs(block.SampleUnit[i].Samples[j].im) / 2;
            }
        }
    }
    return sum /= 2;
}
#endif

#pragma region "intrinsic version"
SORA_EXTERN_C
FINL 
void 
SoraStoreM128(
    OUT SORA_SAMPLE_BLOCK *pSampleBuf, 
    int index, 
    __m128i xmm)
{
    _mm_store_si128((__m128i*)(&pSampleBuf->SampleUnit[index]), xmm);
}

#define SoraStoreSampleBlock(pSampleBuf, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6) \
{\
    SoraStoreM128(pSampleBuf, 0, xmm0);\
    SoraStoreM128(pSampleBuf, 1, xmm1);\
    SoraStoreM128(pSampleBuf, 2, xmm2);\
    SoraStoreM128(pSampleBuf, 3, xmm3);\
    SoraStoreM128(pSampleBuf, 4, xmm4);\
    SoraStoreM128(pSampleBuf, 5, xmm5);\
    SoraStoreM128(pSampleBuf, 6, xmm6);\
}

__inline HRESULT __WaitNewSignals(
                    IN PRX_BLOCK       pScanPoint, 
                    IN USHORT          uRetries,
                    IN ULONG           VStreamMask,
                    OUT FLAG           *fReachEnd);

#pragma endregion
