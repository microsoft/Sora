/*++
Copyright (c) Microsoft Corporation

Module Name: Sora RX DMA buffer manager

Abstract: This file implements interface functions to manager the RX DMA buffer.

History: 
          3/23/2009: Created by senxiang
--*/

#include "sora.h"
#include "__reg_file.h"
#include "__rx_internal.h"

/* 
SoraInitRxBufferManager initialize RX DMA buffer pool/queue manager of radio oject 

 Parameters:
    hRXMgr: RX manager handle of radio object
    nRXBufSize: RX buffer size
    nRXBufNum: RX buffer number in the pool/queue

 Return:    S_OK if succeeded. Error code if failed.

 Note: nRXBufSize and nRXBufNum are reserved for future use. nRXBufNum is assumed 
       to be 1, and nRXBufSize is assumed to be 16MB

 History:   3/23/2009 Created by senxiang
*/
VOID SoraInitVStreamMan(__PRX_QUEUE_MANAGER pRxQueue);
HRESULT SoraInitRxBufferManager(
        IN SORA_RX_MAN_HANDLE   hRXMgr,
        IN ULONG                nRXBufSize,
        IN ULONG                nRXBufNum)

{
    HRESULT             hRes                 = S_OK;
    PHYSICAL_ADDRESS    PhysicalAddress      = {0, 0}; 
    PHYSICAL_ADDRESS    PhysicalAddressLow   = {0, 0};
    PHYSICAL_ADDRESS    PhysicalAddressHigh  = {0x80000000, 0};
    ULONG               Index;
    __PRX_QUEUE_MANAGER pRxQueueManager      = (__PRX_QUEUE_MANAGER)hRXMgr;
    
    SoraInitVStreamMan(pRxQueueManager);

    do 
    {  
        pRxQueueManager->__pVirtualStartAddress = 
            (PUCHAR) MmAllocateContiguousMemorySpecifyCache(
                        nRXBufSize * nRXBufNum, 
                        PhysicalAddressLow, 
                        PhysicalAddressHigh, 
                        PhysicalAddress, 
                        MmCached
                        );

        if (NULL == pRxQueueManager->__pVirtualStartAddress)
        {
            hRes = E_NOT_ENOUGH_CONTINUOUS_PHY_MEM;
            break;
        }
        
        pRxQueueManager->__uSize = nRXBufSize * nRXBufNum;

        RtlZeroMemory(pRxQueueManager->__pVirtualStartAddress, pRxQueueManager->__uSize);

        PhysicalAddress = MmGetPhysicalAddress((PVOID)pRxQueueManager->__pVirtualStartAddress);
        ASSERT(PhysicalAddress.u.LowPart % 0x00001000 == 0);
        //DbgPrint("[TEMP1] %08x,%08x", PhysicalAddress.u.HighPart, PhysicalAddress.u.LowPart);
        pRxQueueManager->__pPhysicalStartAddress.QuadPart = (PhysicalAddress.QuadPart);

        pRxQueueManager->__pVirtualEndAddress = 
            pRxQueueManager->__pVirtualStartAddress + pRxQueueManager->__uSize;

        pRxQueueManager->__pPhysicalEndAddress.QuadPart = 
            pRxQueueManager->__pPhysicalStartAddress.QuadPart + pRxQueueManager->__uSize;

        pRxQueueManager->__uCount = nRXBufNum;

        for (Index = 0; Index < pRxQueueManager->__uCount; Index++)
        {
            pRxQueueManager->__RxQueues[Index].__pRxDesc =
                (__PRX_DESC)(pRxQueueManager->__pVirtualStartAddress + Index * nRXBufSize);
            
            pRxQueueManager->__RxQueues[Index].__PhysicalAddress =
                (ULONG)(pRxQueueManager->__pPhysicalStartAddress.LowPart + Index * nRXBufSize);

            pRxQueueManager->__RxQueues[Index].__uSize = nRXBufSize;
        }

        pRxQueueManager->__uIndex = 0;

    } while (FALSE);

    return hRes;
}

/* 
SoraCleanupRxBufferManager cleanup RX DMA buffer pool/queue manager of radio oject 

 Parameters:
            hRXMgr: RX manager handle of radio object

 Return:    

 Note: SoraCleanupRxBufferManager release non-paged memory allocated for RX buffer.

 History:   3/25/2009 Created by senxiang
*/
VOID SoraCleanupRxBufferManager(IN SORA_RX_MAN_HANDLE hRxMgr)
{
    if (__IS_RX_MANAGER_INIT(hRxMgr))
    {
        LARGE_INTEGER   Delay = {0};
        Delay.QuadPart = -100 * 1000 * 10; // 100ms
        KeDelayExecutionThread(KernelMode, FALSE, &Delay); //wait hardware write left symbols to RX buffer. 
        MmFreeContiguousMemorySpecifyCache(
                ((__PRX_QUEUE_MANAGER)hRxMgr)->__pVirtualStartAddress,
                ((__PRX_QUEUE_MANAGER)hRxMgr)->__uSize, 
                MmCached
                );
        __ZERO_RX_MANAGER((__PRX_QUEUE_MANAGER)hRxMgr);
    }
}