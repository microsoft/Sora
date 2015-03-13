/*++
Copyright (c) Microsoft Corporation

Module Name: 802.11b RX module using inline SSE assembly instructions.

Abstract: 
    The file includes 802.11b physical layer (PLCP, PMD) init and 
    RX function.

History: 
          3/12/2009: Modified by senxiang
--*/
#include <stdio.h>
#include <math.h>
#include "vector128.h"
#include "CRC16.h"
#include "bb/bbb.h"
#include "bbb_fifos.h"
#include "bbb_rx.h"
#include "soradsp.h"
#include "complex_ext.h"

#define DT_NOTHING      0x00
#define DT_LONG_HEADER  0x01
#define DT_SHORT_HEADER 0x02
#define DT_DATA         0x10

/*++
BB11BInitRxContext initializes 802.11b base band rx context.

 Parameters: 
            pRxContext:     point to a 802.11b base band rx context. 
                            See more about BB11B_RX_CONTEXT.

 Return:

 Note:      BB11B_RX_CONTEXT context structure includes RX-required intermedate 
            status, i.e., b_dataByteExpecting (data bytes left to be decoded), 
            b_demodulateRate ( current demodulate phase). BB11BInitRxContext sets it 
            to start status.

 History:   12/05/2009 Modified by senxiang -- remove global variable
--*/
void BB11BInitRxContext(OUT PBB11B_RX_CONTEXT pRxContext)
{
    pRxContext->BB11bCommon.b_errEnergyLoss       = 0;
    pRxContext->BB11bCommon.b_errPLCPHeader       = 0;
    pRxContext->BB11bCommon.b_errFrame            = 0;
    pRxContext->BB11bCommon.b_goodFrameCounter    = 0;

    pRxContext->b_energyLeast                     = -1;
    pRxContext->b_downSampleIndicator             = -1;
    pRxContext->BB11bCommon.b_barkerIndicator     = -1;

    pRxContext->BB11bCommon.b_demodulateRate     = DR_ALG;
    pRxContext->BB11bCommon.b_dataByteExpecting  = 0;    
}

/*++++
 Kun: missing discriptions
---*/
SORA_EXTERN_C FINL HRESULT BB11BSelectSampleLine(
    IN PUSHORT              pDownSampleSum,
    OUT int *               maxIndex,
    OUT unsigned short *    max,
    IN int                  curIndex,
    IN BOOLEAN              isDecoding)
{
    size_t start, end, i;
    if (curIndex < 0)
    {
        start = 0;
        end = M128_DWORD_NUM;
    }
    else
    {
        start = curIndex + M128_DWORD_NUM/2 + 1;
        end = curIndex + M128_DWORD_NUM/2 + M128_DWORD_NUM;
    }

    *max = pDownSampleSum[(start % M128_DWORD_NUM) * 2];
    *maxIndex = start % M128_DWORD_NUM;
    for (i = start + 1; i < end; i++)
    {
        if (*max < pDownSampleSum[(i % M128_DWORD_NUM) * 2])
        {
            *max = pDownSampleSum[(i % M128_DWORD_NUM) * 2];
            *maxIndex = i % M128_DWORD_NUM;
        }
    }

#if defined(USER_MODE) && defined(BBB_VALIDATION)
    assert(*maxIndex >= 0 && *maxIndex < M128_DWORD_NUM);
#endif

    // Terrible down sample: the real max value in pDownSampleSum appears in reverse phase
    if (!isDecoding && curIndex >= 0 && pDownSampleSum[((curIndex + M128_DWORD_NUM/2) % M128_DWORD_NUM) * 2] > *max)
        return BB11B_E_DOWNSAMPLE;
    return S_OK;
}

/*++++
 Kun: missing discriptions
---*/
SORA_EXTERN_C
FINL HRESULT BB11BValidLineCheck(PBB11B_RX_CONTEXT pRxContext, USHORT max)
{
    if (max < pRxContext->b_energyLeast)
    {
        if (!(pRxContext->BB11bCommon.b_dataByteExpecting > 0 && 
                    (pRxContext->BB11bCommon.b_dataByteExpecting - 
                    pRxContext->BB11bCommon.b_dataByteDemodulated <= 5)))
        {
            pRxContext->BB11bCommon.b_errEnergyLoss++;
            pRxContext->BB11bCommon.b_dataByteExpecting   = 0;
            pRxContext->BB11bCommon.b_demodulateRate      = DR_ALG;
            pRxContext->b_energyLeast                     = -1;
            pRxContext->b_downSampleIndicator             = -1;
            pRxContext->BB11bCommon.b_barkerIndicator     = -1;

            BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
            rxFifos->BufDemod.Clear();
            rxFifos->BufFrame.Clear();
            return BB11B_E_ENERGY;
        }
    }
    return S_OK;
}


int BB11BGetBarkerPeak(COMPLEX16 *input)
{
    unsigned int i;
    COMPLEX32 comp;
    unsigned int sum;
    unsigned int max = 0;
    unsigned int maxIndex = 0;
    unsigned int secondMax = 0;

    for (i = 0; i < 11; i++)
    {
        comp = DESPREAD_BARKER(&input[i]);
        sum = norm1(&comp);

        if (sum > max)
        {
            secondMax = max;
            max = sum;
            maxIndex = i;
        }
        else if (sum > secondMax)
        {
            secondMax = sum;
        }
    }

    if ((max >> 1) > secondMax)
        return maxIndex;
    return -1;
}

void BB11BBarkerSyncCheck(PBB11B_RX_CONTEXT pRxContext, int maxIndex)
{
    COMPLEX16 *pc;

    if (maxIndex >= 0)
    {
        if (maxIndex == pRxContext->BB11bCommon.b_barkerIndicator) // same peek
        {
            pRxContext->BB11bCommon.b_barkerIndCount++; // counter++
            if (pRxContext->BB11bCommon.b_barkerIndCount > 2) // twice same peek
            {
                BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;
                pc = rxFifos->BufDemod.RNextPt(maxIndex);
                pRxContext->b_energyLeast = -1;
                pRxContext->BB11bCommon.b_demodulateRate  = DR_PREAMBLE;
            }
        }
        else
        {
            pRxContext->BB11bCommon.b_barkerIndCount = 0; // counter zero
        }
    }
    pRxContext->BB11bCommon.b_barkerIndicator = maxIndex; // set to new
}

/*++ 
If start frame delimitler is detected, Update RX context according to SFD.
If we have searched preamble for more than 200 times and still can't get SFD, 
set state to DR_ALG, so decoding process BB11BDecodePartial will switch to 
barker alignment. 
--*/
FINL BOOLEAN BB11BCheckSFD(PBB11B_RX_CONTEXT pRxContext)
{
    BOOLEAN fEndSFDCheck = TRUE;
    // long scrambled reverted SFD matched
    if (pRxContext->BB11bCommon.b_SFDReg == DOT11B_PLCP_SCRAMBLED_LONG_PREAM_SFD) //scrambled DOT11B_PLCP_LONG_PREAMBLE_SFD
    {
        pRxContext->BB11bCommon.b_demodulateRate          = DR_1M;
        pRxContext->BB11bCommon.b_dataByteDemodulated     = 0;
        pRxContext->BB11bCommon.b_dataByteDescrambled     = 0;
        pRxContext->BB11bCommon.b_scrambleSeed            = 0x92;
        pRxContext->BB11bCommon.b_dataOutputBuffer        = (PUCHAR)&pRxContext->BB11bCommon.b_PLCPHeader;
        pRxContext->BB11bCommon.b_dataByteExpecting       = sizeof(pRxContext->BB11bCommon.b_PLCPHeader);
        pRxContext->BB11bCommon.b_isLongPreamble          = 1;
    }
    // short scrambled reverted SFD matched
    else if (pRxContext->BB11bCommon.b_SFDReg == DOT11B_PLCP_SCRAMBLED_SHORT_PREAM_SFD) //scrambled DOT11B_PLCP_SHORT_PREAMBLE_SFD
    {
        pRxContext->BB11bCommon.b_demodulateRate          = DR_2M;
        pRxContext->BB11bCommon.b_dataByteDemodulated     = 0;
        pRxContext->BB11bCommon.b_dataByteDescrambled     = 0;
        pRxContext->BB11bCommon.b_scrambleSeed            = 0xFE;
        pRxContext->BB11bCommon.b_dataOutputBuffer        = (PUCHAR)&pRxContext->BB11bCommon.b_PLCPHeader;
        pRxContext->BB11bCommon.b_dataByteExpecting       = sizeof(pRxContext->BB11bCommon.b_PLCPHeader);
        pRxContext->BB11bCommon.b_isEven                  = 0;
        pRxContext->BB11bCommon.b_isLongPreamble          = 0;
    }
    else if (pRxContext->BB11bCommon.b_barkerIndCount > 200)
    {
        pRxContext->BB11bCommon.b_barkerIndicator = -1;
        pRxContext->BB11bCommon.b_demodulateRate  = DR_ALG; 
        // so decoding process BB11BDecodePartial will switch to barker alignment. 
    }
    else
    {
        fEndSFDCheck = FALSE;
    }
    
    return fEndSFDCheck;
}

FINL int BB11BCCK5p5DecodeHalfByte(
            COMPLEX16           *input, 
            UCHAR               *output, 
            COMPLEX16           *ref, 
            IN OUT BOOLEAN      *is_even)
{
    if (*is_even == 0)
    {
        *output = 0;
        CCK5P5_EVEN_DECODER(output, input[0], input[1], input[2], input[3],
                input[4], input[5], input[6], input[7], ref);
        *is_even = TRUE;
        return 0;
    }
    else
    {
        CCK5P5_ODD_DECODER(output, input[0], input[1], input[2], input[3],
                input[4], input[5], input[6], input[7], ref);
        *is_even = FALSE;
        return 1;
    }
}

FINL HRESULT BB11BParsePLCPHeader(PBB11B_RX_CONTEXT pRxContext)
{
    USHORT crc = BB11BCalPLCPHeaderCRC16(&pRxContext->BB11bCommon.b_PLCPHeader);
    USHORT len;
    UCHAR service;
    
    if (crc != pRxContext->BB11bCommon.b_PLCPHeader.CRC)
        return BB11B_E_PLCP_HEADER_CRC;

    len = pRxContext->BB11bCommon.b_PLCPHeader.Length;
    service = pRxContext->BB11bCommon.b_PLCPHeader.Service.bValue;

    switch (pRxContext->BB11bCommon.b_PLCPHeader.Signal)
    {
    case DOT11B_PLCP_DATA_RATE_1M:
        pRxContext->BB11bCommon.b_dataByteExpecting   = len >> 3;
        pRxContext->BB11bCommon.b_demodulateRate      = DR_1M;
        break;

    case DOT11B_PLCP_DATA_RATE_2M:
        pRxContext->BB11bCommon.b_dataByteExpecting   = len >> 2;
        pRxContext->BB11bCommon.b_demodulateRate      = DR_2M;
        break;

    case DOT11B_PLCP_DATA_RATE_5P5M:
        pRxContext->BB11bCommon.b_dataByteExpecting   = ((len * 11) >> 4) - (service >> 7) - ((service >> 3) & 0x1);
        pRxContext->BB11bCommon.b_demodulateRate      = DR_5M;
        break;

    case DOT11B_PLCP_DATA_RATE_11M:
        pRxContext->BB11bCommon.b_dataByteExpecting   = 
            ((len * 11) >> 3) - (service >> 7) - ((service >> 3) & 0x1);
        pRxContext->BB11bCommon.b_demodulateRate      = DR_11M;
        break;

    default:
        return BB11B_E_PLCP_HEADER_SIG;
    }

    if (pRxContext->BB11bCommon.b_dataByteExpecting > pRxContext->BB11bCommon.b_maxOutputSize)
        return BB11B_E_PLCP_HEADER_LEN;

    pRxContext->BB11bCommon.b_dataByteDemodulated   = 0;
    pRxContext->BB11bCommon.b_dataByteDescrambled   = 0;
    pRxContext->BB11bCommon.b_dataOutputBuffer      = pRxContext->BB11bCommon.b_outputPt;
    pRxContext->BB11bCommon.b_crc32                 = 0xFFFFFFFF;
    pRxContext->BB11bCommon.b_isEven                = 0;
    pRxContext->BB11bCommon.b_length                = pRxContext->BB11bCommon.b_dataByteExpecting;
    pRxContext->BB11bCommon.b_dataRate              = pRxContext->BB11bCommon.b_PLCPHeader.Signal;
    pRxContext->BB11bCommon.b_isModPBCC             = (pRxContext->BB11bCommon.b_PLCPHeader.Service.bValue >> 3) & 0x1;
    
    return BB11B_OK_PLCP_HEADER;
} 

FINL HRESULT BB11BCheckFrameDataCRC32(PBB11B_RX_CONTEXT pRxContext)
{
    HRESULT hr = S_OK;
    ULONG crc_received = *(PULONG) 
        (pRxContext->BB11bCommon.b_dataOutputBuffer + pRxContext->BB11bCommon.b_dataByteExpecting - 4);

    // 802.11 SDU CRC is at the end of MAC frame
    if ((pRxContext->BB11bCommon.b_dataByteExpecting > 4) 
        && pRxContext->BB11bCommon.b_crc32Store == crc_received)
    {
        pRxContext->BB11bCommon.b_goodFrameCounter++;
        hr = BB11B_OK_FRAME;
    }
    else
    {
        pRxContext->BB11bCommon.b_errFrame++;
        hr = BB11B_E_DATA;
    }
    return hr;
}

// Check if the context data are in expected state
// Returns: BB11B_E_SFD, BB11B_E_BARKER, BB11B_E_DOWNSAMPLE or S_OK;
SORA_EXTERN_C
FINL HRESULT BB11BContextIntegrityCheck(PBB11B_RX_CONTEXT pRxContext, unsigned int descCount)
{
    HRESULT hr = S_OK;
    if (descCount < pRxContext->b_maxDescCount || pRxContext->BB11bCommon.b_barkerIndicator < 0)
        return hr;

    if (pRxContext->BB11bCommon.b_barkerIndicator >= 0)
        hr = BB11B_E_SFD;
    else if (pRxContext->b_downSampleIndicator >= 0)
        hr = BB11B_E_BARKER;
    else
        hr = BB11B_E_DOWNSAMPLE;

    return hr;
}

SORA_EXTERN_C
FINL void BB11BDecodePartial(IN OUT PBB11B_RX_CONTEXT pRxContext)
{
DemodulateStart:
    switch (pRxContext->BB11bCommon.b_demodulateRate)
    {
    case DR_ALG:
        if (BB11B_BARKER_ALIGNED == BB11BAlignBarker(pRxContext))
        {
            goto DemodulateStart;
        }
        break;

    case DR_PREAMBLE:
        if (BB11B_PREAMBLE_DECODE_END == BB11BDecodePreamble(pRxContext))
        {
            goto DemodulateStart;
        }
        break;
    case DR_1M:
        BB11BDecodePartial1MBarker(pRxContext);
        break;
    case DR_2M:
        BB11BDecodePartial2MBarker(pRxContext);
        break;
    case DR_5M:
        BB11BDecodePartial5MCCK(pRxContext);
        break;
    case DR_11M:
        BB11BDecodePartial11MCCK(pRxContext);
        break;
    }
}

/*++
BB11BRx demodulate a frame from one or more radio RX stream.

Parameters:
    pRxContext:     802.11b base band RX context
    pRxInfoBase:    Radio RX stream handle

Return Value:
    BB11B_E_ENERGY if sample energy of selected line is too small.
    E_FETCH_SIGNAL_HW_TIMEOUT if we can't get more signal in fixed time.
    BB11B_E_PLCP_HEADER_XXX if PLCP header is not correct.
    BB11B_OK_FRAME if one good frame is decoded.
    BB11B_E_DATA if frame CRC is not correct.
--*/

HRESULT BB11BRx(PBB11B_RX_CONTEXT pRxContext, PSORA_RADIO_RX_STREAM pRxStream)
{
    FLAG *b_workIndicator = pRxContext->b_workIndicator;        // pointer to flag, 0 for force stop, 1 for work
    FLAG touched;
    HRESULT hr;
    ULONG PeekBlockCount = 0;
    vcs& DcOffsetSlot = (vcs&)pRxContext->DcOffsetSlot;

    // Remove DC offset, due to hardware flaw
    set_all(DcOffsetSlot, pRxContext->b_dcOffset);

    if (pRxContext->b_resetFlag)
        BB11BNewFrameReset(pRxContext);

    BB11B_RX_FIFOS *rxFifos = pRxContext->b_rxFifos;

    SignalBlock block;
    while (1)
    {
        // Check if the context data are in expected state
        hr = BB11BContextIntegrityCheck(pRxContext, PeekBlockCount);
        FAILED_BREAK(hr);

        // Fetch signal block from RxStream to XMM registers
        hr = SoraRadioReadRxStream(pRxStream, &touched, block);
        FAILED_BREAK(hr);

        // Check whether force stopped
        if (*b_workIndicator == 0)
        {
            hr = BB11B_E_FORCE_STOP;
            break;
        }

        // Remove DC offset from singal block in XMM registers
        RemoveDC(block, DcOffsetSlot);

		// Advance to next signal block in the RxStream
        PeekBlockCount++;
		
        if (pRxContext->b_energyLeast == -1)
        {
            // Estimate energy scope
            pRxContext->b_energyLeast = SoraEstimateEnergyScope(block);

            // Divide by 32, used to set check threashold for down sampling
            pRxContext->b_energyLeast >>= 5;
        }
        // Kun: we should do AGC here
        *(SignalBlock *)pRxContext->DescBuffer = block;

        // Calculates sample norm1 and packs them into 4 slots array
        *(vcs *)pRxContext->DownSampleSum = SoraPackNorm1SignalBlock(block);

        // Down sample based on the 4 slots array
        hr = BB11BDownSample(pRxContext, (__m128i*)pRxContext->DownSampleSum);
        if (hr == BB11B_E_ENERGY) break;
        if (hr == BB11B_E_DOWNSAMPLE) continue;

        const size_t burstSize = 32;
        // If BufDemod buffer has enough down sampled sample, ready for decoding
        if (rxFifos->BufDemod.RCheck(burstSize))
        {
            // Demodulate BufDemod buffer, output undescrambled bytes    
            BB11BDecodePartial(pRxContext);
            if (BB11B_DESCRAMBLE_FINISH == BB11BDescramblePartial(pRxContext))
            {
                // For frame data, just return; for Good Header, continue decoding
                hr = BB11BSwitchPLCPHeaderAndData(pRxContext);
                if (BB11B_OK_PLCP_HEADER != hr)
                {
                    // Have decoded a fault PLCP header or data frame, both exits
                    break;
                } 
            } // Descramble a complete frame
        } 
    } // Signal block process

    return hr;
}
