/*++
Copyright (c) Microsoft Corporation

Module Name: TX send packet base 

Abstract: This header file defines base structure of packet received 
          by PHY layer.

History: 
                - Created by Sen Xiang
    16/May/2011 - reviewed and modified by Kun Tan

--*/
#ifndef _PACKET_BASE_H
#define _PACKET_BASE_H
#pragma once 

typedef struct __SORA_RADIO *PSORA_RADIO ;

#include "_radio_manager.h"

enum 
{
    PACKET_NOT_MOD = 0,
    PACKET_TF_FAIL,
    PACKET_CAN_TX,
    PACKET_TX_PEND,
    PACKET_TX_DONE
};

#define IS_PACKET_TX_READY(pPacket) \
    ((pPacket)->fCanTx == PACKET_CAN_TX)

#define IS_PACKET_OWN_RES(pPacket) \
    ((pPacket)->pTxDesc != NULL)

#define IS_PACKET_NO_RES(pPacket) \
    ((pPacket)->pTxDesc == NULL)

//Changed by yichen 15/05/2009 Add PacketSize
typedef struct __PACKET_BASE
{
    PMDL         pMdl;    // point to the MDL for the packet content
    PTX_DESC     pTxDesc; // point to the Tx Descriptor for modulated samples
    
    LONG         fStatus;
    ULONG        PacketSize;
    ULONG        Reserved1; //for customized attachment
    ULONG        Reserved2; //for customized attachment
    ULONG        Reserved3; //for customized attachment
    ULONG        Reserved4; //for customized attachment
    PVOID        pReserved;
} PACKET_BASE, *PPACKET_BASE, **PPPACKET_BASE;

/*++
SoraPacketGetStatus return packet status.

Return: 
    PACKET_CAN_TX - if the packet samples are transferred to 
                    RCB memory using SORA_HW_TX_TRANSFER;

    PACKET_TX_PEND - if the packet samples are successfully Tx out.

    PACKET_TX_DONE - if upper driver calls SoraPacketSetTXDone 
--*/

__inline LONG 
SORAAPI 
SoraPacketGetStatus(IN PPACKET_BASE pPacket)
{
    return pPacket->fStatus;
}

VOID 
SORAAPI 
SoraPacketFreeTxResource(
    IN HANDLE TransferObj,
    IN PPACKET_BASE pPacket);

HRESULT 
SORAAPI 
SoraPacketGetTxResource(
    IN HANDLE TransferObj, 
    IN OUT PPACKET_BASE pPacket);

VOID  
SORAAPI
SoraPacketSetSignalLength(
    IN OUT PPACKET_BASE pPacket, 
    IN ULONG            uLen);

ULONG
SORAAPI
SoraPacketGetSignalSize(
    IN OUT PPACKET_BASE pPacket);

VOID 
SORAAPI 
SoraPacketSetTimeStamp(
    IN OUT PPACKET_BASE pPacket,
    ULONG               TimeStamp);

VOID
SORAAPI
SoraPacketGetTxSampleBuffer(
    IN PPACKET_BASE pPacket, 
    OUT PTXSAMPLE   *ppBuffer,
    OUT PULONG      pBufferSize);

__inline void SORAAPI SoraPacketSetTXDone(IN OUT PPACKET_BASE pPacket)
{
    InterlockedExchange(&pPacket->fStatus, PACKET_TX_DONE);
}

// VOID SORAAPI SoraPacketCtor(OUT PPACKET_BASE pPacket);
VOID 
SORAAPI 
SoraPacketInitialize (OUT PPACKET_BASE pPacket);

// VOID SORAAPI SoraPacketDtor(OUT PPACKET_BASE pPacket);
VOID 
SORAAPI 
SoraPacketCleanup (OUT PPACKET_BASE pPacket);

VOID SORAAPI SoraPacketAssert(IN PPACKET_BASE pPacket, IN HANDLE TransferObj);

VOID SORAAPI SoraPacketPrint(IN PPACKET_BASE pPacket);

__inline void SoraPacketInitializeByBuffer(PPACKET_BASE packet, PMDL pMdl, PUCHAR inputBuf, ULONG inputBufSize, ULONG crc, PVOID outputBuf, ULONG outputBufSize)
{
	pMdl->Next					= NULL;
	pMdl->StartVa				= (PULONG)inputBuf;
	pMdl->ByteOffset			= 0;
	pMdl->MappedSystemVa		= (PULONG)inputBuf;
	pMdl->ByteCount				= inputBufSize;

	packet->pMdl				= pMdl;
	packet->PacketSize			= inputBufSize;
	packet->pReserved			= outputBuf;
	packet->Reserved2			= outputBufSize;
	packet->Reserved1			= crc;
}
#endif 

