#include "miniport.h"
#include "sendrcv.h"

#pragma NDIS_PAGEABLE_FUNCTION(InitSendQueueManager)
#pragma NDIS_PAGEABLE_FUNCTION(CleanSendQueueManager)
#pragma NDIS_PAGEABLE_FUNCTION(ReleaseCompleteSendQueue)

/* 
ReleaseCompleteSendQueue finalizes all packets in completed queue.

Parameters: 
    pMac: pointer to Mac structure
           
Return:    

History:

IRQL: PASSIVE_LEVEL
*/
VOID ReleaseCompleteSendQueue(IN PSEND_QUEUE_MANAGER pSendQueueManager)
{
    HRESULT         hRes    = E_FAIL;
    PDLCB           pDLCB   = NULL;

    PAGED_CODE();
    while(TRUE)
    {
        SafeDequeue(pSendQueueManager, SendCompleteList, pDLCB, DLCB);
        if (!pDLCB) 
        {
            break;
        }
        
        SdrLLSendPacketComplete(&pSendQueueManager->pAdapter->SdrContext, pDLCB);

        InterlockedDecrement(&pSendQueueManager->nCompletePacket);
    }
}

/* 
ReleaseSymbolQueue finalizes all packets in modulated queue. 

Parameters: 
    pMac: pointer to Mac structure
           
Return:    

History: 

IRQL: PASSIVE_LEVEL
*/
VOID ReleaseSymbolQueue(IN PSEND_QUEUE_MANAGER pSendQueueManager)
{
    PDLCB           pDLCB       = NULL;
    PMP_ADAPTER     pAdapter    = pSendQueueManager->pAdapter;
    PSORA_RADIO     pRadio      = RadioInPHY(&pAdapter->Phy, RADIO_SEND); 

    do 
    {
        SafeDequeue(pSendQueueManager, SendSymWaitList, pDLCB, DLCB);
        if (!pDLCB)
        {
            break;
        }
       	SoraPacketFreeTxResource(pAdapter->TransferObj, &pDLCB->PacketBase); 

        SdrLLSendPacketComplete(&pAdapter->SdrContext, pDLCB);
        
        InterlockedDecrement(&pSendQueueManager->nSymPacket);
    } while(TRUE);
    
}

/* 
ReleaseSrcPacketQueue finalizes all packets in source queue.

Parameters: 
    pSendQueueManager: pointer to send queue
           
Return:    

History:   12/5/2009 Created by yichen

IRQL: PASSIVE_LEVEL
*/
VOID ReleaseSrcPacketQueue(IN PSEND_QUEUE_MANAGER pSendQueueManager)
{
    PDLCB       pDLCB       = NULL;
    PMP_ADAPTER pAdapter    = pSendQueueManager->pAdapter;
    PSORA_RADIO pRadio      = RadioInPHY(&pAdapter->Phy, RADIO_SEND); 

    do
    {
        SafeDequeue(pSendQueueManager, SendSrcWaitList, pDLCB, DLCB);
        if (!pDLCB)
        {
            break;
        }
        SdrLLSendPacketComplete(&pAdapter->SdrContext, pDLCB);
        InterlockedDecrement(&pSendQueueManager->nSrcPacket);
    } while(TRUE);
    
}

/*
InitSendQueueManager initializes send queue manager.

Parameters:
    pSendQueueManager:  Pointer to send queue manager
    pAdapter: Pointer to the adapter that manager belongs to

Return:
    NDIS_STATUS_SUCCESS if succeeded, otherwise error code.

IRQL: == PASSIVE_LEVEL
    
*/
VOID
CleanQueuedDpc(struct _KDPC  *Dpc,
	PVOID  DeferredContext,
	PVOID  SystemArgument1,
	PVOID  SystemArgument2) {

	PSEND_QUEUE_MANAGER pSendQueueManager;
	pSendQueueManager = (PSEND_QUEUE_MANAGER)DeferredContext;

	// able to cancel the timer, something are newly queued or TX, give up the clean queue procedure
	if (KeCancelTimer(&pSendQueueManager->CleanQTimer)) { 
		LARGE_INTEGER timeout;
		timeout.QuadPart = (LONGLONG)-10*1000*send_timeout;
		KeSetTimer(&pSendQueueManager->CleanQTimer,
			timeout,
			&pSendQueueManager->CleanQDPC);
	}
	else {
		PDLCB pTCB;
		while(1) {
			SafeDequeue(pSendQueueManager, SendSymWaitList, pTCB, DLCB);
			if (pTCB) {
				SoraPacketFreeTxResource(pSendQueueManager->pAdapter->TransferObj, &pTCB->PacketBase); 
				SoraPacketSetTXDone(&pTCB->PacketBase);
				InterlockedIncrement(&pSendQueueManager->nCompletePacket);
				InterlockedDecrement(&pSendQueueManager->nSymPacket);
				SafeEnqueue(pSendQueueManager, SendCompleteList, pTCB);
				continue;
			}
			break;
		}
		SDR_MAC_INDICATE_PACKET_SENT_COMPLETE(&pSendQueueManager->pAdapter->Mac);
	}
}

NDIS_STATUS 
InitSendQueueManager (
    IN PSEND_QUEUE_MANAGER pSendQueueManager, 
    IN PMP_ADAPTER pAdapter)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PUCHAR pTCBMem;
    LONG index;
    PDLCB  pTCB;

    PAGED_CODE();
    
    pSendQueueManager->pAdapter     = pAdapter;
    pSendQueueManager->TCBMem       = NULL;
    
    NdisInitializeListHead(&pSendQueueManager->SendFreeList);
    NdisInitializeListHead(&pSendQueueManager->SendSrcWaitList);
    NdisInitializeListHead(&pSendQueueManager->SendSymWaitList);
    NdisInitializeListHead(&pSendQueueManager->SendCompleteList);
    NdisAllocateSpinLock(&pSendQueueManager->QueueLock);
    KeInitializeEvent(&(pSendQueueManager->hSendEvent),NotificationEvent,FALSE);

    pSendQueueManager->nFreeTCB         = TCB_MAX_COUNT;
    pSendQueueManager->nSrcPacket       = 0;
    pSendQueueManager->nSymPacket       = 0;
    pSendQueueManager->nCompletePacket  = 0;
    do 
    {
        MP_ALLOCATE_MEMORY(pTCBMem, sizeof(DLCB) * TCB_MAX_COUNT, Status);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        NdisZeroMemory(pTCBMem, sizeof(DLCB) * TCB_MAX_COUNT);
        pSendQueueManager->TCBMem = pTCBMem;

        //Divide the TCBMem blob into TCBs
        for (index = 0; index < TCB_MAX_COUNT; index++)
        {
            pTCB = (PDLCB) pTCBMem;
            NdisInterlockedInsertTailList(
                &pSendQueueManager->SendFreeList, 
                &pTCB->List, 
                &pSendQueueManager->QueueLock);
            pTCBMem = pTCBMem + sizeof(DLCB);
        }
    } while (FALSE);
	
	KeInitializeDpc(&pSendQueueManager->CleanQDPC, 
		CleanQueuedDpc,
		pSendQueueManager);
	
	KeInitializeTimer(&pSendQueueManager->CleanQTimer);
	
    return Status;
}

/*++
CleanQueueWithTxResources finalizes packets in modulated queue.
--*/
VOID
CleanQueueWithTxResources(
    IN PSEND_QUEUE_MANAGER pSendQueueManager
    )
{
    ReleaseSymbolQueue(pSendQueueManager);
    ASSERT(IsListEmpty(&pSendQueueManager->SendSymWaitList));
   
    return;
}

/*
CleanSendQueueManager cleanup send queue manager object

Parameters:
    pSendQueueManager: object to be cleaned up

Return:

IRQL: 
    PASSIVE_LEVEL

Notes:
    Send Queue Manager can never be called unless all packets are completed.
*/
VOID
CleanSendQueueManager(
    IN PSEND_QUEUE_MANAGER pSendQueueManager
    )
{
	KeCancelTimer(&pSendQueueManager->CleanQTimer);

    NdisFreeSpinLock(&pSendQueueManager->SendLock);
    ASSERT(IsListEmpty(&pSendQueueManager->SendSrcWaitList));
    ASSERT(IsListEmpty(&pSendQueueManager->SendSymWaitList));
    ASSERT(IsListEmpty(&pSendQueueManager->SendCompleteList));
    if (pSendQueueManager->TCBMem)
    {
        NdisFreeMemory(
            pSendQueueManager->TCBMem, 
            sizeof(DLCB) * TCB_MAX_COUNT, 
            0); //memory flag must be 0 because it is allocated with 
        pSendQueueManager->TCBMem = NULL;
    }
    
    return;
}



/*
GetNFreeDLCB returns n DLCB to accommodate multiple packets.
*/
BOOL GetNFreeDLCB(
        OUT PLIST_ENTRY pNFreeDLCBHead, 
        IN PSEND_QUEUE_MANAGER pSendQueueMgr, 
        IN ULONG n)
{
    ULONG i;
    PLIST_ENTRY pFreeEntry;
    BOOL succ = TRUE;
    InitializeListHead(pNFreeDLCBHead);
    for (i = 0; i < n; i++)
    {
        pFreeEntry = NdisInterlockedRemoveHeadList(
            &pSendQueueMgr->SendFreeList, 
            &pSendQueueMgr->QueueLock);
        if (pFreeEntry)
        {
            InterlockedDecrement(&pSendQueueMgr->nFreeTCB);
            InsertTailList(pNFreeDLCBHead, pFreeEntry);
        }
        else
        {
            succ = FALSE;
            while(!IsListEmpty(pNFreeDLCBHead))
            {
                pFreeEntry = RemoveHeadList(pNFreeDLCBHead);
                NdisInterlockedInsertTailList(
                    &pSendQueueMgr->SendFreeList, 
                    pFreeEntry, 
                    &pSendQueueMgr->QueueLock);
                InterlockedIncrement(&pSendQueueMgr->nFreeTCB);
            }
            break;
        }
    }

    return succ;
}


 
