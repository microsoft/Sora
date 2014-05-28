/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: sdr_macc_recv_queue.h

Abstracts:
    this file defines structures  of ULCB (Receive Control Block), RECV_QUEUE_MANAGER. 
    Provides macros and interfaces definitions for the management of ULCB and receive queue

History:
    Created by senxiang 2009/10/11

Notes: 

--*/

#ifndef    _RECV_QUEUE_H
#define    _RECV_QUEUE_H
#include "sora.h"
#include "dlink_list_queue.h"
#pragma once

enum
{
    RCB_MAX_COUNT           = 128,
    RCB_MIN_COUNT           = 2,
    RCB_DATA_BUF_SIZE       = 12 * 1024,
    RECV_BUFFER_IDEAL_SIZE  = RCB_MAX_COUNT * RCB_DATA_BUF_SIZE
};

/* 802.11 specific Up Link Control Block for receive*/
typedef struct _ULCB{
    LIST_ENTRY          List;
    
    ULONG               PacketLength;
    ULONG               CRC32;  //802.11 MAC frame CRC 
    PUCHAR              pVirtualAddress; //decoded MAC frame address;
    //MAC frame descriptor for easy frame format tranformation;
    PMDL                pMdl; 

    ULONG               BufSize;

}ULCB, *PULCB, **PPULCB;

//CCASSERT(sizeof(PULCB)

typedef struct _RECV_QUEUE_MANAGER{

    PUCHAR          pVirtualStartAddress; // Buffer for all ULCB decoded MAC frame buffers
    ULONG           Size;
    KEVENT          hRecvEvent;

    NDIS_SPIN_LOCK  QueueLock;
    LIST_ENTRY      RecvFreeList;
    LIST_ENTRY      RecvWaitList;

    ULONG           RCBCount;
    ULCB             RecvCtrlBlocks[RCB_MAX_COUNT];

    ULONG           nFreeRCB; //for statistics
    ULONG           nPendingRXPackets; ////for statistics

}RECV_QUEUE_MANAGER, *PRECV_QUEUE_MANAGER;

#define  RADIO_RECEIVE   0

#define GET_RECEIVE_QUEUE_MANAGER(pMac)\
    ((PRECV_QUEUE_MANAGER)(&pMac->RecvQueueManager))

HRESULT
InitializeRecvQueueManager(IN PRECV_QUEUE_MANAGER pRecvQueueManager);

VOID
CleanupRecvQueueManager(IN PRECV_QUEUE_MANAGER pRecvQueueManager);

__forceinline
PULCB GetFreeRecvSlot(IN PRECV_QUEUE_MANAGER pRecvQueueManager)
{
    PULCB pRCB;
    SafeDequeue(pRecvQueueManager, RecvFreeList, pRCB, ULCB);
    return pRCB;
}

#endif