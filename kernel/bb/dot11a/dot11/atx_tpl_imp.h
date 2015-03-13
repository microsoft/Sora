#include "bb/bba.h"
#include "atx.h"
#include "atx_tpl.h"

HRESULT Dot11aTxFrameEncodeX(PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket)
{
    unsigned int uiScrambledSize;
    unsigned int uiSymbolCountData;
    unsigned int uiPilotIndex = 0;
    unsigned int i;
    char * pbInput;
    PTXSAMPLE pcOutput;
    ULONG PacketSizePlusCRC32 = pPacket->PacketSize + 4;

    // ClearWindow(info->cWindow);

    uiScrambledSize = SERVICE_LEN_IN_BITS + PADDING_LEN_IN_BITS
        + (PacketSizePlusCRC32 << BITS_PER_BYTE_SHIFT);
    uiSymbolCountData = (uiScrambledSize + (DBPS - 1)) / DBPS;

    ULONG SignalBytes = GetSignalBytes(info, uiSymbolCountData);
    
    ULONG SampleBufferSize = 0;
    SoraPacketGetTxSampleBuffer(pPacket, (PTXSAMPLE *)&pcOutput, &SampleBufferSize);
    ASSERT(SampleBufferSize >= SignalBytes);

    ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(pcOutput, SignalBytes);
    SoraPacketSetSignalLength (pPacket, SignalBytes);	
	
    // Copy preamble
    pcOutput += CopyPreamble16_NT(info, pcOutput, info->cWindow);

    ConvEncodeReset(info->bConvEncoderReg);
    // Generate Signal
    pcOutput += GenerateSignal(info, pcOutput, info->cWindow, SIGNAL_RATE,
            (unsigned short)PacketSizePlusCRC32);
    
    // Scramble Data
    Scramble11a(pPacket->pMdl, info->bFrameScrambled,
            uiSymbolCountData * (DBPS / BITS_PER_BYTE), pPacket->Reserved1, info->ulRadom);

    ConvEncodeReset(info->bConvEncoderReg);
    pbInput = info->bFrameScrambled;
    
    for (i = 0; i < uiSymbolCountData; i++)
    {
        pcOutput += GenerateSymbol(info, pcOutput, info->cWindow, 
                pbInput, (*PILOTSGN(uiPilotIndex)));
        uiPilotIndex++;
        if (uiPilotIndex == 127)
            uiPilotIndex = 0;
        pbInput += (DBPS / BITS_PER_BYTE);
    }
    
    UpsampleTailAndCopyNT(info, pcOutput);

    return S_OK;
}
