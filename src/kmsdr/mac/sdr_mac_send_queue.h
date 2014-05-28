/*++
Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: 
    sdr_mac_send_queue.h
  
Abstracts:
    This file defines queue management for packet transmission. 

Revision History:
    New double-linked cirular list implementation by senxiang 2009/11/03

Notes: 

--*/
#ifndef    _SEND_QUEUE_
#define    _SEND_QUEUE_
#include "sora.h"
#include "dlink_list_queue.h"
#pragma once

#define   TCB_MAX_COUNT   256
typedef   struct _MP_ADAPTER  *PMP_ADAPTER;

/* 802.11 specific Down Link Control Block for transmission */
typedef struct _DLCB
{
    LIST_ENTRY      List;
    PACKET_BASE     PacketBase;

    PVOID           pNdisPktOrNBL;
    ULONG           PacketType;     // Need a ACK or not?
    ULONG           RetryCount;
    ULONG           Dot11HeaderSize;
    PUCHAR          pDot11HeaderVa;
    BOOLEAN         bSendOK;

    BOOLEAN         bLastNB;//For NDIS 6
} DLCB, *PDLCB, **PPDLCB; 

#ifndef _M_X64
CCASSERT(sizeof(DLCB) == 68)
#endif

#define DLCB_CONTAIN_VALID_PACKET(pDLCB) ((pDLCB)->PacketBase.pMdl != NULL)

typedef struct _SEND_QUEUE_MANAGER{
    KEVENT          hSendEvent;
    PMP_ADAPTER     pAdapter; //for NDIS operation, such as NdisMSendComplete to finalize transmission
    NDIS_SPIN_LOCK  QueueLock;
    LIST_ENTRY      SendFreeList; // free DLCB
    LIST_ENTRY      SendSrcWaitList; // source packet DLCB waiting for encoding and modulating
    LIST_ENTRY      SendSymWaitList; // symbol packet waiting for TX out
    LIST_ENTRY      SendCompleteList; // packet DLCB list need to be finalized
    PUCHAR          TCBMem;// DLCB memory
    
    ULONG           nFreeTCB; // for statistic;
    ULONG           nSrcPacket; // for statistic;
    ULONG           nSymPacket; // for statistic;
    ULONG           nCompletePacket; // for statistic;

	KDPC 			CleanQDPC;
	KTIMER			CleanQTimer;
}SEND_QUEUE_MANAGER, *PSEND_QUEUE_MANAGER;

enum 
{
    PACKET_NEED_ACK           = 1,
    PACKET_NOT_NEED_ACK       = 2
};

#define  GET_SEND_QUEUE_MANAGER(pMac)\
    ((PSEND_QUEUE_MANAGER)(&((pMac)->SendQueueManager)))

VOID ReleaseCompleteSendQueue(IN PSEND_QUEUE_MANAGER pSendQueueManager);

VOID ReleaseSymbolQueue(IN PSEND_QUEUE_MANAGER pSendQueueManager);

VOID ReleaseSrcPacketQueue(IN PSEND_QUEUE_MANAGER pSendQueueManager);

/* New Send Queue Interface */

NDIS_STATUS 
InitSendQueueManager (
    IN PSEND_QUEUE_MANAGER pSendQueueManager, 
    IN PMP_ADAPTER pAdapter);

VOID
CleanSendQueueManager(
    IN PSEND_QUEUE_MANAGER pSendQueueManager
    );

VOID
CleanQueueWithTxResources(
    IN PSEND_QUEUE_MANAGER pSendQueueManager
    );

BOOL GetNFreeDLCB(
        OUT PLIST_ENTRY pNFreeDLCBHead, 
        IN PSEND_QUEUE_MANAGER pSendQueueMgr, 
        IN ULONG n);

#endif