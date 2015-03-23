#include "sora.h"
#include "_signal_cache.h"
#include "__tx_res.h"
#include "__reg_file.h"
#include "__transfer_obj.h"

void __SoraSetTxDescAddress (
        IN PCOMPLEX8            pSampleBufferVa, 
        IN ULONG                pSampleBufferPaLo,
        IN ULONG                pSampleBufferPaHi,
        IN ULONG                SamplebufferSize,
        IN OUT PTX_DESC pTxDesc);

void __SoraSetTxDescSize (
                IN OUT PTX_DESC  pTxDesc, 
                ULONG            uSize);


HRESULT 
__SORA_HW_TX_TRANSFER_SIGNAL_EX(
    IN HANDLE    		TransferObj,
    IN PRCB_MEM_POOL    pTxMemPool,
    IN PTX_DESC         pTxDesc);

__inline ULONG __FnHash(CACHE_KEY key)
{
    return ((ULONG)(key.QuadKey.u.LowPart + key.QuadKey.u.HighPart) % __HASH_ENTRY);
}

__inline  PTX_DESC __QuerySignal(PHASH_TBL pHTbl, CACHE_KEY key)
{
    ULONG Index = __FnHash(key);
    PLIST_ENTRY  pHead = &pHTbl->Tbl[Index];
    PLIST_ENTRY  pEntry;
    KIRQL        OldIrql;
    __PKEY_VALUE p = NULL;

    KeAcquireSpinLock(&pHTbl->LockTbl[Index], &OldIrql);
    pEntry = pHead->Flink;
    while (pHead != pEntry)
    {
        p = CONTAINING_RECORD(pEntry, __KEY_VALUE, __List);
        if (p->__Key.QuadKey.u.LowPart == key.QuadKey.u.LowPart
            &&
            p->__Key.QuadKey.u.HighPart == key.QuadKey.u.HighPart)
        {
            break;
        }
        else
        {
            p = NULL;
        }
        pEntry = pEntry->Flink;
    }
    KeReleaseSpinLock(&pHTbl->LockTbl[Index], OldIrql);
    return p? p->__Value : NULL;
}

HRESULT 
SORAAPI
SoraInitSignalCache(
    OUT PSIGNAL_CACHE    pCache, 
    IN  HANDLE   	 	 TransferObj,
    IN  PSORA_RADIO      pRadio,
    ULONG                uSize,
    ULONG                uMaxEntryNum )
{
    ULONG i; 
    HRESULT hr;

    if (uSize & RCB_BUFFER_ALIGN_MASK)
    { 
        hr = E_INVALID_SIGNAL_SIZE;
        return hr;
    }
    
    ExInitializeNPagedLookasideList(
            &pCache->__KeyValueList,
            NULL, 
            NULL, 
            0,
            sizeof(__KEY_VALUE),
            __SORA_TAG,
            0);
	pCache->__TransferObj = TransferObj;
    pCache->__pRadio = pRadio;
    for (i = 0; i < __HASH_ENTRY; i++)
    {
        InitializeListHead(&pCache->__HashTbl.Tbl[i]);
        KeInitializeSpinLock(&pCache->__HashTbl.LockTbl[i]);
    }

    hr = SoraInitRCBMemPool ( &pCache->__TxMemPool, 
                              pRadio->pTxResMgr, 
                              uSize, 
                              uMaxEntryNum );
    if (FAILED(hr))
    {
        ExDeleteNPagedLookasideList(&pCache->__KeyValueList);
        pCache->__pRadio    = NULL;
    }
    return hr;
}

VOID SORAAPI SoraCleanSignalCache(IN PSIGNAL_CACHE pCache)
{
    int i;
    PLIST_ENTRY  pEntry;
    __PKEY_VALUE p          = NULL;
    
    for (i = 0; i < __HASH_ENTRY; i++)
    {
        PLIST_ENTRY  pHead      = &pCache->__HashTbl.Tbl[i];
        pEntry = pHead->Flink;
        while (pHead != pEntry)
        {
            p = CONTAINING_RECORD(pEntry, __KEY_VALUE, __List); 
			SoraFreeRCBMemToPool(&pCache->__TxMemPool, p->__Value->pRMD);
            SoraFreeTxDesc(pCache->__pRadio->pTxResMgr, p->__Value);
            p->__Value = NULL;
            pEntry = pEntry->Flink;
        }
    }
    ExDeleteNPagedLookasideList(&pCache->__KeyValueList);
    SoraCleanRCBMemPool(&pCache->__TxMemPool, pCache->__pRadio->pTxResMgr);
    pCache->__pRadio = NULL;
}


PTX_DESC
SORAAPI
SoraGetSignal(
    IN PSIGNAL_CACHE pCache, 
    IN CACHE_KEY     Key)
{
    return __QuerySignal(&pCache->__HashTbl, Key);
}

HRESULT 
SORAAPI
SoraInsertSignal (
    IN PSIGNAL_CACHE     pCache, 
    IN PCOMPLEX8         pSampleBuffer,
    IN PHYSICAL_ADDRESS *pSampleBufferPa,
    IN ULONG             uSampleCount,
    IN CACHE_KEY         Key)
{
    __PKEY_VALUE pKV        = (__PKEY_VALUE) ExAllocateFromNPagedLookasideList(&pCache->__KeyValueList);
    HRESULT hr              = S_OK;
    ULONG Index             = __FnHash(Key);
    PLIST_ENTRY  pHead      = &pCache->__HashTbl.Tbl[Index];
    PLIST_ENTRY  pEntry;
    KIRQL OldIrql;
    __PKEY_VALUE p          = NULL;
    PTX_DESC pSignal = SoraAllocateTxDesc(pCache->__pRadio->pTxResMgr);
    
    do 
    {
        if (NULL == pKV || NULL == pSignal)
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }
        
        if ((uSampleCount & RCB_BUFFER_ALIGN_MASK) != 0)
        {
            hr = E_INVALID_SIGNAL_SIZE;
            break;
        }

        __SoraSetTxDescAddress (
            pSampleBuffer, 
            pSampleBufferPa->u.LowPart,
            pSampleBufferPa->u.HighPart,
            uSampleCount,
            pSignal);
        
        __SoraSetTxDescSize(pSignal, uSampleCount);

        pKV->__Key.QuadKey.QuadPart = Key.QuadKey.QuadPart;
        pKV->__Value                = pSignal;
        InitializeListHead(&pKV->__List);
        
        KeAcquireSpinLock(&pCache->__HashTbl.LockTbl[Index], &OldIrql);
        pEntry = pHead->Flink;
        while (pHead != pEntry)
        {
            p = CONTAINING_RECORD(pEntry, __KEY_VALUE, __List);
            if (p->__Key.QuadKey.u.LowPart == pKV->__Key.QuadKey.u.LowPart
            &&
            p->__Key.QuadKey.u.HighPart == pKV->__Key.QuadKey.u.HighPart)
            {
                hr = E_SIGNAL_EXISTS;
                break;
            }
            pEntry = pEntry->Flink;
        }
        if (SUCCEEDED(hr))
        {
            DbgPrint("[TEMP1] transfer ack\n");
            hr = __SORA_HW_TX_TRANSFER_SIGNAL_EX(
                pCache->__TransferObj, &pCache->__TxMemPool, pSignal);
            if (SUCCEEDED(hr))
                InsertHeadList(pHead, &pKV->__List);   
        }
        KeReleaseSpinLock(&pCache->__HashTbl.LockTbl[Index], OldIrql);
    }while(FALSE);
    
    if (FAILED(hr)) //free occupied resource
    {
        if (pKV)
        {
            ExFreeToNPagedLookasideList(&pCache->__KeyValueList, pKV);
            pKV = NULL;
        }
        if (pSignal)
        {
            if (pSignal->pRMD)
            {
                SoraFreeRCBMemToPool(&pCache->__TxMemPool, pSignal->pRMD);
                pSignal->pRMD = NULL;
            }
            SoraFreeTxDesc(pCache->__pRadio->pTxResMgr, pSignal);
        }
    }

    return hr;
}
