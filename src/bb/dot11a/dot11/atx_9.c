 
#include "bb/bba.h"

#include "atx.h"

#define DBPS DBPS_9M
#define CBPS DBPS_9M
#define BPSC BPSC_9M
#define SIGNAL_RATE SIGNAL_RATE_9M
#define GenerateSymbol Generate9MSymbol

#include "atx_tpl.h"

HRESULT Dot11aTxFrameEncode9(PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket)
{
    unsigned int uiScrambledSize;
    unsigned int uiSymbolCountData;
    unsigned int uiPilotIndex = 0;
    unsigned int i;
    char * pbInput;
    PTXSAMPLE pcOutput;
    ULONG PacketSizePlusCRC32 = pPacket->PacketSize + 4;

    ClearWindow(info->cWindow);

    uiScrambledSize = SERVICE_LEN_IN_BITS + PADDING_LEN_IN_BITS
        + (PacketSizePlusCRC32 << BITS_PER_BYTE_SHIFT);
    uiSymbolCountData = (uiScrambledSize + (DBPS - 1)) / DBPS;

    ULONG SignalBytes = GetSignalBytes(info, uiSymbolCountData);
    
    ULONG SampleBufferSize = 0;
    SoraPacketGetTxSampleBuffer(pPacket, (PTXSAMPLE *)&pcOutput, &SampleBufferSize);

    ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(pcOutput, SignalBytes);
    SoraPacketSetSignalLength(pPacket, SignalBytes);
    
    // Copy preamble
    pcOutput += CopyPreamble16_NT(info, pcOutput, info->cWindow);

    ConvEncodeReset(info->bConvEncoderReg);
    // Generate Signal
    pcOutput += GenerateSignal(info, pcOutput, info->cWindow, SIGNAL_RATE,
            (unsigned short)PacketSizePlusCRC32);
    
    // Scramble Data
    if (!(uiSymbolCountData & 0x1))
    {
        Scramble11a(pPacket->pMdl, info->bFrameScrambled,
            (uiSymbolCountData >> 1) * (DBPS * 2 / BITS_PER_BYTE),
            pPacket->Reserved1, info->ulRadom);
    }
    else
    {
        Scramble11a(pPacket->pMdl, info->bFrameScrambled,
                (uiSymbolCountData >> 1) * (DBPS * 2 / BITS_PER_BYTE)
                + ((DBPS / BITS_PER_BYTE) + 1),
                pPacket->Reserved1, info->ulRadom);
    }

    ConvEncodeReset(info->bConvEncoderReg);
    pbInput = info->bFrameScrambled;

    for (i = 0; i < uiSymbolCountData; i++)
    {
        if (!(i & 0x1))
        {
            pcOutput += Generate9MSymbol1(info, pcOutput, info->cWindow, 
                    pbInput, (*PILOTSGN(uiPilotIndex)));
            uiPilotIndex++;
            if (uiPilotIndex == 127)
                uiPilotIndex = 0;
            pbInput += (DBPS / BITS_PER_BYTE);
        }
        else
        {
            pcOutput += Generate9MSymbol2(info, pcOutput, info->cWindow,
                    pbInput, (*PILOTSGN(uiPilotIndex)));
            uiPilotIndex++;
            if (uiPilotIndex == 127)
                uiPilotIndex = 0;
            pbInput += (DBPS / BITS_PER_BYTE) + 1;
        }
    }
    
    UpsampleTailAndCopyNT(info, pcOutput);

    return S_OK;
}
