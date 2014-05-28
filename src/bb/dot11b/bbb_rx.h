#pragma once

#ifndef __cplusplus
#error "Must be compiled under C++"
#endif

#include "bbb_fifos.h"
#include "bbb_debarker.h"
#include "bbb_dbpsk.h"
#include "bbb_dqpsk.h"
#include "bbb_cck5.h"
#include "bbb_cck11.h"
#include "bbb_descramble.h"
#include "crc16.h"
#include "crc32.h"
#include "soradsp.h"

SORA_EXTERN_C
void BB11BInitRxContext(OUT PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
int BB11BGetBarkerPeak(COMPLEX16 *input);

SORA_EXTERN_C
void BB11BBarkerSyncCheck(PBB11B_RX_CONTEXT pRxContext, int maxIndex);

SORA_EXTERN_C
BOOLEAN BB11BCheckSFD(PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
HRESULT BB11BParsePLCPHeader(PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
HRESULT BB11BCheckFrameDataCRC32(PBB11B_RX_CONTEXT pRxContext);

SORA_EXTERN_C
HRESULT BB11BValidLineCheck(PBB11B_RX_CONTEXT pRxContext, USHORT max);

SORA_EXTERN_C FINL HRESULT BB11BSelectSampleLine(
    IN PUSHORT                  pDownSampleSum,
    OUT int *                   maxIndex,
    OUT unsigned short *        max,
    IN int                      curIndex,
    IN BOOLEAN                  isDecoding
    );

SORA_EXTERN_C int BB11BCCK5p5DecodeHalfByte(
    COMPLEX16 *                 input, 
    UCHAR *                     output, 
    COMPLEX16 *                 ref, 
    IN OUT BOOLEAN *            is_even
    );

/*++ 
BB11BNewFrameReset resets 802.11b RX context before decoding a new frame.
--*/
SORA_EXTERN_C
FINL void BB11BNewFrameReset(PBB11B_RX_CONTEXT pRxContext)
{
    pRxContext->BB11bCommon.b_dataByteExpecting   = 0;
    pRxContext->BB11bCommon.b_dataByteDemodulated = 0;
    pRxContext->BB11bCommon.b_dataByteDescrambled = 0;
    pRxContext->BB11bCommon.b_barkerIndicator     = -1;
    pRxContext->BB11bCommon.b_demodulateRate      = DR_ALG; //decode phase to Alignment

    pRxContext->b_energyLeast                     = -1;
    pRxContext->b_downSampleIndicator             = -1;
    
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
    rxFifos->BufDemod.Clear();
    rxFifos->BufFrame.Clear();
#ifdef USER_MODE
    rxFifos->BufDSSymbol.Clear();
    rxFifos->BufDecodeByte.Clear();
#endif
}

/*++ 
BB11BAlignBarker will find the barker code board, and set 
b_demodulateRate field of pRxContext 
--*/
FINL HRESULT BB11BAlignBarker(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
    int checkRet;
    COMPLEX16 *pc = NULL;
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
    
    while(1)
    {
        checkRet = rxFifos->BufDemod.RCheck(22);
        if (!checkRet) return BB11B_NO_ENOUGH_SIGNAL;

        pc = rxFifos->BufDemod.RNextPt(11);
        BB11BBarkerSyncCheck(pRxContext, BB11BGetBarkerPeak(pc));
        rxFifos->BufDemod.RFlush();

        if (pRxContext->BB11bCommon.b_demodulateRate != DR_ALG) return BB11B_BARKER_ALIGNED;
    }
}

FINL void BarkerDespread(COMPLEX16 *input, COMPLEX16 *output)
{
    COMPLEX32 tp;
    tp = DESPREAD_BARKER(input);
    output->re = (short)(tp.re >> 4);
    output->im = (short)(tp.im >> 4);
#if defined(USER_MODE) && defined(BBB_VALIDATION)
    assert(output->re == tp.re >> 4);
    assert(output->im == tp.im >> 4);
#endif
}

FINL void DBPSKDemapBit(char *b, int pos, COMPLEX16 *ref, COMPLEX16 *input)
{
    DEMAP_BIT_DBPSK(*b, pos, (*ref), (*input));
    *ref = *input;
}

FINL HRESULT BB11BDecodePreamble(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
    int checkRet;
    COMPLEX16 *pc = NULL;
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
    
    while (1)
    {
        COMPLEX16 comp16;

        checkRet = rxFifos->BufDemod.RCheck(11);
        if (!checkRet) return BB11B_NO_ENOUGH_SIGNAL;
        pc = rxFifos->BufDemod.RNextPt(11);
        
        BarkerDespread(pc, &comp16);
        rxFifos->BufDemod.RFlush();

        pRxContext->BB11bCommon.b_SFDReg <<= 1;
        
        DBPSKDemapBit((char *)&(pRxContext->BB11bCommon.b_SFDReg), 
            0, &(pRxContext->b_cLast), &comp16);

        pRxContext->BB11bCommon.b_barkerIndCount++;

        OutputBitInByteToBitStream(&rxFifos->BufDecodeByte, (BYTE)pRxContext->BB11bCommon.b_SFDReg, 0);

        if (BB11BCheckSFD(pRxContext))
        {
#if USER_MODE
            QueryPerformanceCounter(&pRxContext->BB11bCommon.b_SFDFindTimeStamp);
#else
            pRxContext->BB11bCommon.b_SFDFindTimeStamp = KeQueryPerformanceCounter(NULL);
#endif
            return BB11B_PREAMBLE_DECODE_END;
        }
    }
}

FINL void BarkerDespreadN(COMPLEX16 * input, int n, COMPLEX16 * output)
{
    int i;
    for (i = 0; i < n; i++)
    {
        BarkerDespread(input, &output[i]);
        input += 11;
    }
}

FINL UCHAR DBPSKDemapByte(COMPLEX16 *input, COMPLEX16 *ref)
{
    UCHAR ret = 0;
    DEMAP_BIT_DBPSK(ret, 0,   (*ref), input[0]);
    DEMAP_BIT_DBPSK(ret, 1, input[0], input[1]);
    DEMAP_BIT_DBPSK(ret, 2, input[1], input[2]);
    DEMAP_BIT_DBPSK(ret, 3, input[2], input[3]);
    DEMAP_BIT_DBPSK(ret, 4, input[3], input[4]);
    DEMAP_BIT_DBPSK(ret, 5, input[4], input[5]);
    DEMAP_BIT_DBPSK(ret, 6, input[5], input[6]);
    DEMAP_BIT_DBPSK(ret, 7, input[6], input[7]);
    *ref = input[7];
    return ret;
}

FINL HRESULT BB11BDecodePartial1MBarker(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
    int checkRet;
    unsigned char b;
    COMPLEX16 *pc;
    COMPLEX16 items[8];
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
    
    while (pRxContext->BB11bCommon.b_dataByteDemodulated
        < pRxContext->BB11bCommon.b_dataByteExpecting)
    {
        checkRet = rxFifos->BufDemod.RCheck(88);
        if (!checkRet) return BB11B_NO_ENOUGH_SIGNAL;

        pc = rxFifos->BufDemod.RNextPt(88);
        BarkerDespreadN(pc, 8, items);
        rxFifos->BufDemod.RFlush();

        b = DBPSKDemapByte(items, &(pRxContext->b_cLast));
        rxFifos->BufFrame.Write(b);
        
        pRxContext->BB11bCommon.b_dataByteDemodulated++;

        OutputByteToBitStream(&rxFifos->BufDecodeByte, b);
    }
    return BB11B_1M_DATARATE_DECODED;
}

FINL UCHAR DQPSKDemapByte(COMPLEX16 *input, COMPLEX16 *ref)
{
    UCHAR ret = 0;
    DEMAP_BIT_DQPSK(ret, 0, (*ref), input[0]);
    DEMAP_BIT_DQPSK(ret, 2, input[0], input[1]);
    DEMAP_BIT_DQPSK(ret, 4, input[1], input[2]);
    DEMAP_BIT_DQPSK(ret, 6, input[2], input[3]);
    *ref = input[3];
    return ret;
}

FINL HRESULT BB11BDecodePartial2MBarker(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
    int checkRet;
    unsigned char b;
    PCOMPLEX16 pc;
    COMPLEX16 items[4];
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
    
    while (pRxContext->BB11bCommon.b_dataByteDemodulated
        < pRxContext->BB11bCommon.b_dataByteExpecting)
    {
        checkRet = rxFifos->BufDemod.RCheck(44);
        if (!checkRet) return BB11B_NO_ENOUGH_SIGNAL;

        pc = rxFifos->BufDemod.RNextPt(44);
        BarkerDespreadN(pc, 4, items);
        rxFifos->BufDemod.RFlush();

        b = DQPSKDemapByte(items, &(pRxContext->b_cLast));
        rxFifos->BufFrame.Write(b);

        pRxContext->BB11bCommon.b_dataByteDemodulated++;

        OutputByteToBitStream(&rxFifos->BufDecodeByte, b);
    }
    return BB11B_2M_DATARATE_DECODED;
}

FINL HRESULT BB11BDecodePartial5MCCK(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
    int checkRet;
    unsigned char b;
    PCOMPLEX16 pc;
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
    
    while (pRxContext->BB11bCommon.b_dataByteDemodulated
        < pRxContext->BB11bCommon.b_dataByteExpecting)
    {
        checkRet = rxFifos->BufDemod.RCheck(8);
        if (!checkRet) return BB11B_NO_ENOUGH_SIGNAL;

        pc = rxFifos->BufDemod.RNextPt(8);

        if (BB11BCCK5p5DecodeHalfByte(pc, &(pRxContext->BB11bCommon.b_byteTemp5M),
            &(pRxContext->b_cLast), &(pRxContext->BB11bCommon.b_isEven)))
        {
            b = pRxContext->BB11bCommon.b_byteTemp5M;
            rxFifos->BufFrame.Write(b);

            pRxContext->BB11bCommon.b_dataByteDemodulated++;

            OutputByteToBitStream(&rxFifos->BufDecodeByte, b);
        }
        rxFifos->BufDemod.RFlush();
    }
    return BB11B_5M_DATARATE_DECODED;
}

FINL UCHAR BB11BCCK11DecodeByte(COMPLEX16 *input, COMPLEX16 *ref, BOOLEAN *is_even)
{
    UCHAR ret;

    CCK11_DECODER(&ret, input[0], input[1], input[2], input[3],
            input[4], input[5], input[6], input[7], ref, (*is_even),
            _CCK_FIL_L41, _CCK_FIL_LE1);

    return ret;
}

FINL HRESULT BB11BDecodePartial11MCCK(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
    int checkRet;
    unsigned char b;
    PCOMPLEX16 pc;
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
    
    while (pRxContext->BB11bCommon.b_dataByteDemodulated
        < pRxContext->BB11bCommon.b_dataByteExpecting)
    {
        // unsigned char bOutput;
        checkRet = rxFifos->BufDemod.RCheck(8);
        if (!checkRet) return BB11B_NO_ENOUGH_SIGNAL;

        pc = rxFifos->BufDemod.RNextPt(8);

        b = BB11BCCK11DecodeByte(pc, &(pRxContext->b_cLast), &(pRxContext->BB11bCommon.b_isEven));
        rxFifos->BufFrame.Write(b);

        rxFifos->BufDemod.RFlush();

        pRxContext->BB11bCommon.b_dataByteDemodulated++;

        OutputByteToBitStream(&rxFifos->BufDecodeByte, b);
    }
    return BB11B_11M_DATARATE_DECODED;
}

SORA_EXTERN_C
FINL UCHAR BB11BDescramble(UCHAR input, PUCHAR seed)
{
    UCHAR ret;
    DESCRAMBLE(ret, input, (*seed));
    *seed = input;
    return ret;
}

SORA_EXTERN_C
FINL HRESULT BB11BDescramblePartial(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
    int checkRet;
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
    
    if (pRxContext->BB11bCommon.b_dataByteExpecting == 0)
        return BB11B_NO_ENOUGH_SCRAMBLED;

    while (pRxContext->BB11bCommon.b_dataByteDescrambled
        < pRxContext->BB11bCommon.b_dataByteExpecting)
    {
        unsigned char bOutput;

        checkRet = rxFifos->BufFrame.RCheck(1);
        if (!checkRet) return BB11B_NO_ENOUGH_SCRAMBLED;

        unsigned char b = rxFifos->BufFrame.Read();
        bOutput = BB11BDescramble(b, &(pRxContext->BB11bCommon.b_scrambleSeed));
        pRxContext->BB11bCommon.b_dataOutputBuffer
            [pRxContext->BB11bCommon.b_dataByteDescrambled++] = (char)(bOutput);

        CalcCRC32Incremental(bOutput, &(pRxContext->BB11bCommon.b_crc32));

        if (pRxContext->BB11bCommon.b_dataByteExpecting 
            == pRxContext->BB11bCommon.b_dataByteDescrambled + 4) // 4 = CRC32 Length
        {
            pRxContext->BB11bCommon.b_crc32Store = ~ pRxContext->BB11bCommon.b_crc32;
        }
    }
    return BB11B_DESCRAMBLE_FINISH;
}

SORA_EXTERN_C
FINL BOOLEAN BB11BDecodingPLCPHeader(IN PBB11B_RX_CONTEXT pRxContext)
{
    return pRxContext->BB11bCommon.b_dataOutputBuffer == (PUCHAR)&pRxContext->BB11bCommon.b_PLCPHeader;
}

SORA_EXTERN_C
FINL HRESULT BB11BSwitchPLCPHeaderAndData(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
    HRESULT hr = S_OK;
    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
   
    if (BB11BDecodingPLCPHeader(pRxContext))
    {
        hr = BB11BParsePLCPHeader(pRxContext);
        if (FAILED(hr))
        {
            pRxContext->BB11bCommon.b_demodulateRate      = DR_ALG;
            pRxContext->BB11bCommon.b_dataByteExpecting   = 0;
            pRxContext->BB11bCommon.b_barkerIndicator     = -1;
            rxFifos->BufFrame.Clear();
            pRxContext->BB11bCommon.b_errPLCPHeader++;
        }
    }
    else // For frame bytes
    {
        hr = BB11BCheckFrameDataCRC32(pRxContext);

        // Reset all
        pRxContext->BB11bCommon.b_dataByteExpecting   = 0;
        pRxContext->BB11bCommon.b_prevDataRate        = pRxContext->BB11bCommon.b_demodulateRate;
        pRxContext->BB11bCommon.b_demodulateRate      = DR_ALG;
        pRxContext->b_energyLeast                     = -1;
        pRxContext->b_downSampleIndicator             = -1;
        pRxContext->BB11bCommon.b_barkerIndicator     = -1;
        rxFifos->BufDemod.Clear();
#ifdef USER_MODE
        rxFifos->BufDSSymbol.Clear();
#endif
    }
    return hr;
}

template<class BUF_TYPE>
FINL HRESULT BB11BDecimateSignalBlock(
        IN OUT int *        curIndex,
        IN int              maxIndex,
        IN PCOMPLEX16       input,
        IN OUT BUF_TYPE *   cb
#ifdef USER_MODE
        , IN OUT BUF_TYPE * cbSymbol
        , IN OUT int *      rgDSIndex
#endif
        )
{
    COMPLEX16 *pc;
    unsigned int i;

#ifdef USER_MODE
    int nTickIndex = 0;
#endif

    ASSERT(*curIndex >= -1 && *curIndex < 4);
    ASSERT( maxIndex >=  0 &&  maxIndex < 4);

    if (*curIndex == 3 && maxIndex == 0)
    {
        for (i = 4; i < 28; i += 4)
        {
            pc = cb->WNextPt(1);
            *pc = input[i];
#ifdef USER_MODE
            //pc = cbSymbol->WNextPt(1);
            //*pc = input[i];
            //rgDSIndex[nTickIndex++] = i;
#endif
        }
        cb->WFlush();
#ifdef USER_MODE
        //cbSymbol->WFlush();
#endif
        *curIndex = maxIndex;

        return S_OK;
    }
    else if (*curIndex == 0 && maxIndex == 3)
    {
        pc = cb->WNextPt(1);
        *pc = input[0];
#ifdef USER_MODE
        //pc = cbSymbol->WNextPt(1);
        //*pc = input[0];
        //rgDSIndex[nTickIndex++] = 0;
#endif
        for (i = 3; i < 28; i += 4)
        {
            pc = cb->WNextPt(1);
            *pc = input[i];
#ifdef USER_MODE
            //pc = cbSymbol->WNextPt(1);
            //*pc = input[i];
            //rgDSIndex[nTickIndex++] = i;
#endif
        }
        cb->WFlush();
#ifdef USER_MODE
        //cbSymbol->WFlush();
#endif
        *curIndex = maxIndex;

        return S_OK;
    }
    else if (*curIndex >= 0 && ((maxIndex - *curIndex > 1) || (maxIndex - *curIndex < -1)))
    {
        // this is guarrented by algorithm in BB11BSelectSampleLine()
        ASSERT(0);
    }

    for (i = maxIndex; i < 28; i += 4)
    {
        pc = cb->WNextPt(1);
        *pc = input[i];
#ifdef USER_MODE
        //pc = cbSymbol->WNextPt(1);
        //*pc = input[i];
        //rgDSIndex[nTickIndex++] = i;
#endif
    }
    cb->WFlush();
#ifdef USER_MODE
    //cbSymbol->WFlush();
#endif
    *curIndex = maxIndex;

    return S_OK;
}

FINL HRESULT BB11BDownSample(IN OUT PBB11B_RX_CONTEXT pRxContext, __m128i *pDownSampleSum)
{
    HRESULT hr;
    int maxIndex = 0;
    unsigned short max;

    hr = BB11BSelectSampleLine(
        (USHORT*)pDownSampleSum, &maxIndex, &max,
        pRxContext->b_downSampleIndicator,
        pRxContext->BB11bCommon.b_dataByteExpecting > 0);
    if (FAILED(hr)) return hr;

    hr  = BB11BValidLineCheck(pRxContext, max);
    if (SUCCEEDED(hr))
    {
        BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;

        SignalBlock& block = *(SignalBlock *)pRxContext->DescBuffer;

        if (pRxContext->b_shiftRight > 0)
            SoraRightShiftSignalBlock(block, pRxContext->b_shiftRight);
        else if (pRxContext->b_shiftRight < 0)
            SoraLeftShiftSignalBlock(block, - pRxContext->b_shiftRight);
        // else zero, no need to shift at all

        hr = BB11BDecimateSignalBlock(&(pRxContext->b_downSampleIndicator),
            maxIndex, pRxContext->DescBuffer, &(rxFifos->BufDemod)
#ifdef USER_MODE
            , &(rxFifos->BufDSSymbol)
            , (int *)pRxContext->rgDSIndex
#endif
            );
    }
    return hr;
}

FINL USHORT BB11BCalPLCPHeaderCRC16(
    IN OUT PDOT11B_PLCP_HEADER pPLCPHeader)
{
    USHORT ret = 0xFFFF;
    CalcCRC16Incremental(pPLCPHeader->Signal, &ret);
    CalcCRC16Incremental(pPLCPHeader->Service.bValue, &ret);
    CalcCRC16Incremental(pPLCPHeader->LengthBytes[0], &ret);
    CalcCRC16Incremental(pPLCPHeader->LengthBytes[1], &ret);

    return ~ret;
}
