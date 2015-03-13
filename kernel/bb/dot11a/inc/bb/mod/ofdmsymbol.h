#ifndef BB_MOD_OFDMSYMBOL_H
#define BB_MOD_OFDMSYMBOL_H

#include "vector128.h"
#include "bb/bba.h"
#include "bb/mod/amap.h"
#include "bb/mod/ainterleave.h"
#include "bb/mod/convenc.h"
#include "bb/mod/addpilot.h"
#include "bb/mod/copynt.h"
#include "bb/mod/ifft64x.h"
#include "bb/mod/upsample.h"

DSP_INLINE void CopyGI(COMPLEX16 * pcTo, COMPLEX16 * pcFrom)
{
    // Copy 8 vcs
    memcpy(pcTo, pcFrom, sizeof(vcs) * 8);
}

DSP_INLINE
void Window(COMPLEX16 * pcSymbol, COMPLEX16 * pcLast)
{
    pcSymbol[0].re >>= 2;
    pcSymbol[0].im >>= 2;
    pcSymbol[1].re >>= 1;
    pcSymbol[1].im >>= 1;
    pcSymbol[2].re -= pcSymbol[2].re >> 2;
    pcSymbol[2].im -= pcSymbol[2].im >> 2;

    *(vcs*)pcSymbol = saturated_add(*(vcs*)pcSymbol, *(vcs*)pcLast);

    pcSymbol[0].re += pcLast->re;
    pcSymbol[0].im += pcLast->im;

    pcLast[0].re = pcSymbol[32].re - (pcSymbol[32].re >> 2);
    pcLast[0].im = pcSymbol[32].im - (pcSymbol[32].im >> 2);

    pcLast[1].re = pcSymbol[33].re >> 1;
    pcLast[1].im = pcSymbol[33].im >> 1;

    pcLast[2].re = pcSymbol[34].re >> 2;
    pcLast[2].im = pcSymbol[34].im >> 2;
    
    pcLast[3].re = 0;
    pcLast[3].im = 0;
}

__forceinline
unsigned int UpsampleAndCopyNT(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, const COMPLEX16 * pcInput, unsigned int sizeInput)
{
    if (info->SampleRate == 44)
    {
        assert(sizeInput % 160 == 0);

        const unsigned int COMPLEX_PER_OFDM_SYMBOL_44M = COMPLEX_PER_OFDM_SYMBOL/10*11;
        size_t i;
        for (i = 0; i < sizeInput; i += 160)
        {
            Upsample40MTo44M_160(pcInput + i, info->cSymbol44M);
            Copy_NT(pcOutput, info->cSymbol44M, COMPLEX_PER_OFDM_SYMBOL_44M);
            pcOutput += COMPLEX_PER_OFDM_SYMBOL_44M;
        }
        return sizeInput/10*11;
    }
    else if (info->SampleRate == 40)
    {
        Copy_NT(pcOutput, pcInput, sizeInput);
        return sizeInput;
    }
    else
    {
        ASSERT(0);
        return 0;
    }
}

__forceinline
void UpsampleTailAndCopyNT(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput)
{
    if (info->SampleRate == 44)
    {
        Upsample40MTo44M_3(info->cWindow, info->cSymbol44M);
        set_zero(((vcs*)info->cSymbol44M)[1]);
        Copy_NT(pcOutput, info->cSymbol44M, 8);
    }
    else if (info->SampleRate == 40)
    {
        Copy_NT(pcOutput, info->cWindow, 8);
    }
    else
    {
        ASSERT(0);
    }
}

__forceinline
unsigned int Generate6MSymbol(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_1_2(pbData, info->bEncoded, 3, info->bConvEncoderReg);
    InterleaveBPSK(info->bEncoded, info->bInterleaved);
    MapBPSK_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

__forceinline
unsigned int Generate9MSymbol1(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_3_4_9MSpecial1(pbData, info->bEncoded, info->bConvEncoderReg);
    InterleaveBPSK(info->bEncoded, info->bInterleaved);
    MapBPSK_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

__forceinline
unsigned int Generate9MSymbol2(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast,
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_3_4_9MSpecial2(pbData, info->bEncoded, info->bConvEncoderReg);
    InterleaveBPSK(info->bEncoded, info->bInterleaved);
    MapBPSK_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

__forceinline
unsigned int Generate12MSymbol(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_1_2(pbData, info->bEncoded, 6, info->bConvEncoderReg);
    InterleaveQPSK(info->bEncoded, info->bInterleaved);
    MapQPSK_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

__forceinline
unsigned int Generate18MSymbol(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_3_4(pbData, info->bEncoded, 9, info->bConvEncoderReg);

    InterleaveQPSK(info->bEncoded, info->bInterleaved);
    MapQPSK_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

__forceinline
unsigned int Generate24MSymbol(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_1_2(pbData, info->bEncoded, 12, info->bConvEncoderReg);

    Interleave16QAM(info->bEncoded, info->bInterleaved);
    Map16QAM_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

__forceinline
unsigned int Generate36MSymbol(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_3_4(pbData, info->bEncoded, 18, info->bConvEncoderReg);

    Interleave16QAM(info->bEncoded, info->bInterleaved);
    Map16QAM_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

__forceinline
unsigned int Generate48MSymbol(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_2_3(pbData, info->bEncoded, 24, info->bConvEncoderReg);

    Interleave64QAM(info->bEncoded, info->bInterleaved);
    Map64QAM_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

__forceinline
unsigned int Generate54MSymbol(PBB11A_TX_VECTOR info, COMPLEX8 * pcOutput, COMPLEX16 * pcLast, 
        char * pbData, unsigned char bPilotSgn)
{
    ConvEncode_3_4(pbData, info->bEncoded, 27, info->bConvEncoderReg);

    Interleave64QAM(info->bEncoded, info->bInterleaved);
    Map64QAM_11a(info->bInterleaved, info->cMapped);

    AddPilot(info->cMapped, info->cPilotAdded, bPilotSgn);
    IFFT64x(info->cPilotAdded, info->cSymbol + COMPLEX_GI);

    CopyGI(info->cSymbol, info->cSymbol + 128);
    Window(info->cSymbol, pcLast);
    return UpsampleAndCopyNT(info, pcOutput, info->cSymbol, COMPLEX_PER_OFDM_SYMBOL);
}

#endif//BB_MOD_OFDMSYMBOL_H
