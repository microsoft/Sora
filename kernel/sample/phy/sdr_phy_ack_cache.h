/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_phy_ack_cache.h

Abstract: Interface for 802.11b software physical layer's ACK symbol cache manager. 
          
History: 
          7/Jul/2009: Created by senxiang
--*/

#ifndef _SDR_PHY_ACK_CACHE_H
#define _SDR_PHY_ACK_CACHE_H

#include "Wdm.h"

#define MAX_PHY_ACK_SIZE      16 * 1024 * sizeof(COMPLEX8)
#define MAX_PHY_ACK_NUM       16
#define MAX_ACK_MAKE_REQ_NUM  16

CCASSERT((sizeof(DOT11B_PLCP_LONG_FRAME) + sizeof(DOT11_MAC_ACK_FRAME)) 
         * 8 * 4 * 11 * sizeof(COMPLEX8) < MAX_PHY_ACK_SIZE)

typedef struct _ACK_MAKE_REQ
{
    CACHE_KEY   MacAddress; //Mac Address as ACK key
    LONG        fOccupied;
}ACK_MAKE_REQ, *PACK_MAKE_REQ;

typedef struct _PHY *PPHY;

typedef struct _ACK_CACHE_MAN
{
    SIGNAL_CACHE             AckCache;                     //Signal Cache for ACKs
    PPHY                     pOwnerPhy;                    //parent phy reference this object.
    COMPLEX8                *pAckModulateBuffer;           //ACK Modulate buffer
    ULONG                    AckModulateBufferSize;        //ACK Modulate buffer size
    PHYSICAL_ADDRESS         AckModulateBufferPA;          //ACK Modulate buffer physical address, for Hardware Read
    
    ACK_MAKE_REQ             ReqQueue[MAX_ACK_MAKE_REQ_NUM]; //Ack-Make Async request queue
    LONG                     QueueHeadIndex;                 //queue head index
    LONG                     QueueTailIndex;                 //queue tail index
    NDIS_SPIN_LOCK           ReqQueueLock;                   //queue lock
    
    LONG                     PendingReqNum;                  //queue length, or sys worker thread active flag
    PIO_WORKITEM             pSysThreadWorkItem;             //sys worker thread 's work items.

    NDIS_EVENT               RemoveEvent;
    LONG                     RefCount;
} ACK_CACHE_MAN, *PACK_CACHE_MAN;

HRESULT SdrPhyInitAckCache(
            OUT PACK_CACHE_MAN pAckCacheMan, 
            IN PDEVICE_OBJECT pDeviceObj,
            IN PPHY pOwnerPhy,
            IN PSORA_RADIO pRadio, 
            IN ULONG MaxAckSize, 
            IN ULONG MaxAckNum
            );

VOID SdrPhyCleanupAckCache(
            OUT PACK_CACHE_MAN pAckCacheMan);

HRESULT SdrPhyAsyncMakeAck(
            IN PACK_CACHE_MAN pAckCacheMan, 
            IN PMAC_ADDRESS pMAC_Addr);

#endif