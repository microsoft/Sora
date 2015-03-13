/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_mac_recv_queue.c

Abstract: This file implements RECV queue initialization and cleanup.

History: 
          2009/11/5: double linked list queue implementation by senxiang
--*/

#include "miniport.h"

/* 
InitializeRecvQueueManager is called to initialize receive queue manager, 

Parameters: 
    pRecvQueueManager: pointer to RECV_QUEUE_MANAGER structure
           
Return:    

Note:
    This routine should be called in one thread a time.

History:   
    2009/11/5: double linked list queue by senxiang

IRQL: <= DPC_LEVEL
*/
HRESULT
InitializeRecvQueueManager(IN PRECV_QUEUE_MANAGER pRecvQueueManager)
{
    NDIS_STATUS         Status              = NDIS_STATUS_SUCCESS;
    ULONG               Index;
    HRESULT             hRes                = E_FAIL;
    PHYSICAL_ADDRESS    PhysicalAddress     = {0, 0};
    PHYSICAL_ADDRESS    PhysicalAddressLow  = {0, 0};
    PHYSICAL_ADDRESS    PhysicalAddressHigh = {0x80000000, 0};

    do 
    {
        KeInitializeEvent(&(pRecvQueueManager->hRecvEvent), NotificationEvent, FALSE);
        NdisAllocateSpinLock(&pRecvQueueManager->QueueLock);
        NdisInitializeListHead(&pRecvQueueManager->RecvFreeList);
        NdisInitializeListHead(&pRecvQueueManager->RecvWaitList);

        pRecvQueueManager->Size                 = RECV_BUFFER_IDEAL_SIZE;
        pRecvQueueManager->pVirtualStartAddress = NULL;

        for (Index = RCB_MIN_COUNT; Index <= RCB_MAX_COUNT; Index <<= 2)
        {
            MP_ALLOCATE_MEMORY(pRecvQueueManager->pVirtualStartAddress, pRecvQueueManager->Size, Status);
            if(Status == NDIS_STATUS_SUCCESS)
            {
                hRes = S_OK;
                break;
            }

            pRecvQueueManager->Size >>= 2;
        }

        FAILED_BREAK(hRes);

        pRecvQueueManager->RCBCount = pRecvQueueManager->Size / RCB_DATA_BUF_SIZE;
        pRecvQueueManager->nFreeRCB = pRecvQueueManager->RCBCount;
        pRecvQueueManager->nPendingRXPackets = 0;

        for (Index = 0; Index < pRecvQueueManager->RCBCount; Index++)
        {
            pRecvQueueManager->RecvCtrlBlocks[Index].BufSize        = RCB_DATA_BUF_SIZE;
            pRecvQueueManager->RecvCtrlBlocks[Index].pVirtualAddress =
                pRecvQueueManager->pVirtualStartAddress + Index * RCB_DATA_BUF_SIZE;

            pRecvQueueManager->RecvCtrlBlocks[Index].pMdl =
                IoAllocateMdl(
                    pRecvQueueManager->RecvCtrlBlocks[Index].pVirtualAddress, 
                    pRecvQueueManager->RecvCtrlBlocks[Index].BufSize, 
                    FALSE, 
                    FALSE, 
                    NULL);
            
            MmBuildMdlForNonPagedPool(pRecvQueueManager->RecvCtrlBlocks[Index].pMdl);
            NdisInterlockedInsertTailList(
                &pRecvQueueManager->RecvFreeList,
                &pRecvQueueManager->RecvCtrlBlocks[Index].List,
                &pRecvQueueManager->QueueLock);

        }

    } while (FALSE);

    return hRes;
}

/* 
CleanupRecvQueueManager Release memory resources and mdl resources in all ULCB
and destroy the RecvQueue

Parameters:
    pRecvQueueManager: Pointer to PRECV_QUEUE_MANAGER structure

Return:    

Note:
    It is not thread-safe.

History:   
    2009/11/5 : double linked list queue by senxiang
IRQL: <= DPC_LEVEL
 
*/
VOID
CleanupRecvQueueManager(IN PRECV_QUEUE_MANAGER pRecvQueueManager)
{
    ULONG   Index;

    NdisFreeSpinLock(&pSendQueueManager->QueueLock);
    do 
    {
        // Free MDLs
        for (Index = 0; Index < pRecvQueueManager->RCBCount; Index++)
        {
            if (NULL != pRecvQueueManager->RecvCtrlBlocks[Index].pMdl)
            {
                IoFreeMdl(pRecvQueueManager->RecvCtrlBlocks[Index].pMdl);
                pRecvQueueManager->RecvCtrlBlocks[Index].pMdl = NULL;
            }
        }

        // Free data memory
        if (NULL != pRecvQueueManager->pVirtualStartAddress)
        {
            NdisFreeMemory(
                pRecvQueueManager->pVirtualStartAddress, 
                pRecvQueueManager->Size,
                0);
            pRecvQueueManager->pVirtualStartAddress = NULL;
        }

    } while (FALSE);

}