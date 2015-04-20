/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_tx.h

Abstract:   Modulation and encoding function declaration for 802.11b Physical 
            Media Dependent (PMD) sub-layer. 

History: 
          7/7/2009: Modified by senxiang.
--*/

#pragma once

#include "bb/bbb.h"

SORA_EXTERN_C
void FIRInit();

SORA_EXTERN_C
HRESULT Scramble(
    IN  PUCHAR      pData, 
    IN  UINT        uiLength, 
    IN  UCHAR       bInitialRegister, 
    OUT PUCHAR      pbNewRegister
    );

SORA_EXTERN_C
HRESULT ScrambleMDLChain(
    IN  PMDL        pMdl, 
    IN  UCHAR       bInitialRegister, 
    IN  PUCHAR      pbNewRegister
    );

#define DBPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR 88
SORA_EXTERN_C
HRESULT DBPSKEncodeBytesToBarkerSpreadedComplex(
    IN  PUCHAR      pbInput, 
    IN  UINT        uiInputLength, 
    IN  UCHAR       bRef, 
    OUT PCOMPLEX8   pbOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

SORA_EXTERN_C
HRESULT DBPSKEncodeBytesToBarkerSpreadedComplex4(
    IN  PUCHAR      pbInput, 
    IN  UINT        uiInputLength, 
    IN  UCHAR       bRef, 
    OUT PCOMPLEX8   pbOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

SORA_EXTERN_C
HRESULT DBPSKEncodeMDLChainToBarkerSpreadedComplex4(
    IN  PMDL        pMdl,
    IN  UCHAR       bRef, 
    OUT PCOMPLEX8   pOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

#define DQPSK_OUTPUT_COMPLEX_COUNT_PER_UCHAR 44
SORA_EXTERN_C
HRESULT DQPSKEncodeBytesToBarkerSpreadedComplex(
    IN  PUCHAR      pbInput, 
    IN  UINT        uiInputLength, 
    IN  UCHAR       bRef, 
    OUT PCOMPLEX8   pbOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
);

SORA_EXTERN_C
HRESULT DQPSKEncodeBytesToBarkerSpreadedComplex4(
    IN  PUCHAR      pbInput, 
    IN  UINT        uiInputLength, 
    IN  UCHAR       bRef, 
    OUT PCOMPLEX8   pbOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

SORA_EXTERN_C
HRESULT DQPSKEncodeMDLChainToBarkerSpreadedComplex4(
    IN  PMDL        pMdl,
    IN  UCHAR       bRef, 
    OUT PCOMPLEX8   pOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

#define CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR 16
SORA_EXTERN_C
HRESULT CCK5EncodeBytesToSpreadedComplex(
    IN  PUCHAR      pbInput, 
    IN  UINT        uiInputLength, 
    IN  UCHAR       bRef, 
    OUT PCOMPLEX8   pbOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

SORA_EXTERN_C
HRESULT CCK5EncodeBytesToSpreadedComplex4(
    IN  PUCHAR      pbInput, 
    IN  UINT        uiInputLength, 
    IN  UCHAR       bRef, 
    OUT PCOMPLEX8   pbOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

SORA_EXTERN_C
HRESULT CCK5EncodeMDLChainToSpreadedComplex4(
    IN  PMDL        pMdl,
    IN  UCHAR       bRef,
    OUT PCOMPLEX8   pOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

#define CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR 8
SORA_EXTERN_C
HRESULT CCK11EncodeBytesToSpreadedComplex(
    IN  PUCHAR      pbInput, 
    IN  UINT        uiInputLength, 
    IN  UCHAR       bRef, 
    IN  UCHAR       bEven, 
    OUT PCOMPLEX8   pbOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

SORA_EXTERN_C
HRESULT CCK11EncodeBytesToSpreadedComplex4(
    IN  PUCHAR      pbInput, 
    IN  UINT        uiInputLength, 
    IN  UCHAR       bRef, 
    IN  UCHAR       bEven, 
    OUT PCOMPLEX8   pbOutput, 
    IN   UINT       uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

SORA_EXTERN_C
HRESULT CCK11EncodeMDLChainToSpreadedComplex4(
    IN  PMDL        pMdl,
    IN  UCHAR       bRef, 
    IN  PUCHAR      pbEven, 
    OUT PCOMPLEX8   pOutput, 
    IN  UINT        uiOutputLength, 
    OUT PUCHAR      pbNewRef
    );

#define INT32_SIZE 32
#define INT16_SIZE 16
#define INT8_SIZE  8
#define BARKER_SPREAD_SEQUENCE_LENGTH 11

#define UPSAMPLE_FACTOR     4
#define TX_FIR_DEPTH        37

SORA_EXTERN_C
HRESULT PLCP_DOT11B_TX4(
                PDOT11B_PLCP_TXVECTOR pTxVector, 
                PPACKET_BASE pSendSlot, 
                PUCHAR pOutput, 
                PUINT puiOutputLength);

SORA_EXTERN_C
HRESULT BB11BPMDPacketTx4XWithShortHeader(
                PDOT11B_PLCP_TXVECTOR pTxVector, 
                PPACKET_BASE pSendSlot, 
                PUCHAR pOutput, 
                PULONG puiOutputLength);

SORA_EXTERN_C
HRESULT BB11BPMDPacketTx4XWithLongHeader(
                PDOT11B_PLCP_TXVECTOR pTxVector, 
                PPACKET_BASE pSendSlot, 
                PUCHAR pOutput, 
                PULONG puiOutputLength);

USHORT PLCPGetLength(IN PDOT11B_PLCP_TXVECTOR pTxVector, IN OUT PUINT ext);

SORA_EXTERN_C const A16 SHORT SSEFilterTaps[TX_FIR_DEPTH+3][8];
