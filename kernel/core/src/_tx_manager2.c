/*++
Copyright (c) Microsoft Corporation

Module Name: _tx_manager2.c

Abstract: This file implements the managment to RCB onboard memory and
          Tx Descriptors. 
          All public function is thread-safe.

History: 
          5/15/2009: Created by senxiang
          13/May/2011: Reviewed and modified by Kun Tan

Remark:
    The RCB memory is organized into 512KB blocks. A RCB Memory
    Descriptor (RMD) is a data structure specifing a continous memory
    on RCB.
--*/

#include "sora.h"
#include "stdlib.h"
#include "__tx_res.h"

//
// Convert memory size from bytes to memory units
//
__inline ULONG __ConvertMemLenFromByteToUnit ( ULONG uSize )
{
    return (uSize == 0) ? 0 : 
           ((uSize - 1) >> TX_RCB_MEM_UNIT_SHIFT) + 1;
}

//
// Get a RCB memory descriptor (RMD) from a list
//
__inline PRCB_MD __GetFreeRMD (PLIST_ENTRY list)
{
    PLIST_ENTRY     entry = NULL;

    if (!IsListEmpty(list))
        entry = RemoveHeadList(list);

    return CONTAINING_RECORD(entry, RCB_MD, __List);
}

//
// Split a large RCB memory block into into two smaller blocks. 
// The function returns the leftover block.
//
__inline PRCB_MD 
__SplitMemSeg( IN PTX_RM           pTxResMgr, 
               IN OUT PRCB_MD      pBigMem, 
               ULONG               uReqUnits)
{
    PRCB_MD    leftDesc = NULL;

    if (uReqUnits < pBigMem->Units) 
    {        
        leftDesc = __GetFreeRMD (&(pTxResMgr->__FreeRMDList));
        leftDesc->Start = pBigMem->Start + 
                          (uReqUnits << TX_RCB_MEM_UNIT_SHIFT);
        leftDesc->Units = pBigMem->Units - uReqUnits;
        pBigMem->Units  = uReqUnits;
    }
    return leftDesc;
}

//
// Add a free Tx descriptor to the list head
//
__inline void 
__InsertHead ( IN PTX_DESC *pList, IN PTX_DESC pEntry)
{
    pEntry->__NextDesc = *pList;
    *pList = pEntry;
}

/*++
SoraInitTxResManager initiate the TX RCB memory pool and Tx descriptor pool.

Parameters:
    pTxResMgr: pointer to TX Resource Manager.

Return:

Note: 
    IRQL: <= DISPATCH_LEVEL.

History:    25/May/2009 Created by senxiang
--*/
HRESULT SoraInitTxResManager(OUT PTX_RM pTxResMgr)
{
    HRESULT hr = E_NOT_ENOUGH_CONTINUOUS_PHY_MEM;
    ULONG i;
    PRCB_MD pWholeMemDesc;
    PHYSICAL_ADDRESS    PhysicalAddress      = {0, 0};
    PHYSICAL_ADDRESS    PhysicalAddressLow   = {0, 0};
    PHYSICAL_ADDRESS    PhysicalAddressHigh  = {0x80000000, 0};

    //
    // TX_DESC is a shared resource - allocate just enough
    //
    ULONG size = sizeof(TX_DESC) * MAX_RCB_MD_NUM * 32;

    KeInitializeSpinLock(&pTxResMgr->__Lock);
    InitializeListHead  (&pTxResMgr->__FreeRMDList);
    pTxResMgr->__pFreeTxDesc      = NULL;

    for ( ; size >  sizeof(TX_DESC) * MAX_RCB_MD_NUM; 
            size -= sizeof(TX_DESC) * MAX_RCB_MD_NUM )
    {
        pTxResMgr->__pTxDescList = 
            (PTX_DESC) MmAllocateContiguousMemorySpecifyCache(
                        size , 
                        PhysicalAddressLow, 
                        PhysicalAddressHigh, 
                        PhysicalAddress, 
                        MmNonCached
                        );
        if (pTxResMgr->__pTxDescList != NULL)
        {
            pTxResMgr->__uTxDescSize = size;
            hr = S_OK;
            break;
        }
    }
    if (FAILED(hr))
    {
        return hr;
    }

    PhysicalAddress = MmGetPhysicalAddress((PVOID)pTxResMgr->__pTxDescList);
    
    //should always 4K aligned
    ASSERT(PhysicalAddress.u.LowPart % 0x00001000 == 0); 

    // Initialize the Tx Descriptor pool
    for (i = 0; i < pTxResMgr->__uTxDescSize / sizeof(TX_DESC); i++) 
    {
        __InsertHead (  &pTxResMgr->__pFreeTxDesc, 
                        &pTxResMgr->__pTxDescList[i]);
        
        pTxResMgr->__pTxDescList[i].ThisPa.QuadPart  = 
            PhysicalAddress.QuadPart + sizeof(TX_DESC) * i;
        
        pTxResMgr->__pTxDescList[i].__Delimiter = 0xCDCDCDCD;
        pTxResMgr->__pTxDescList[i].__StatusValid = 1;
        pTxResMgr->__pTxDescList[i].__MoreDesc = 0;
        pTxResMgr->__pTxDescList[i].__Reserved = 0xC;
    }

    // Initialize the RCB memory descriptor
    for (i = 0; i < MAX_RCB_MD_NUM; i++) 
    {
        pTxResMgr->__DescPool[i].Bytes = 0xFFFFFFFF;
        InsertTailList( &pTxResMgr->__FreeRMDList, 
                        &pTxResMgr->__DescPool[i].__List);
        
        InitializeListHead(&pTxResMgr->__FreeMem[i]);
    }

    // now, all RCB memory is free; allocate one RMD to describe
    // the whole memory
    pWholeMemDesc = __GetFreeRMD (&pTxResMgr->__FreeRMDList);
    ASSERT(pWholeMemDesc);
    
    pWholeMemDesc->Start    = (HW_RESERVED_UNIT_NUM << TX_RCB_MEM_UNIT_SHIFT);
    pWholeMemDesc->Units    = MAX_RCB_MEM_UNIT_NUM - HW_RESERVED_UNIT_NUM;
    InsertHeadList( &pTxResMgr->__FreeMem[MAX_RCB_MD_NUM - 1], 
                    &pWholeMemDesc->__List);

    return hr;
}

/*++
SoraCleanTxResManager release resources.

Parameters:
    pTxResMgr : pointer to the Tx Resource Manager.

Return : 

Note: 
    IRQL: <= DISPATCH_LEVEL
--*/
void SoraCleanTxResManager(IN PTX_RM pTxResMgr)
{
    // Only free up the allocated Tx Descriptors
    if (pTxResMgr->__pTxDescList)
    {
        MmFreeContiguousMemorySpecifyCache(
            pTxResMgr->__pTxDescList,
            pTxResMgr->__uTxDescSize,
            MmNonCached);
        
        pTxResMgr->__pTxDescList = NULL;
        pTxResMgr->__uTxDescSize = 0;
    }
    
    RtlZeroMemory(pTxResMgr, sizeof(TX_RM));
}

/*++
SoraAllocateRCBMem allocate a segment of RCB memory.

Parameters:
    pTxResMgr: pointer to TX Resource Manager.
    uSize    : number of bytes needed

Return: 
    NULL if no memory is available or size equals to 0. 
    Otherwise a memory descriptor returned.

Note: 
        IRQL: <= DISPATCH_LEVEL.

History:    25/May/2009 Created by senxiang
--*/
PRCB_MD SoraAllocateRCBMem (
        IN PTX_RM       pTxResMgr, 
        IN ULONG        uSize)
{
    ULONG       UnitNum;
    PRCB_MD     pMem = NULL;
    int i;
    UnitNum = __ConvertMemLenFromByteToUnit (uSize);
    do {
        KIRQL oldIrql;
        if (UnitNum == 0) break;
        if (pTxResMgr == NULL)
        {
            DbgPrint("[Error] mm = NULL, can't allocate\n");
            return NULL;
        }
        KeAcquireSpinLock(&pTxResMgr->__Lock, &oldIrql);
        for (i = UnitNum - 1; i < MAX_RCB_MD_NUM; i++)
        {
            if (!IsListEmpty(&pTxResMgr->__FreeMem[i]))
            {
                PRCB_MD matchedMem, leftMem;
                matchedMem = CONTAINING_RECORD(
                                RemoveHeadList(&pTxResMgr->__FreeMem[i]), RCB_MD, __List);
                leftMem = __SplitMemSeg(pTxResMgr, matchedMem, UnitNum);
                if (leftMem != NULL)
                {
                    InsertHeadList( &pTxResMgr->__FreeMem[leftMem->Units - 1],
                                    &leftMem->__List);
                }
                pMem = matchedMem;
                break;
            }
        }
        KeReleaseSpinLock(&pTxResMgr->__Lock, oldIrql);
    }while(FALSE);
    return pMem;   
}

/*++
SoraAllocateTxDesc allocate a free Tx descriptor.

Parameters:
    pTxResMgr: pointer to Tx Resource manager.

Return: 
    NULL if no Tx descriptor is available. 

Note: 
    IRQL: <= DISPATCH_LEVEL.

History: 25/May/2009 Created by senxiang
         13/May/2011 Reviewed and modified by Kun Tan
--*/
PTX_DESC 
SoraAllocateTxDesc ( IN PTX_RM pTxResMgr )
{
    PTX_DESC pDesc = NULL;
    KIRQL oldIrql;
    KeAcquireSpinLock(&pTxResMgr->__Lock, &oldIrql);
    if (pTxResMgr->__pFreeTxDesc != NULL)
    {
        // Get a descriptor from the free list
        pDesc = pTxResMgr->__pFreeTxDesc;
        pTxResMgr->__pFreeTxDesc = pDesc->__NextDesc;

        // initialize the descriptor
        pDesc->__NextDesc       = NULL;
        pDesc->pSampleBuffer    = NULL;
        pDesc->pRMD     = NULL;
    }
    
    KeReleaseSpinLock(&pTxResMgr->__Lock, oldIrql);
    
    return pDesc;
}

/*++
SoraFreeTxDesc releases a Tx descriptor into pool.

Parameters:
    pTxResMgr: pointer to TX Resource Manager.
    pDesc    : unused Tx descriptor.

Return: 

Note: 
        IRQL: <= DISPATCH_LEVEL.

History:    25/May/2009 Created by senxiang
--*/
void SoraFreeTxDesc(IN PTX_RM pTxResMgr, IN PTX_DESC pDesc)
{
    KIRQL oldIrql;
    
    KeAcquireSpinLock(&pTxResMgr->__Lock, &oldIrql);
    __InsertHead     (&pTxResMgr->__pFreeTxDesc, pDesc);
    KeReleaseSpinLock(&pTxResMgr->__Lock, oldIrql);
}

/*++
SoraFreeRCBMem free a segment of TX RCB memory.

Parameters:
    pTxResMgr: pointer to TX Resource Manager.
    pRMD     : RCB memory descriptor.
    
Return: 

Note: 
    IRQL: <= DISPATCH_LEVEL. 
    
    If pRMD is allocated from a RCB memory pool, DO NOT free it 
    through SoraFreeRCBMem, use SoraFreeRCBMemToPool instead.

History:    26/May/2009 Created by senxiang
--*/
void SoraFreeRCBMem(IN PTX_RM pTxResMgr, IN PRCB_MD pRMD)
{
    KIRQL oldIrql;

    // Mem is allocated in a pool, can't release to tx mm
    if (pRMD->Bytes != 0xFFFFFFFF) 
    {
        KeBugCheck(BUGCODE_ID_DRIVER);
    }
    
    ASSERT(pRMD != NULL);
    ASSERT(pRMD->Units > 0 && pRMD->Units <= MAX_RCB_MEM_UNIT_NUM);
    
    KeAcquireSpinLock(&pTxResMgr->__Lock, &oldIrql);
    InsertHeadList   (&pTxResMgr->__FreeMem[pRMD->Units - 1], &pRMD->__List);
    KeReleaseSpinLock(&pTxResMgr->__Lock, oldIrql);
}

//
// Compare two TX RCB memory blocks by the start address
//
int (__cdecl _PtFuncCompare)(const void *x, const void *y)
{
    ASSERT(x && y);
    return ((*(PRCB_MD*)x)->Start - (*(PRCB_MD*)y)->Start);
}

//
// Decide if two TX RCB memory is adjacent
//
__inline BOOL __IsAdjacent(
            IN const PRCB_MD pMLo, 
            IN const PRCB_MD pMHi)
{
    return (pMLo->Start + (pMLo->Units << TX_RCB_MEM_UNIT_SHIFT) == pMHi->Start);
}

//
// Merge two adjacent small RCB memory blocks into big one.
//
PRCB_MD __Merge2Mem(IN PTX_RM pTxResMgr, IN PRCB_MD pMLo, IN PRCB_MD pMHi)
{
    ASSERT(__IsAdjacent(pMLo, pMHi));
    RemoveEntryList(&pMLo->__List);
    RemoveEntryList(&pMHi->__List);
    pMLo->Units += pMHi->Units;
    
    InsertHeadList(&pTxResMgr->__FreeMem[pMLo->Units - 1], &pMLo->__List);
    InsertHeadList(&pTxResMgr->__FreeRMDList, &pMHi->__List);
    
    return pMLo;
}

/*++
SoraMergeRCBMem defrages RCB memory.

Parameters:
    pTxResMgr : point to the Tx Resource Manager.
    
Return: 

Note: 
        IRQL: <= DISPATCH_LEVEL.

History:    5/26/2009 Created by senxiang
--*/
void SoraMergeRCBMem (IN PTX_RM pTxResMgr )
{
    int i, j = 0;
    KIRQL oldIrql;

#ifdef DBG
    RtlZeroMemory(pTxResMgr->__SortedMemDesc, sizeof(pTxResMgr->__SortedMemDesc));
#endif 

    KeAcquireSpinLock(&pTxResMgr->__Lock, &oldIrql);
    for (i = 0; i < MAX_RCB_MD_NUM; i++)
    {
        PLIST_ENTRY p = &pTxResMgr->__FreeMem[i];
        if (!IsListEmpty(p))
        {    
            while (p->Flink != &pTxResMgr->__FreeMem[i])
            {
                p = p->Flink;
                pTxResMgr->__SortedMemDesc[j] = CONTAINING_RECORD(p, RCB_MD, __List);
                j++;
            }
        }
    }
    
    qsort(pTxResMgr->__SortedMemDesc, j, sizeof(PRCB_MD), _PtFuncCompare);

    for (i = 0; i < j - 1; i++)
    {
        if (__IsAdjacent(pTxResMgr->__SortedMemDesc[i], pTxResMgr->__SortedMemDesc[i + 1]))
        {
            PRCB_MD pBigMem = 
                __Merge2Mem(pTxResMgr, pTxResMgr->__SortedMemDesc[i], pTxResMgr->__SortedMemDesc[i + 1]);
                pTxResMgr->__SortedMemDesc[i + 1] = pBigMem;
        }
    }

#ifdef DBG
    RtlZeroMemory(pTxResMgr->__SortedMemDesc, sizeof(pTxResMgr->__SortedMemDesc));
#endif 

    KeReleaseSpinLock(&pTxResMgr->__Lock, oldIrql);
}

//
// Debug helpers
//

void __PrintFreeRCBMem(PRCB_MD pRMD)
{
    ULONG i;
    KdPrint(("[%dK]", pRMD->Start >> 10));
    
    for (i = 0; i < pRMD->Units; i++)
        KdPrint(("-"));

    KdPrint(("[%dK] \n", (pRMD->Start + (pRMD->Units << TX_RCB_MEM_UNIT_SHIFT)) >> 10));
}

void __PrintUsedRCBMem(PRCB_MD pLeft, PRCB_MD pRight)
{
    ULONG i;
    if (!__IsAdjacent(pLeft, pRight))
    {
        KdPrint(("[%dK]", (pLeft->Start + (pLeft->Units << TX_RCB_MEM_UNIT_SHIFT)) >> 10));
        for (i = 0; 
            i < ((pRight->Start - pLeft->Start) >> TX_RCB_MEM_UNIT_SHIFT) - pLeft->Units;
            i++ )
        {
            KdPrint(("+"));
        }
        KdPrint(("[%dK]\n", pRight->Start >> 10));
    }

}

void __PrintOneUsedRCBMem(PRCB_MD pRMD)
{
    ULONG i;
    KdPrint(("[%dK]", pRMD->Start >> 10));
    
    for (i = 0; i < pRMD->Units; i++)
        KdPrint(("+"));

    KdPrint(("[%dK] \n", (pRMD->Start + (pRMD->Units << TX_RCB_MEM_UNIT_SHIFT)) >> 10));
}

void __SoraPrintRCBMem (IN PTX_RM pTxResMgr)
{
    int i, j = 0;
    KIRQL oldIrql;

    KdPrint(("\nRCB memory status:\n"));
    KeAcquireSpinLock(&pTxResMgr->__Lock, &oldIrql);
    for (i = 0; i < MAX_RCB_MD_NUM; i++)
    {
        PLIST_ENTRY p = &pTxResMgr->__FreeMem[i];
        if (!IsListEmpty(p))
        {    
            while (p->Flink != &pTxResMgr->__FreeMem[i])
            {
                p = p->Flink;
                pTxResMgr->__SortedMemDesc[j] = CONTAINING_RECORD(p, RCB_MD, __List);
                j++;
            }
        }
    }
    
    qsort(pTxResMgr->__SortedMemDesc, j, sizeof(PRCB_MD), _PtFuncCompare);

    for (i = 0; i < j; i++)
    {
        if (i == 0)
        {
            if (pTxResMgr->__SortedMemDesc[0]->Start != 0)
            {
                RCB_MD begin;
                begin.Start = 0;
                begin.Units = pTxResMgr->__SortedMemDesc[0]->Start >> TX_RCB_MEM_UNIT_SHIFT;
                __PrintOneUsedRCBMem (&begin);
            }
        }
        __PrintFreeRCBMem(pTxResMgr->__SortedMemDesc[i]);
        if (i < j - 1)
            __PrintUsedRCBMem(pTxResMgr->__SortedMemDesc[i], pTxResMgr->__SortedMemDesc[i+1]);
    }
    KeReleaseSpinLock(&pTxResMgr->__Lock, oldIrql);
}

#pragma region "RCB Memory Pool Management"

/*++
SoraInitRCBMemPool initialize a RCB memory pool.

Parameters:
        pTxMemPool: pointer to the pool, the pool object should 
                    be provided by caller.
        pTxResMgr : TX Resource Manager.
        uMemSize  : size of the memory block.
        uCount    : number of memory block in the pool will contain.

Return:     S_OK if succeeded. E_NOT_RCB_MEMORY if there no enough free TX RCB buffer 
            available for the pool.

Note: 
            IRQL: <= DISPATCH_LEVEL. 

History:    26/May/2009 Created by senxiang
--*/
HRESULT SoraInitRCBMemPool(
            OUT PRCB_MEM_POOL   pTxMemPool,
            IN PTX_RM           pTxResMgr,
            IN ULONG            uMemSize,
            IN ULONG            uCount)
{
    HRESULT hr = S_OK;
    ULONG i;
    ASSERT(pTxMemPool->__pTotalRCBMD == NULL);
    pTxMemPool->__pTotalRCBMD =
                    SoraAllocateRCBMem(pTxResMgr, uMemSize * uCount);
    
    if (NULL == pTxMemPool->__pTotalRCBMD) //move up
    {
        hr = E_NOT_RCB_MEMORY;
        return hr;
    }
    else
    {
        InitializeListHead  (&pTxMemPool->__FreeRCBMDL);
        
        KeInitializeSpinLock(&pTxMemPool->__PoolLock);
        ExInitializeNPagedLookasideList(
            &pTxMemPool->__SmallRCBMDL,
            NULL, 
            NULL, 
            0,
            sizeof(RCB_MD),
            __SORA_TAG,
            0);
        
        //if allocate is ok, then set frame size for RCB_MEM_POOL
        pTxMemPool->__nSize = uMemSize;
    }
    for (i = 0; i < uCount; i++)
    {
        PRCB_MD pTxMem = (PRCB_MD)
            ExAllocateFromNPagedLookasideList(&pTxMemPool->__SmallRCBMDL);
        
        if (NULL == pTxMem)
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }
        pTxMem->Units = 0xFFFFFFFF;
        pTxMem->Bytes = uMemSize;
        pTxMem->Start = pTxMemPool->__pTotalRCBMD->Start + i * uMemSize;
        InsertHeadList(&pTxMemPool->__FreeRCBMDL, &pTxMem->__List);
    }
    
    return hr;
}

/*++
SoraCleanRCBMemPool releases resouces occupied by the pool.

Parameters:
    pTxMemPool: pointer to the pool.
    pTxResMgr : TX Resource Manager.

Return:     

Note: 
    IRQL: <= DISPATCH_LEVEL. 

History:    26/May/2009 Created by senxiang
--*/
void SoraCleanRCBMemPool (
            IN OUT PRCB_MEM_POOL pTxMemPool,
            IN PTX_RM            pTxResMgr )
{
    if (NULL != pTxMemPool->__pTotalRCBMD)
    {
        SoraFreeRCBMem(pTxResMgr, pTxMemPool->__pTotalRCBMD);
        pTxMemPool->__pTotalRCBMD = NULL;
    }
	KIRQL OldIrql;
    KeAcquireSpinLock(&pTxMemPool->__PoolLock, &OldIrql);
    while (!IsListEmpty(&pTxMemPool->__FreeRCBMDL))
    {
        PLIST_ENTRY pEntry = RemoveHeadList(&pTxMemPool->__FreeRCBMDL);
        PRCB_MD pMem = CONTAINING_RECORD(pEntry, RCB_MD, __List);
		ExFreeToNPagedLookasideList(&pTxMemPool->__SmallRCBMDL, pMem);
    }
    KeReleaseSpinLock(&pTxMemPool->__PoolLock, OldIrql);
	
    InitializeListHead(&pTxMemPool->__FreeRCBMDL);
    ExDeleteNPagedLookasideList(&pTxMemPool->__SmallRCBMDL);
    return;
}

/*++
SoraAllocateRCBMemFromPool allocate a fixed-size TX RCB memory from the pool.

Parameters:
    pTxMemPool: pointer to the pool.

Return:     
    An RMD for free TX RCB memory, or NULL if fails.

Note: 
        IRQL: <= DISPATCH_LEVEL. 

History:    26/May/2009 Created by senxiang
--*/
PRCB_MD SoraAllocateRCBMemFromPool(
                IN PRCB_MEM_POOL pTxMemPool)
{
    KIRQL OldIrql;
    PRCB_MD pMem = NULL;
    KeAcquireSpinLock(&pTxMemPool->__PoolLock, &OldIrql);
    if (!IsListEmpty(&pTxMemPool->__FreeRCBMDL))
    {
        PLIST_ENTRY pEntry = RemoveHeadList(&pTxMemPool->__FreeRCBMDL);
        pMem = CONTAINING_RECORD(pEntry, RCB_MD, __List);
    }
    KeReleaseSpinLock(&pTxMemPool->__PoolLock, OldIrql);
    return pMem;
}

/*++
SoraFreeRCBMemIntoPool releases TX RCB memory into the pool.

Parameters:
    pRCBMemPool: pointer to the pool.
    pRMD       : RMD.

Return:     

Note: 
        IRQL: <= DISPATCH_LEVEL. 
        DO NOT release a RMD allocated by SoraAllocateRCBMem using this function, 
        use SoraFreeRCBMem instead.

History:    5/26/2009 Created by senxiang
--*/
void SoraFreeRCBMemToPool(
            IN PRCB_MEM_POOL pRCBMemPool,
            IN PRCB_MD pRMD
            )
{
    KIRQL OldIrql;
    
    if (pRMD->Units != 0xFFFFFFFF)
    {
        KeBugCheck(BUGCODE_ID_DRIVER); //Unit TX memory can't release into tx mem pool 
    }
    KeAcquireSpinLock(&pRCBMemPool->__PoolLock, &OldIrql);
    InsertHeadList(&pRCBMemPool->__FreeRCBMDL, &pRMD->__List);
    KeReleaseSpinLock(&pRCBMemPool->__PoolLock, OldIrql);
}


/*++
  Currently only for RCB signal cache, not public yet.
--*/
// void __SoraSignalGetTxResource(
void __SoraSetTxDescAddress (
                    IN PTXSAMPLE  pSampleBufferVa, 
                    IN ULONG      pSampleBufferPaLo,
                    IN ULONG      pSampleBufferPaHi,
                    IN ULONG      SamplebufferSize,
                    IN OUT PTX_DESC pTxDesc)
{
    pTxDesc->__SourceAddressLo   = pSampleBufferPaLo;
    pTxDesc->__SourceAddressHi   = pSampleBufferPaHi;
    pTxDesc->pSampleBuffer       = pSampleBufferVa;
    pTxDesc->SampleBufferSize    = SamplebufferSize;
}

// void __SoraPhyFrameSetFrameSize(
void __SoraSetTxDescSize(
    IN OUT PTX_DESC  pTxDesc, 
    ULONG            uSize)
{
    pTxDesc->Size = uSize;
}

// void SoraPhyFrameFreeTxResource(
void SoraResetTxDesc (
    IN PSORA_RADIO   pRadio, 
    IN OUT PTX_DESC  pTxDesc)
{
    if (pTxDesc->pRMD)
    {
        SoraFreeRCBMem(pRadio->pTxResMgr, pTxDesc->pRMD);
        pTxDesc->pRMD = NULL;
    }
    pTxDesc->pSampleBuffer       = NULL;
}

// void SoraPhyFrameFreeTxResourceEx(
void SoraResetTxDescWithPool (
    IN PRCB_MEM_POOL pTxMemPool,
    IN OUT PTX_DESC  pTxDesc)
{
    if (pTxDesc->pRMD)
    {
        SoraFreeRCBMemToPool(pTxMemPool, pTxDesc->pRMD);
        pTxDesc->pRMD = NULL;
    }
    pTxDesc->pSampleBuffer       = NULL;
}

#pragma endregion 