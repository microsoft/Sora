#include "bb/bba.h"
#include "bb/mod/convenc.h"
#include "atx.h"
#include "atx_tpl.h"
#include "CRC32.h"

#define SERVICE_LEN_IN_BYTES    2

HRESULT Dot11aTxFrameEncode6(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket);
HRESULT Dot11aTxFrameEncode9(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket);
HRESULT Dot11aTxFrameEncode12(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket);
HRESULT Dot11aTxFrameEncode18(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket);
HRESULT Dot11aTxFrameEncode24(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket);
HRESULT Dot11aTxFrameEncode36(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket);
HRESULT Dot11aTxFrameEncode48(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket);
HRESULT Dot11aTxFrameEncode54(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket);

HRESULT BB11ATxFrameMod(IN PBB11A_TX_VECTOR info, IN OUT PPACKET_BASE pPacket)
{
    KIRQL OldIrql;
    HRESULT hr;

	if (pPacket->PacketSize + 4 > INPUT_MAX)
    {
        KdPrint(("TX_FAIL: input size too large, given %d, "
                    "expect less than or equal to %d.\n", pPacket->PacketSize, INPUT_MAX - 4));
        return E_FAIL;
    }

    info->ulRadom = BBRAND(&info->ulRandSeed);
	KeAcquireSpinLock(&info->txLock, &OldIrql);

    switch (info->ti_uiDataRate)
    {
        case DOT11A_RATE_6M:
            hr = Dot11aTxFrameEncode6(info, pPacket);
            break;
        case DOT11A_RATE_9M:
            hr = Dot11aTxFrameEncode9(info, pPacket);
            break;
        case DOT11A_RATE_12M:
            hr = Dot11aTxFrameEncode12(info, pPacket);
            break;
        case DOT11A_RATE_18M:
            hr = Dot11aTxFrameEncode18(info, pPacket);
            break;
        case DOT11A_RATE_24M:
            hr = Dot11aTxFrameEncode24(info, pPacket);
            break;
        case DOT11A_RATE_36M:
            hr = Dot11aTxFrameEncode36(info, pPacket);
            break;
        case DOT11A_RATE_48M:
            hr = Dot11aTxFrameEncode48(info, pPacket);
            break;
        case DOT11A_RATE_54M:
            hr = Dot11aTxFrameEncode54(info, pPacket);
            break;
        default:
            KdPrint(("TX_FAIL: rate %d not support\n", info->ti_uiDataRate));
            hr = E_FAIL;
            break;
    }

	KeReleaseSpinLock(&info->txLock, OldIrql);

    return hr;
}

__forceinline
void Scramble11aBuffer(
        IN PUCHAR pInputData,
		IN unsigned int uiInputDataSize,
		IN OUT char * pbOutput,
        IN unsigned int uiOutTotal,
        ULONG ulRadom)
{
    unsigned char * pbIn;
    unsigned char * pbOut = (unsigned char *)
        (pbOutput + SERVICE_LEN_IN_BYTES);
    unsigned char * pbOutEnd = (unsigned char *)
        (pbOutput + uiOutTotal);
    unsigned int i;
    unsigned char bReg = (unsigned char)ulRadom;

    if ((bReg & 0xFE) == 0)
        bReg = 0xFF;
    bReg = 0xFF;

    pbOutput[0] = (bReg = *(SCRAMBLE_11A(bReg >> 1)));
    pbOutput[1] = (bReg = *(SCRAMBLE_11A(bReg >> 1)));

    pbIn = pInputData;
    for(i = 0; i < uiInputDataSize; i++)
    {
        *(pbOut++) = pbIn[i] ^ (bReg = *(SCRAMBLE_11A(bReg >> 1)));
    }

    *(pbOut++) = (bReg = *(SCRAMBLE_11A(bReg >> 1))) & 0xC0;

    while (pbOut != pbOutEnd)
    {
        *(pbOut++) = (bReg = *(SCRAMBLE_11A(bReg >> 1)));
    }

    pbOutput += SERVICE_LEN_IN_BYTES;
    /*
    for (i = 0; i < uiOutTotal; i++)
    {
        KdPrint(("%02x", (unsigned char)(pbOutput[i])));
    }
    */
}

HRESULT BB11ATxBufferMod6M(
    IN PBB11A_TX_VECTOR info,
	IN PUCHAR           pData,
    IN OUT PUCHAR       pOutput,
    IN OUT PULONG       pOutputLengthInBytes)
{
    unsigned int uiScrambledSize;
    unsigned int uiSymbolCountData;
    unsigned int uiPilotIndex = 0;
    unsigned int i;
    char * pbInput;
    KIRQL OldIrql;
    PCOMPLEX8 pcOutput = (PCOMPLEX8)pOutput;
    ULONG BufferSize = info->ti_uiBufferLength;

    info->ulRadom = BBRAND(&info->ulRandSeed);
	KeAcquireSpinLock(&info->txLock, &OldIrql);

    uiScrambledSize = SERVICE_LEN_IN_BITS + PADDING_LEN_IN_BITS
        + (BufferSize << BITS_PER_BYTE_SHIFT);
    uiSymbolCountData = (uiScrambledSize + (DBPS_6M - 1)) / DBPS_6M;

    *pOutputLengthInBytes = GetSignalBytes(info, uiSymbolCountData);
    ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(pcOutput, *pOutputLengthInBytes);

    // Copy preamble
    pcOutput += CopyPreamble16_NT(info, pcOutput, info->cWindow);

    ConvEncodeReset(info->bConvEncoderReg);
    // Generate Signal
    pcOutput += GenerateSignal(info, pcOutput, info->cWindow, SIGNAL_RATE_6M,
            (unsigned short)BufferSize);

    // Scramble Data
    Scramble11aBuffer(
		pData,
        BufferSize,
		info->bFrameScrambled,
        uiSymbolCountData * (DBPS_6M / BITS_PER_BYTE), info->ulRadom);

    ConvEncodeReset(info->bConvEncoderReg);
    pbInput = info->bFrameScrambled;

    for (i = 0; i < uiSymbolCountData; i++)
    {
        pcOutput += Generate6MSymbol(info, pcOutput, info->cWindow, 
                pbInput, (*PILOTSGN(uiPilotIndex)));
        uiPilotIndex++;
        if (uiPilotIndex == 127)
            uiPilotIndex = 0;
        pbInput += (DBPS_6M / BITS_PER_BYTE);
    }

    UpsampleTailAndCopyNT(info, pcOutput);

    KeReleaseSpinLock(&info->txLock, OldIrql);

	return S_OK;
}

ULONG BB11AModulateACK(unsigned int SampleRate, const PMAC_ADDRESS RecvMacAddress, PVOID PhyACKBuffer)
{
    DOT11_MAC_ACK_FRAME  AckFrame           = {0};
    BB11A_TX_VECTOR      Dot11ATxVector     = {0};
    ULONG OutputLengthInBytes               = 0;
    
    AckFrame.FrameControl.Subtype   = SUBT_ACK;
    AckFrame.FrameControl.Type      = FRAME_CTRL;
    AckFrame.RecvAddress            = *RecvMacAddress;
    AckFrame.Duration               = 0;
    AckFrame.FCS                    = CalcCRC32((PUCHAR)&AckFrame, sizeof(DOT11_MAC_ACK_FRAME) - sizeof(ULONG));

    Dot11ATxVector.ti_uiBufferLength= sizeof(DOT11_MAC_ACK_FRAME);
    Dot11ATxVector.ti_uiDataRate    = DOT11A_RATE_24M;
    Dot11ATxVector.SampleRate       = SampleRate;

	BB11ATxBufferMod6M(
        &Dot11ATxVector,
        (PUCHAR)&AckFrame,
        (PUCHAR)PhyACKBuffer,
		&OutputLengthInBytes);

	return OutputLengthInBytes;
}
