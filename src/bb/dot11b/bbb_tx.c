/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_tx.c

Abstract:   
    Modulation and encoding function implementation for 802.11b Physical 
    Media Dependent (PMD) sub-layer. 

History: 
          7/7/2009: Modified by senxiang.
--*/
#include "CRC16.h"
#include "bbb_tx.h"

#pragma warning(disable:4127)

/*++
BB11BTxVectorInit initializes 802.11b modulation and encoding parameters vector.
--*/
void BB11BTxVectorInit(
        OUT PDOT11B_PLCP_TXVECTOR   pTxVector, 
        IN  UCHAR                   cDataRate,
        IN  UCHAR                   cModSelect,
        IN  UCHAR                   cPreambleType)
{
    pTxVector->DateRate          = cDataRate;
    pTxVector->ModSelect         = cModSelect;
    pTxVector->PreambleType      = cPreambleType;

    FIRInit();
}

/*++
PLCP Length field represents the number of microseconds that it takes to 
transmit the contents of the PPDU, and the receiver uses this 
information to determine the end of the frame. 
--*/
USHORT PLCPGetLength(IN PDOT11B_PLCP_TXVECTOR pTxVector, UINT packetLength, IN OUT PUINT ext)
{
    UINT ret;
    UINT iSize = packetLength + 4; //plus CRC32

    *ext = 0;
    switch (pTxVector->DateRate)
    {
    case DOT11B_PLCP_DATA_RATE_1M:
        return (USHORT)(iSize << 3);
    case DOT11B_PLCP_DATA_RATE_2M:
        return (USHORT)(iSize << 2);
    case DOT11B_PLCP_DATA_RATE_5P5M:
        if (pTxVector->ModSelect == DOT11B_PLCP_IS_PBCC)
            iSize++;
        return (USHORT)(((iSize << 4) - 1) / 11 + 1);
    case DOT11B_PLCP_DATA_RATE_11M:
        if (pTxVector->ModSelect == DOT11B_PLCP_IS_PBCC)
            iSize++;
        ret = ((iSize << 3) - 1) / 11 + 1;
        if (ret * 11 - (iSize << 3) >= 8)
            (*ext)++;
        return (USHORT)(ret);
    default:
        return 0;
    }
}

/*++
802.11b physical media dependent sub-layer (PMD), encoding and modulation 4X-Up-Sample.
It supports 1M, 2M, 5.5M, 11M data rate, short and long preamble.

Note: Output buffer must be big enough, see also BB11B_MAX_SYMBOL_LENGTH. 
--*/
HRESULT BB11BPMDPacketTx4X(
            IN PDOT11B_PLCP_TXVECTOR    pTxVector, 
            IN PPACKET_BASE             pSendSlot, 
            OUT PUCHAR                  pOutput, 
            IN ULONG                    BufferLength, 
            OUT PULONG                  puiOutputLength)
{
    HRESULT hRes = S_OK;
    
    do
    {
        UNREFERENCED_PARAMETER(BufferLength);

        if (pTxVector->PreambleType == DOT11B_PLCP_IS_SHORT_PREAMBLE)
        {
            hRes = BB11BPMDPacketTx4XWithShortHeader(pTxVector, pSendSlot, pOutput, puiOutputLength);
        }
        else if (pTxVector->PreambleType == DOT11B_PLCP_IS_LONG_PREAMBLE)
        {
            hRes = BB11BPMDPacketTx4XWithLongHeader(pTxVector, pSendSlot, pOutput, puiOutputLength);
        }
        else
        {
            hRes = E_FAIL;
        }

    } while (FALSE);

    return hRes;
}

/*++
BB11BPMDPacketGenSignal modulate and encode a packet into raw signals 
as part of physical medium dependent (PMD) sub-layer.

Parameters:
            pFrame: source packet.
            pTxVector: 802.11b PCLP TX parameter vector.
            pTempBuffer: temporary buffer for modulation.
            TempBufferLength: temporary buffer length.

Return: Always return S_OK.

Note: For User Mode and Kernel Mode. If modulation buffer is not big enough, 
        signal gen will fail.
--*/
HRESULT BB11BPMDPacketGenSignal(
            IN OUT PPACKET_BASE                 pPacket, 
            IN PDOT11B_PLCP_TXVECTOR            pTxVector, 
            IN PUCHAR                           pTempBuffer,
            IN ULONG                            TempBufferLength
            )
{
    ULONG TempOutLength = 0;
    ULONG SignalLength  = 0;
    PTXSAMPLE SampleBuffer;
    ULONG SampleBufferSize;
    
    SoraPacketGetTxSampleBuffer(pPacket, &SampleBuffer, &SampleBufferSize); // Get Sample Buffer of TX resource.

    if (TempBufferLength < BB11B_MAX_SYMBOL_LENGTH ||
        SampleBufferSize < BB11B_MAX_SYMBOL_LENGTH)
    {
        return E_FAIL;
    }
    BB11BPMDPacketTx4X(pTxVector, pPacket, pTempBuffer, TempBufferLength, &TempOutLength);
    
    memset(pTempBuffer + TempOutLength * sizeof(COMPLEX8), 0, 64); //tailed 0 for FIR

    BB11BPMDSpreadFIR4SSE(
        (PCOMPLEX8)pTempBuffer, 
        TempOutLength, 
        SampleBuffer, 
        &SignalLength);
    
    SoraPacketSetSignalLength (pPacket, SignalLength * sizeof(TXSAMPLE));
    return S_OK;
}

/*++
Encoding and modulation, 4X-Up Sample for packet with short preamble and PLCP Header.
--*/
HRESULT BB11BPMDPacketTx4XWithShortHeader(
            IN PDOT11B_PLCP_TXVECTOR    pTxVector, 
            IN PPACKET_BASE             pSendSlot, 
            OUT PUCHAR                  pOutput, 
            PULONG                      pOutputLength)
{
    HRESULT         hRes = S_OK;
    PCOMPLEX8       pOutputComplex = (PCOMPLEX8)pOutput;
    UINT            Ext = 0;
    UCHAR           bRegister = 0;
    UCHAR           bRef = 0;
    UCHAR           bEven = DOT11B_PLCP_EVEN_SYMBOL;
    DOT11B_PLCP_SHORT_FRAME PLCPFrame = {0};

    UINT            uiComplexLength = 0;
    UINT            uiPadding = TX_FIR_DEPTH;

    do
    {
        memset(PLCPFrame.Preamble.Sync, DOT11B_PLCP_SHORT_PREAMBLE_SYNC_VALUE, DOT11B_PLCP_SHORT_PREAMBLE_SYNC_LENGTH);
        PLCPFrame.Preamble.SFD = DOT11B_PLCP_SHORT_PREAMBLE_SFD;

        PLCPFrame.Header.Signal = pTxVector->DateRate;
        PLCPFrame.Header.Length = PLCPGetLength(pTxVector, pSendSlot->PacketSize, &Ext);
        PLCPFrame.Header.Service.Bits.LengthExt = (UCHAR) Ext;
        PLCPFrame.Header.CRC = CalcCRC16((PUCHAR)&PLCPFrame.Header.Signal, 4);

        // Scramble encoding for PLCP short frame, MAC SDU, and CRC32
        Scramble((PUCHAR)&PLCPFrame, DOT11B_PLCP_SHORT_LENGTH, DOT11B_PLCP_SHORT_TX_SCRAMBLER_REGISTER, &bRegister);
        ScrambleMDLChain(pSendSlot->pMdl, bRegister, &bRegister);
        Scramble((PUCHAR)&pSendSlot->Reserved1, sizeof(ULONG), bRegister, &bRegister);

        // Encode PLCP short preamble using DBPSK, Data Rate 1Mbps
        DBPSKEncodeBytesToBarkerSpreadedComplex4(
            (PUCHAR)&PLCPFrame, 
            DOT11B_PLCP_SHORT_PREAMBLE_LENGTH,
            bRef,
            pOutputComplex,
            DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_SHORT_PREAMBLE_LENGTH * UPSAMPLE_FACTOR,
            &bRef
            );

        uiComplexLength += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_SHORT_PREAMBLE_LENGTH * UPSAMPLE_FACTOR;
        pOutputComplex  += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_SHORT_PREAMBLE_LENGTH * UPSAMPLE_FACTOR;

        // Encoding PLCP header using DQPSK (differential quadrature phase shift keying)
        // Data Rate 2Mbps
        bRef |= bRef << 1; // Convert from BPSK to QPSK
        DQPSKEncodeBytesToBarkerSpreadedComplex4(
            (PUCHAR)&PLCPFrame.Header,
            DOT11B_PLCP_HEADER_LENGTH,
            bRef,
            pOutputComplex,
            DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_HEADER_LENGTH * UPSAMPLE_FACTOR,
            &bRef
            );

        uiComplexLength += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_HEADER_LENGTH * UPSAMPLE_FACTOR;
        pOutputComplex  += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_HEADER_LENGTH * UPSAMPLE_FACTOR;

        switch (pTxVector->DateRate)
        {
        case DOT11B_PLCP_DATA_RATE_11M:
            CCK11EncodeMDLChainToSpreadedComplex4(
                pSendSlot->pMdl,
                bRef,
                &bEven,
                pOutputComplex,
                CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;

            // CRC32 encoding
            CCK11EncodeBytesToSpreadedComplex4(
                (PUCHAR)&pSendSlot->Reserved1,
                sizeof(ULONG),
                bRef,
                bEven,
                pOutputComplex,
                CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR;
#pragma warning(disable: 6305)
            pOutputComplex  += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR;
#pragma warning(default: 6305)

            break;
        case DOT11B_PLCP_DATA_RATE_5P5M:
            CCK5EncodeMDLChainToSpreadedComplex4(
                pSendSlot->pMdl,
                bRef,
                pOutputComplex,
                CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;

            CCK5EncodeBytesToSpreadedComplex4(
                (PUCHAR)&pSendSlot->Reserved1,
                sizeof(ULONG),
                bRef,
                pOutputComplex,
                CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR;
            // Note: Divide by sizeof(UCHAR) to prevent PREFast warning C6305
            pOutputComplex  += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG)/sizeof(UCHAR) * UPSAMPLE_FACTOR;

            break;
        case DOT11B_PLCP_DATA_RATE_2M:
            DQPSKEncodeMDLChainToBarkerSpreadedComplex4(
                pSendSlot->pMdl,
                bRef,
                pOutputComplex,
                DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize
                * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;
            pOutputComplex  += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;

            DQPSKEncodeBytesToBarkerSpreadedComplex4(
                (PUCHAR)&pSendSlot->Reserved1,
                sizeof(ULONG),
                bRef,
                pOutputComplex,
                DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR;
            pOutputComplex  += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG)/sizeof(UCHAR) * UPSAMPLE_FACTOR;

            break;
        }

        if ((uiPadding + uiComplexLength) & 127)
            uiPadding += 128 - ((uiPadding + uiComplexLength) & 127);

        uiComplexLength += uiPadding;
        memset(pOutputComplex, 0, sizeof(COMPLEX8) * uiPadding);

        *pOutputLength = uiComplexLength;
    } while (FALSE);

    return hRes;
}


/*++
Encoding, modulation, 4X-Up-Sample of packet with long preamble and PLCP Header.

Note:
    CRC32 is pSendSlot->Reserved1
--*/
HRESULT BB11BPMDPacketTx4XWithLongHeader(
            PDOT11B_PLCP_TXVECTOR   pTxVector, 
            PPACKET_BASE            pSendSlot, 
            PUCHAR                  pOutput, 
            PULONG                  pOutputLength)
{
    HRESULT     hRes = S_OK;
    PCOMPLEX8   pOutputComplex = (PCOMPLEX8)pOutput;
    UINT        Ext = 0;
    UCHAR       bRegister = 0;
    UCHAR       bRef = 0;
    UCHAR       bEven = DOT11B_PLCP_EVEN_SYMBOL;
    DOT11B_PLCP_LONG_FRAME PLCPFrame = {0};

    UINT uiComplexLength = 0;
    UINT uiPadding = TX_FIR_DEPTH;

    do
    {
        memset(
            PLCPFrame.Preamble.Sync, 
            DOT11B_PLCP_LONG_PREAMBLE_SYNC_VALUE, 
            DOT11B_PLCP_LONG_PREAMBLE_SYNC_LENGTH);
        PLCPFrame.Preamble.SFD = DOT11B_PLCP_LONG_PREAMBLE_SFD;

        PLCPFrame.Header.Signal = pTxVector->DateRate;
        PLCPFrame.Header.Length = PLCPGetLength(pTxVector, pSendSlot->PacketSize, &Ext);
        PLCPFrame.Header.Service.Bits.LengthExt = (UCHAR) Ext;
        PLCPFrame.Header.CRC    = CalcCRC16((PUCHAR)&PLCPFrame.Header.Signal, 4);

        Scramble((PUCHAR)&PLCPFrame, DOT11B_PLCP_LONG_LENGTH, DOT11B_PLCP_LONG_TX_SCRAMBLER_REGISTER, &bRegister);
        ScrambleMDLChain(pSendSlot->pMdl, bRegister, &bRegister);
        Scramble((PUCHAR)&pSendSlot->Reserved1, sizeof(ULONG), bRegister, &bRegister);

        // Encode PLCP long preamble and header using DBPSK (differential binary phase shift keying), 
        //1Mbps Data Rate
        DBPSKEncodeBytesToBarkerSpreadedComplex4(
            (PUCHAR)&PLCPFrame, 
            DOT11B_PLCP_LONG_LENGTH,
            bRef,
            pOutputComplex,
            DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_LONG_LENGTH * UPSAMPLE_FACTOR,
            &bRef
            );

        pOutputComplex  += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_LONG_LENGTH * UPSAMPLE_FACTOR;
        uiComplexLength += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_LONG_LENGTH * UPSAMPLE_FACTOR;

        switch (pTxVector->DateRate)
        {
        case DOT11B_PLCP_DATA_RATE_11M:
            bRef |= bRef << 1; // Convert from BPSK to QPSK

            CCK11EncodeMDLChainToSpreadedComplex4(
                pSendSlot->pMdl,
                bRef,
                &bEven,
                pOutputComplex,
                CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;

            CCK11EncodeBytesToSpreadedComplex4(
                (PUCHAR)&pSendSlot->Reserved1,
                sizeof(ULONG),
                bRef,
                bEven,
                pOutputComplex,
                CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG)/sizeof(UCHAR) * UPSAMPLE_FACTOR;

            break;
        case DOT11B_PLCP_DATA_RATE_5P5M:
            bRef |= bRef << 1; // Convert from BPSK to QPSK

            CCK5EncodeMDLChainToSpreadedComplex4(
                pSendSlot->pMdl,
                bRef,
                pOutputComplex,
                CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;

            CCK5EncodeBytesToSpreadedComplex4(
                (PUCHAR)&pSendSlot->Reserved1,
                sizeof(ULONG),
                bRef,
                pOutputComplex,
                CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG)/sizeof(UCHAR) * UPSAMPLE_FACTOR;

            break;
        case DOT11B_PLCP_DATA_RATE_2M:
            bRef |= bRef << 1; // Convert from BPSK to QPSK

            DQPSKEncodeMDLChainToBarkerSpreadedComplex4(
                pSendSlot->pMdl,
                bRef,
                pOutputComplex,
                DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;
            pOutputComplex  += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;

            DQPSKEncodeBytesToBarkerSpreadedComplex4(
                (PUCHAR)&pSendSlot->Reserved1,
                sizeof(ULONG),
                bRef,
                pOutputComplex,
                DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR;
            pOutputComplex  += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG)/sizeof(UCHAR) * UPSAMPLE_FACTOR;

            break;
        case DOT11B_PLCP_DATA_RATE_1M:
            DBPSKEncodeMDLChainToBarkerSpreadedComplex4(
                pSendSlot->pMdl,
                bRef,
                pOutputComplex,
                DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;
            pOutputComplex  += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * pSendSlot->PacketSize * UPSAMPLE_FACTOR;

            DBPSKEncodeBytesToBarkerSpreadedComplex4(
                (PUCHAR)&pSendSlot->Reserved1,
                sizeof(ULONG),
                bRef,
                pOutputComplex,
                DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR,
                &bRef
                );

            uiComplexLength += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG) * UPSAMPLE_FACTOR;
            pOutputComplex  += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * sizeof(ULONG)/sizeof(UCHAR) * UPSAMPLE_FACTOR;

            break;
        }

        if ((uiPadding + uiComplexLength) & 127)
        {
            uiPadding += 128 - ((uiPadding + uiComplexLength) & 127);
        }

        uiComplexLength += uiPadding;
        memset(pOutputComplex, 0, sizeof(COMPLEX8) * uiPadding);

        *pOutputLength = uiComplexLength;
    } while (FALSE);

    return hRes;
}

/*++
BB11BPMDBufferTx4XWithShortHeader encodes and modulate raw bits to symbols without FIR, 
the source bytes are located in a continuous buffer, including CRC32 value.

Parameters:
    pTxVector: modulate parameter vector.
    pbData: pointer to SDU buffer. 
    pOutput: pointer to symbol buffer.
    pOutputLength: pointer to symbol length.
Note:
    Before:                                                         |MAC Header|IP Header|TCP/UDP Header|FCS|
    After: symblols for |DSSS PLCP Short Preamble | DSSS PLCP Header|MAC Header|IP Header|TCP/UDP Header|FCS|

For User-mode and kernel-mode.

--*/
HRESULT 
BB11BPMDBufferTx4XWithShortHeader(
    IN PDOT11B_PLCP_TXVECTOR    pTxVector,
    IN PUCHAR                   pbData,
    IN UINT                     dataLength,
    IN PUCHAR                   pOutput, 
    IN PUINT                    pOutputLength
    )
{
    HRESULT                     hRes                = S_OK;
    PCOMPLEX8                   pOutputComplex      = (PCOMPLEX8)pOutput;
    UINT                        Ext                 = 0;
    UCHAR                       bRegister           = 0;
    UCHAR                       bRef                = 0;
    UINT                        uiComplexLength     = 0;
    UINT                        uiPadding           = TX_FIR_DEPTH;
    UINT                        uiPSDULength        = dataLength + 4;    // The PSDU length includes CRC32
    DOT11B_PLCP_SHORT_FRAME     PLCPFrame           = {0};

    do
    {
        memset(PLCPFrame.Preamble.Sync, DOT11B_PLCP_SHORT_PREAMBLE_SYNC_VALUE, DOT11B_PLCP_SHORT_PREAMBLE_SYNC_LENGTH);
        PLCPFrame.Preamble.SFD  = DOT11B_PLCP_SHORT_PREAMBLE_SFD;

        PLCPFrame.Header.Signal = pTxVector->DateRate;        
        PLCPFrame.Header.Length = PLCPGetLength(pTxVector, dataLength, &Ext);
        PLCPFrame.Header.Service.Bits.LengthExt = (UCHAR) Ext;
        PLCPFrame.Header.CRC    = CalcCRC16((PUCHAR)&PLCPFrame.Header.Signal, 4);

        Scramble((PUCHAR)&PLCPFrame, DOT11B_PLCP_SHORT_LENGTH, DOT11B_PLCP_SHORT_TX_SCRAMBLER_REGISTER, &bRegister);
        Scramble(pbData, uiPSDULength, bRegister, &bRegister);

        DBPSKEncodeBytesToBarkerSpreadedComplex4(
            (PUCHAR)&PLCPFrame, 
            DOT11B_PLCP_SHORT_PREAMBLE_LENGTH,
            bRef,
            pOutputComplex,
            DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_SHORT_PREAMBLE_LENGTH * UPSAMPLE_FACTOR,
            &bRef
            );
        uiComplexLength += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_SHORT_PREAMBLE_LENGTH * UPSAMPLE_FACTOR;
        pOutputComplex  += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_SHORT_PREAMBLE_LENGTH * UPSAMPLE_FACTOR;

        bRef |= bRef << 1; // Convert from BPSK to QPSK
        DQPSKEncodeBytesToBarkerSpreadedComplex4(
            (PUCHAR)&PLCPFrame.Header,
            DOT11B_PLCP_HEADER_LENGTH,
            bRef,
            pOutputComplex,
            DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_HEADER_LENGTH * UPSAMPLE_FACTOR,
            &bRef
            );
        uiComplexLength += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_HEADER_LENGTH * UPSAMPLE_FACTOR;
        pOutputComplex  += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_HEADER_LENGTH * UPSAMPLE_FACTOR;

        switch (pTxVector->DateRate)
        {
        case DOT11B_PLCP_DATA_RATE_11M:
            CCK11EncodeBytesToSpreadedComplex4(
                pbData,
                uiPSDULength,
                bRef,
                DOT11B_PLCP_EVEN_SYMBOL,
                pOutputComplex,
                CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;

            break;
        case DOT11B_PLCP_DATA_RATE_5P5M:
            CCK5EncodeBytesToSpreadedComplex4(
                pbData,
                uiPSDULength,
                bRef,
                pOutputComplex,
                CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;

            break;
        case DOT11B_PLCP_DATA_RATE_2M:
            DQPSKEncodeBytesToBarkerSpreadedComplex4(
                pbData,
                uiPSDULength,
                bRef,
                pOutputComplex,
                DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;
            pOutputComplex  += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;

            break;
        }

        if ((uiPadding + uiComplexLength) & 127)
        {
            uiPadding += 128 - ((uiPadding + uiComplexLength) & 127);
        }

        uiComplexLength += uiPadding;
        memset(pOutputComplex, 0, sizeof(COMPLEX8) * uiPadding);

        *pOutputLength = uiComplexLength;
    } while (FALSE);

    return hRes;
}


/*++
BB11BPMDBufferTx4XWithLongHeader encodes and modulate raw bits to symbols without FIR, 
the source bytes are located in a continuous buffer, including CRC32 value.

Parameters:
    pTxVector: modulate parameter vector.
    pbData: pointer to SDU buffer (with CRC32). 
    pOutput: pointer to symbol buffer.
    pOutputLength: pointer to symbol length.
Note:
    Before:                                                         |MAC Header|IP Header|TCP/UDP Header|FCS|
    After: symblols for |DSSS PLCP Long Preamble | DSSS PLCP Header |MAC Header|IP Header|TCP/UDP Header|FCS|

For User-mode and kernel-mode.

--*/
HRESULT 
BB11BPMDBufferTx4XWithLongHeader(
    IN PDOT11B_PLCP_TXVECTOR    pTxVector,
    IN PUCHAR                   pbData,
    IN UINT                     dataLength,
    IN PUCHAR                   pOutput,
    IN PUINT                    pOutputLength
    )
{
    HRESULT                     hRes                            = S_OK;
    PCOMPLEX8                   pOutputComplex                  = (PCOMPLEX8)pOutput;
    UINT                        Ext                             = 0;
    UCHAR                       bRegister                       = 0;
    UCHAR                       bRef                            = 0;
    
    UINT                        uiComplexLength                 = 0;
    UINT                        uiPadding                       = TX_FIR_DEPTH;
    UINT                        uiPSDULength                    = dataLength + 4;    // The PSDU length includes CRC32
    DOT11B_PLCP_LONG_FRAME      PLCPFrame                       = {0};

    do
    {

        memset(PLCPFrame.Preamble.Sync, DOT11B_PLCP_LONG_PREAMBLE_SYNC_VALUE, DOT11B_PLCP_LONG_PREAMBLE_SYNC_LENGTH);
        PLCPFrame.Preamble.SFD  = DOT11B_PLCP_LONG_PREAMBLE_SFD;

        PLCPFrame.Header.Signal = pTxVector->DateRate;
        PLCPFrame.Header.Length = PLCPGetLength(pTxVector, dataLength, &Ext);
        PLCPFrame.Header.Service.Bits.LengthExt = (UCHAR) Ext;
        PLCPFrame.Header.CRC    = CalcCRC16((PUCHAR)&PLCPFrame.Header.Signal, 4);

        Scramble((PUCHAR)&PLCPFrame, DOT11B_PLCP_LONG_LENGTH, DOT11B_PLCP_LONG_TX_SCRAMBLER_REGISTER, &bRegister);
        Scramble(pbData, uiPSDULength, bRegister, &bRegister);

        DBPSKEncodeBytesToBarkerSpreadedComplex4(
            (PUCHAR)&PLCPFrame, 
            DOT11B_PLCP_LONG_LENGTH,
            bRef,
            pOutputComplex,
            DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_LONG_LENGTH * UPSAMPLE_FACTOR,
            &bRef
            );
        uiComplexLength += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_LONG_LENGTH * UPSAMPLE_FACTOR;
        pOutputComplex  += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * DOT11B_PLCP_LONG_LENGTH * UPSAMPLE_FACTOR;

        switch (pTxVector->DateRate)
        {
        case DOT11B_PLCP_DATA_RATE_11M:
            bRef |= bRef << 1; // Convert from BPSK to QPSK

            CCK11EncodeBytesToSpreadedComplex4(
                pbData,
                uiPSDULength,
                bRef,
                DOT11B_PLCP_EVEN_SYMBOL,
                pOutputComplex,
                CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;

            break;
        case DOT11B_PLCP_DATA_RATE_5P5M:
            bRef |= bRef << 1; // Convert from BPSK to QPSK

            CCK5EncodeBytesToSpreadedComplex4(
                pbData,
                uiPSDULength,
                bRef,
                pOutputComplex,
                CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;
            pOutputComplex  += CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;

            break;
        case DOT11B_PLCP_DATA_RATE_2M:
            bRef |= bRef << 1; // Convert from BPSK to QPSK

            DQPSKEncodeBytesToBarkerSpreadedComplex4(
                pbData,
                uiPSDULength,
                bRef,
                pOutputComplex,
                DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;
            pOutputComplex  += DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;
            
            break;
        case DOT11B_PLCP_DATA_RATE_1M:
            DBPSKEncodeBytesToBarkerSpreadedComplex4(
                pbData,
                uiPSDULength,
                bRef,
                pOutputComplex,
                DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR,
                &bRef
                );
            uiComplexLength += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;
            pOutputComplex  += DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR * uiPSDULength * UPSAMPLE_FACTOR;

            break;
        }

        if ((uiPadding + uiComplexLength) & 127)
        {
            uiPadding += 128 - ((uiPadding + uiComplexLength) & 127);
        }

        uiComplexLength += uiPadding;
        memset(pOutputComplex, 0, sizeof(COMPLEX8) * uiPadding);

        *pOutputLength = uiComplexLength;
    } while (FALSE);

    return hRes;
}
