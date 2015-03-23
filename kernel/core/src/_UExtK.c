#ifndef USER_MODE
#include "sora.h"
#include "resource_collection.h"
#include "thread_if.h"
#include "_scheduler.h"
#include "__reg_file.h"
#include "__radio_man_internal.h"

#define HWTEST_USERNAME     		L"HwtestMiniport"

#define RX_BUFFER_SIZE              (16 * 1024 * 1024)
#define MODULATE_BUFFER_SIZE        (2  * 1024 * 1024)

#define DoRelease(function, monitor, input, insize) {		\
															\
	DoRelease_##function(monitor, input, insize);			\
}

void SoraKUExtKeInit(IN PUEXT_KE UExtKeObj)
{
    SoraKTxQueueInit(&UExtKeObj->TxQueue);
}

void SoraKUExtKeCtor(PUEXT_KE  UExtKeObj)
{
	UExtKeObj->Monitor = AllocateProcessObjectMonitor();

	KeInitializeSpinLock(&UExtKeObj->SendPacketLock);
	UExtKeObj->SendPacketList = allocate_thread_safe_enlist();
	UExtKeObj->SendPacketControl = NULL;

    int i;
    for (i = 0; i < MAX_APP_EXT_NUM; i++)
    {
        UExtKeObj->RXBufMapMgr.BufferMDL[i]         = NULL;
        UExtKeObj->RXBufMapMgr.BufferUserSpaceVA[i] = NULL;
        UExtKeObj->RXBufMapMgr.UserProcess[i]       = NULL;
        UExtKeObj->TXBufMapMgr.BufferMDL[i]         = NULL;
        UExtKeObj->TXBufMapMgr.BufferUserSpaceVA[i] = NULL;
    }

    UExtKeObj->RXBufMapMgr.FreeBitMap   = 0xFFFFFFFF;//all 32 slots are free.
    UExtKeObj->TXBufMapMgr.FreeBitMap   = 0xFFFFFFFF;//all 32 slots are free.

    KeInitializeSpinLock(&UExtKeObj->RXBufMapMgr.BufferMapLock);
    KeInitializeSpinLock(&UExtKeObj->TXBufMapMgr.BufferMapLock);

    SoraKInitQueuedRWLock(&UExtKeObj->QueuedRxTxLock);
}

void __CleanQueue(IN  PTX_SIGNAL_QUEUE  TxQueue, PTRANSFER_OBJ pTransferObj)
{
    PLIST_ENTRY Entry;
    PTXCB Tcb ;
    do
    {
        Entry = NdisInterlockedRemoveHeadList(
            &TxQueue->CanTxQueue, &TxQueue->QueueLock);
        if (Entry == NULL)  break;
        Tcb = CONTAINING_RECORD(Entry, TXCB, TXCBList);
        //DEBUGP(MP_INFO, ("[EXIT] Clean NO. %d Cached Entry of %08x\n", Tcb->PacketID, Tcb));
        SoraPacketFreeTxResource(pTransferObj, &Tcb->Base);
        
        NdisFreeToNPagedLookasideList(
            &TxQueue->TxCBLookaside, Tcb);
    } while(TRUE);  
    
    return;
}
void SoraKTxQueueClean(IN  PTX_SIGNAL_QUEUE  TxQueue, PTRANSFER_OBJ pTransferObj)
{
	if (!TxQueue->Inited)
		return;
	
    __CleanQueue(TxQueue, pTransferObj);
    NdisFreeSpinLock(&TxQueue->QueueLock);
    NdisDeleteNPagedLookasideList(&TxQueue->TxCBLookaside);
}

void UExtKeCleanupBufMapMgr(PBUF_MAP_MGR  mgr);
void SoraKUExtKeDtor(PUEXT_KE  UExtKeObj, HANDLE TransferObj)
{
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;

	if (UExtKeObj->Monitor) {
		ReleaseProcessObjectMonitor(UExtKeObj->Monitor);
		UExtKeObj->Monitor = NULL;
	}

	if (UExtKeObj->SendPacketList) {
		release_thread_safe_enlist(UExtKeObj->SendPacketList);
		UExtKeObj->SendPacketList = NULL;
	}

	if (UExtKeObj->SendPacketControl) {
		ObDereferenceObject(UExtKeObj->SendPacketControl);
		UExtKeObj->SendPacketControl = NULL;
	}
	
    SoraKTxQueueClean(&UExtKeObj->TxQueue, pTransferObj);
    UExtKeCleanupBufMapMgr(&UExtKeObj->RXBufMapMgr);
    UExtKeCleanupBufMapMgr(&UExtKeObj->TXBufMapMgr);
}

void _UnmapBuffer(PVOID buf, PMDL mdl, HANDLE pid) {

	if (buf)
		if (PsGetCurrentProcessId() == pid)
			if (mdl)
				MmUnmapLockedPages(buf, mdl);
			
	if (mdl)
		IoFreeMdl(mdl);
}

#if 0
void _UnmapBuffer(PBUF_MAP_MGR  mgr, LONG i)
{
    if (mgr->BufferUserSpaceVA[i] != NULL)
    {
        MmUnmapLockedPages(mgr->BufferUserSpaceVA[i], mgr->BufferMDL[i]);
        mgr->BufferUserSpaceVA[i] = NULL;
        mgr->UserProcess[i] = NULL;
    }
    if (mgr->BufferMDL[i] != NULL)  
    {
        IoFreeMdl(mgr->BufferMDL[i]);
        mgr->BufferMDL[i] = NULL;
    }
}
#endif

void UExtKeCleanupBufMapMgr(PBUF_MAP_MGR  mgr)
{
    int i;
    for (i = 0; i < MAX_APP_EXT_NUM; i++)
    {
		KIRQL OldIrql;
		KeAcquireSpinLock(&mgr->BufferMapLock, &OldIrql);
		
    	PVOID buf;
		PMDL mdl;
		HANDLE pid;
		buf = mgr->BufferUserSpaceVA[i];
		mdl = mgr->BufferMDL[i];
		pid = mgr->UserProcess[i];
		mgr->BufferUserSpaceVA[i] = NULL;
		mgr->BufferMDL[i] = NULL;
		mgr->UserProcess[i] = NULL;
		
		KeReleaseSpinLock(&mgr->BufferMapLock, OldIrql);
		
        _UnmapBuffer(buf, mdl, pid);
    }    
}

LONG SoraKUExtKGetBufMapSlot(PBUF_MAP_MGR  mgr)
{
    KIRQL OldIrql;
    LONG ret = -1;
    KeAcquireSpinLock(&mgr->BufferMapLock, &OldIrql);
    if (mgr->FreeBitMap)
    {
        BitScanReverse((ULONG*)&ret, mgr->FreeBitMap);
        mgr->FreeBitMap &= ~(1 << ret);
    }
    KeReleaseSpinLock(&mgr->BufferMapLock, OldIrql);
    return ret;
}

VOID SoraKUExtKFreeBufMapSlot(PBUF_MAP_MGR  mgr, LONG index)
{    
    if (index >=0 && index < MAX_APP_EXT_NUM)
    {    
		KIRQL OldIrql;
    	PVOID buf;
		PMDL mdl;
		HANDLE pid;
		
		KeAcquireSpinLock(&mgr->BufferMapLock, &OldIrql);

		mgr->FreeBitMap |= (1 << index);
		
		buf = mgr->BufferUserSpaceVA[index];
		mdl = mgr->BufferMDL[index];
		pid = mgr->UserProcess[index];
		mgr->BufferUserSpaceVA[index] = NULL;
		mgr->BufferMDL[index] = NULL;
		mgr->UserProcess[index] = NULL;

		KeReleaseSpinLock(&mgr->BufferMapLock, OldIrql);
		
        _UnmapBuffer(buf, mdl, pid);
    }
}


HRESULT SoraKMapBuf2User(PBUF_MAP_MGR mgr, ULONG Slot, PVOID SysBuf, ULONG Size)
{
    HRESULT hr = S_OK;
    HANDLE hProc = PsGetCurrentProcessId();
    ASSERT(Slot < MAX_APP_EXT_NUM);
    do 
    {
        mgr->BufferMDL[Slot] = IoAllocateMdl(SysBuf, Size, FALSE, TRUE, NULL);
        if (mgr->BufferMDL[Slot] == NULL)
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }
        MmBuildMdlForNonPagedPool(mgr->BufferMDL[Slot]);
        
        mgr->BufferUserSpaceVA[Slot] = 
            MmMapLockedPagesSpecifyCache(mgr->BufferMDL[Slot], UserMode, MmCached, NULL, FALSE, NormalPagePriority);

        if (mgr->BufferUserSpaceVA[Slot] == NULL)
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }
        
        mgr->UserProcess[Slot] = hProc;
    } while(FALSE);
    
    if (FAILED(hr))
    {
        SoraKUExtKFreeBufMapSlot(mgr, Slot);
    }

    return hr;
}

void SoraKUExtKMapModBuffer(PUEXT_KE ExtObj, PTRANSFER_OBJ pTransferObj, PMAP_MOD_BUF_OUT Out)
{
    KIRQL OldIrql;
    LONG slot;
    HRESULT hr = S_OK;
    
    Out->ModBuf = NULL;
    Out->ModBufSize  = 0;
    
    do
    {
        slot = SoraKUExtKGetBufMapSlot(&ExtObj->TXBufMapMgr);
        if (slot < 0)
        {
            hr = E_BUF_MAP_EXCEEDED;
            break;
        }
        hr = SoraKMapBuf2User(
                &ExtObj->TXBufMapMgr, 
                slot, 
                pTransferObj->__TxSampleBufVa,
                pTransferObj->__TxSampleBufSize);
        FAILED_BREAK(hr);
        Out->hResult    = hr;
        Out->ModBuf     = ExtObj->TXBufMapMgr.BufferUserSpaceVA[slot];
        Out->ModBufSize = pTransferObj->__TxSampleBufSize;
    } while(FALSE);

    if (FAILED(hr))
    {
        Out->hResult    = hr;
        SoraKUExtKFreeBufMapSlot(&ExtObj->TXBufMapMgr, slot);
    }
}

HRESULT SoraKUExtKUnmapModBuffer(PUEXT_KE ExtObj, PTRANSFER_OBJ pTransferObj, PVOID UserBuffer)
{
    HANDLE hProc = PsGetCurrentProcessId();
    LONG i;
    for (i = 0; i < MAX_APP_EXT_NUM; i++)
    {
        if (ExtObj->TXBufMapMgr.BufferUserSpaceVA[i] == UserBuffer &&
            ExtObj->TXBufMapMgr.UserProcess[i] == hProc)
            break;
    }
    if (i < MAX_APP_EXT_NUM) //found
    {
        SoraKUExtKFreeBufMapSlot(&ExtObj->TXBufMapMgr, i);
        return S_OK;
    }
    else
        return E_FAIL;
}

HRESULT SoraKUExtKForceUnmapModBuffer(PUEXT_KE ExtObj, PSORA_RADIO Radio, PVOID UserBuffer)
{
    LONG i;
    for (i = 0; i < MAX_APP_EXT_NUM; i++)
    {
        if (ExtObj->TXBufMapMgr.BufferUserSpaceVA[i] == UserBuffer)
            break;
    }
    if (i < MAX_APP_EXT_NUM) //found
    {
        SoraKUExtKFreeBufMapSlot(&ExtObj->TXBufMapMgr, i);
        return S_OK;
    }
    else
        return E_FAIL;
}

void SoraKUExtKAllocVStreamMask(PUEXT_KE ExtObj, PSORA_RADIO Radio, PVSTREAM_MASK_ALLOC_OUT Output)
{
    ULONG VStreamMask = 0;
    HRESULT hr = S_OK;

    if (!SORA_RADIO_WELL_CONFIGED2(Radio))
    {
        Output->hResult = E_RADIO_NOT_CONFIGURED;
        return;
    }
    do
    {
        hr = SoraAllocateVStream(Radio, &VStreamMask);
        FAILED_BREAK(hr);

        DbgPrint("[UExtK] Vstream mask %d assigned\n", VStreamMask);
        Output->VStreamMask = VStreamMask;
    } while(FALSE);

    Output->hResult = hr;
    return;
}

HRESULT 
SoraKUExtKReleaseVStreamMask(
    PUEXT_KE ExtObj,
    PSORA_RADIO Radio, 
    ULONG VStreamMask)
{
    SoraFreeVStream(Radio, VStreamMask);
    return S_OK;
}

void SoraKUExtKMapRxBuf(PUEXT_KE ExtObj, PSORA_RADIO Radio, PMAP_RX_BUF_OUT Output)
{
    LONG slot;
    HRESULT hr = S_OK;

    if (!SORA_RADIO_WELL_CONFIGED2(Radio))
    {
        Output->hResult = E_RADIO_NOT_CONFIGURED;
        return;
    }
    do
    {
        slot = SoraKUExtKGetBufMapSlot(&ExtObj->RXBufMapMgr);
        if (slot < 0)
        {
            hr = E_BUF_MAP_EXCEEDED;
            break;
        }
        DbgPrint("[UExtK] slot %d allocated \n", slot);
        hr = SoraKMapBuf2User(
                &ExtObj->RXBufMapMgr, 
                slot, 
                SORA_GET_RX_DMA_BUFFER_VADDRESS(&Radio->__rx_queue), 
                SORA_GET_RX_DMA_BUFFER_SIZE(&Radio->__rx_queue));
        FAILED_BREAK(hr);
        DbgPrint("[UExtK] RX buffer mapped to %08x \n", ExtObj->RXBufMapMgr.BufferUserSpaceVA[slot]);

        Output->RxBuf = ExtObj->RXBufMapMgr.BufferUserSpaceVA[slot];
        Output->RxBufSize = SORA_GET_RX_DMA_BUFFER_SIZE(&Radio->__rx_queue);
    } while(FALSE);

    Output->hResult = hr;

    if (FAILED(hr))
    {
        if (slot >= 0) 
        {
            SoraKUExtKFreeBufMapSlot(&ExtObj->RXBufMapMgr, slot);
        }
    }
    
    return;
}

HRESULT 
SoraKUExtKForceUnmapRxBuf(
    PUEXT_KE ExtObj,
    PSORA_RADIO Radio, 
    PVOID UserBuffer)
{
    LONG i;
    for (i = 0; i < MAX_APP_EXT_NUM; i++)
    {
        if (ExtObj->RXBufMapMgr.BufferUserSpaceVA[i] == UserBuffer)
            break;
    }
    
    if (i < MAX_APP_EXT_NUM) //found
    {
        DbgPrint("[UExtK] rx map found at %d slot\n", i);
        SoraKUExtKFreeBufMapSlot(&ExtObj->RXBufMapMgr, i);
        return S_OK;
    }
    else
        return E_FAIL;

}

HRESULT 
SoraKUExtKUnmapRxBuf(
    PUEXT_KE ExtObj,
    PSORA_RADIO Radio, 
    PVOID UserBuffer)
{
    HANDLE hProc = PsGetCurrentProcessId();
    LONG i;
    for (i = 0; i < MAX_APP_EXT_NUM; i++)
    {
        if (ExtObj->RXBufMapMgr.BufferUserSpaceVA[i] == UserBuffer &&
            ExtObj->RXBufMapMgr.UserProcess[i] == hProc)
            break;
    }
    
    if (i < MAX_APP_EXT_NUM) //found
    {
        DbgPrint("[UExtK] rx map found at %d slot\n", i);
        SoraKUExtKFreeBufMapSlot(&ExtObj->RXBufMapMgr, i);
        return S_OK;
    }
    else
        return E_FAIL;

}

void free_alloc_kernel_buffer_data(struct ALLOC_KERNEL_BUFFER_DATA* alloc_kb) {

	if (alloc_kb->m_ubuf) {									
		if (alloc_kb->m_mdl)
			if (alloc_kb->m_map_process == PsGetCurrentProcessId())
				MmUnmapLockedPages(alloc_kb->m_ubuf,
					alloc_kb->m_mdl);
		alloc_kb->m_ubuf = NULL;
	}														
	if (alloc_kb->m_mdl) {									
		IoFreeMdl(alloc_kb->m_mdl);							
		alloc_kb->m_mdl = NULL;								
	}														
	if (alloc_kb->m_kbuf) {
		if (alloc_kb->m_is_physical_continous) {
			MmFreeContiguousMemory(alloc_kb->m_kbuf);
			alloc_kb->m_kbuf = NULL;
		}
		else {
			ExFreePool(alloc_kb->m_kbuf);
			alloc_kb->m_kbuf = NULL;
		}
	}
}

void free_alloc_kernel_buffer_resource_object_data(
	PVOID context,	
	struct RESOURCE_OBJECT_DATA* rdata) {

	free_alloc_kernel_buffer_data(&rdata->m_associated.m_alloc_kernel_buffer_data);
}

HRESULT 
SoraKUExtKAllocKernelBuffer(
	PUEXT_KE ExtObj,
	ULONG Size,
	BOOLEAN force_Continuous,
	struct ALLOC_KERNEL_BUFFER_DATA* alloc_kb) {

	PHYSICAL_ADDRESS low;
	PHYSICAL_ADDRESS high;
	PHYSICAL_ADDRESS mul;
	low.QuadPart = 0;
	high.u.LowPart = 0x80000000;
	high.u.HighPart = 0;
	mul.QuadPart = 0;

	RtlZeroMemory(alloc_kb,
		sizeof(struct ALLOC_KERNEL_BUFFER_DATA));

	alloc_kb->m_size = Size;
	alloc_kb->m_kbuf = MmAllocateContiguousMemorySpecifyCache(alloc_kb->m_size,
		low,
		high,
		mul,
		MmWriteCombined);
	if (alloc_kb->m_kbuf)
		alloc_kb->m_is_physical_continous = 1;
	else
	if (!force_Continuous)
		alloc_kb->m_kbuf = ExAllocatePoolWithTag(NonPagedPool, 
			alloc_kb->m_size,
			'resu');
	
	if (alloc_kb->m_kbuf) {
		alloc_kb->m_mdl = IoAllocateMdl(alloc_kb->m_kbuf, 
			alloc_kb->m_size,
			FALSE,
			FALSE,
			FALSE);
		if (alloc_kb->m_mdl) {
			MmBuildMdlForNonPagedPool(alloc_kb->m_mdl);
			
			alloc_kb->m_ubuf = MmMapLockedPagesSpecifyCache(alloc_kb->m_mdl,
				UserMode,
				MmWriteCombined,
				NULL, 
				FALSE,
				NormalPagePriority);
			if (alloc_kb->m_ubuf) {
				alloc_kb->m_map_process = PsGetCurrentProcessId();
				return S_OK;
			}
		}
	}
	free_alloc_kernel_buffer_data(alloc_kb);
	return E_FAIL;
}

ULONG 
SoraKHandleUExtReq(
    PUEXT_KE UExtKeObj,
    HANDLE TransferObj,
    PSORA_RADIO Radios2[MAX_RADIO_NUMBER], // numbers of this pointer should always be MAX_RADIO_NUMBER
    ULONG code,
    PVOID Input, 
    ULONG InSize, 
    PVOID Output, 
    ULONG OutSize)
{
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
    ULONG ret = 0;

    switch (code)
    {
    case UEXT_CMD(MAP_MOD_BUFFER):
        if (OutSize == sizeof(MAP_MOD_BUF_OUT) && InSize == sizeof(ULONG)) //
        {
            SoraKUExtKMapModBuffer(UExtKeObj, pTransferObj, (PMAP_MOD_BUF_OUT)Output);
            DbgPrint("[UExtK] mapping modulate buffer to %08x\n", ((PMAP_MOD_BUF_OUT)Output)->ModBuf);
            ret = sizeof(MAP_MOD_BUF_OUT);
			if (((PMAP_MOD_BUF_OUT)Output)->hResult == S_OK)
				DoCollect(UExtKeObj->Monitor, UExtKeObj, pTransferObj, NULL, code, Output, sizeof(MAP_MOD_BUF_OUT));
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(UNMAP_MOD_BUFFER):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(UNMAP_MOD_BUF_IN))
        {
            *((HRESULT*)Output) = SoraKUExtKUnmapModBuffer(UExtKeObj, pTransferObj, ((PUNMAP_MOD_BUF_IN)Input)->ModBuf);
            DbgPrint("[UExtK] unmapping modulate buffer %08x: %08x\n", 
                ((PUNMAP_MOD_BUF_IN)Input)->ModBuf, *((HRESULT*)Output));
            ret = sizeof(HRESULT);
			if (*((HRESULT*)Output) == S_OK)
				DoRelease(UNMAP_MOD_BUFFER, UExtKeObj->Monitor, Input, sizeof(UNMAP_MOD_BUF_IN));
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;    
    case UEXT_CMD(TX_RES_ALLOC):
        if (OutSize == sizeof(TX_RES_ALLOC_OUT) && InSize == sizeof(TX_RES_ALLOC_IN))
        {
            SoraKUExtKTxResAlloc(pTransferObj, &UExtKeObj->TxQueue, 
                ((PTX_RES_ALLOC_IN)Input)->SampleSize, (PTX_RES_ALLOC_OUT)Output);
            ret = sizeof(TX_RES_ALLOC_OUT);
			if (((PTX_RES_ALLOC_OUT)Output)->hResult == S_OK)
				DoCollect(UExtKeObj->Monitor, UExtKeObj, pTransferObj, NULL, code, Output, sizeof(TX_RES_ALLOC_OUT));
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(TX_SIGNAL):
        if (InSize == sizeof(TX_IN) && OutSize == sizeof(HRESULT))
        {
			PTX_IN tx_in = (PTX_IN)Input;
			if (tx_in->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[tx_in->RadioNo])
					*((HRESULT*)Output) = SoraKTxSignalByID(&UExtKeObj->TxQueue, Radios2[tx_in->RadioNo], ((PTX_IN)Input)->TxID, 1);
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else 
				*((HRESULT*)Output) = E_INVALID_PARAMETER;
		    
            ret = sizeof(HRESULT);
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(TX_RES_RELEASE):
        if (InSize == sizeof(TX_RES_REL_IN) && OutSize == sizeof(HRESULT))
        {
            *((HRESULT*)Output) = SoraKTxDoneByID(&UExtKeObj->TxQueue, pTransferObj, ((PTX_RES_REL_IN)Input)->TxID);
            ret = sizeof(HRESULT);
			if (*((HRESULT*)Output) == S_OK)
				DoRelease(TX_RES_RELEASE, UExtKeObj->Monitor, Input, sizeof(TX_RES_REL_IN));
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(VSTREAM_MASK_ALLOC):
        if (OutSize == sizeof(VSTREAM_MASK_ALLOC_OUT) && InSize == sizeof(VSTREAM_MASK_ALLOC_IN))
        {
	        PVSTREAM_MASK_ALLOC_IN vstream_mask_alloc_in = (PVSTREAM_MASK_ALLOC_IN)Input;
			ULONG RadioNo = vstream_mask_alloc_in->RadioNo;
			if (RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[RadioNo])
					SoraKUExtKAllocVStreamMask(UExtKeObj, Radios2[RadioNo], (PVSTREAM_MASK_ALLOC_OUT)Output);
				else 
					((PVSTREAM_MASK_ALLOC_OUT)Output)->hResult = E_RADIO_NOT_CONFIGURED;
			else 
				((PVSTREAM_MASK_ALLOC_OUT)Output)->hResult = E_INVALID_PARAMETER;
			
			ret = sizeof(VSTREAM_MASK_ALLOC_OUT);
			
			if (((PVSTREAM_MASK_ALLOC_OUT)Output)->hResult == S_OK)
				DoCollect(UExtKeObj->Monitor, UExtKeObj, pTransferObj, Radios2[RadioNo], code, Output, sizeof(VSTREAM_MASK_ALLOC_OUT));
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(VSTREAM_MASK_RELEASE):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(VSTREAM_MASK_RELEASE_IN))
        {			
			PVSTREAM_MASK_RELEASE_IN vstream_mask_release_in = (PVSTREAM_MASK_RELEASE_IN)Input;
			if (vstream_mask_release_in->RadioNo < MAX_RADIO_NUMBER) 
				if (Radios2[vstream_mask_release_in->RadioNo])					
		            *((HRESULT*)Output) = SoraKUExtKReleaseVStreamMask(
		                                    UExtKeObj,
		                                    Radios2[vstream_mask_release_in->RadioNo],
		                                    ((PVSTREAM_MASK_RELEASE_IN)Input)->VStreamMask);
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;
			
            ret = sizeof(HRESULT);
			
			if (*((HRESULT*)Output) == S_OK)
				DoRelease(VSTREAM_MASK_RELEASE, UExtKeObj->Monitor, Input, sizeof(VSTREAM_MASK_RELEASE_IN));
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(MAP_RX_BUFFER):
        if (OutSize == sizeof(MAP_RX_BUF_OUT) && InSize == sizeof(ULONG))
        {
			ULONG RadioNo = *(ULONG*)Input;
			if (RadioNo < MAX_RADIO_NUMBER) 
				if (Radios2[RadioNo])
					SoraKUExtKMapRxBuf(UExtKeObj, Radios2[RadioNo], (PMAP_RX_BUF_OUT)Output);
				else
					((PMAP_RX_BUF_OUT)Output)->hResult = E_RADIO_NOT_CONFIGURED;
			else
				((PMAP_RX_BUF_OUT)Output)->hResult = E_INVALID_PARAMETER;
			            
            ret = sizeof(MAP_RX_BUF_OUT);
			
			if (((PMAP_RX_BUF_OUT)Output)->hResult == S_OK)
				DoCollect(UExtKeObj->Monitor, UExtKeObj, pTransferObj, Radios2[RadioNo], code, Output, sizeof(MAP_RX_BUF_OUT));		
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(UNMAP_RX_BUFFER):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(UNMAP_RX_BUF_IN))
        {
			PUNMAP_RX_BUF_IN unmap_rx_buf_in = (PUNMAP_RX_BUF_IN)Input;
			if (unmap_rx_buf_in->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[unmap_rx_buf_in->RadioNo])
					*((HRESULT*)Output) = SoraKUExtKUnmapRxBuf(
											UExtKeObj,
											Radios2[unmap_rx_buf_in->RadioNo], 
											((PUNMAP_RX_BUF_IN)Input)->UserBuffer);
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;
			
            ret = sizeof(HRESULT);
			
			if (*((HRESULT*)Output) == S_OK)
				DoRelease(UNMAP_RX_BUFFER, UExtKeObj->Monitor, Input, sizeof(UNMAP_RX_BUF_IN));
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(RX_LOCK_ACQUIRE):
        if (OutSize == sizeof(HRESULT))
        {
            *((HRESULT*)Output) = SoraKAcquireRXLock(&UExtKeObj->QueuedRxTxLock);
            ret = sizeof(HRESULT);
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(TX_LOCK_ACQUIRE):
        if (OutSize == sizeof(HRESULT))
        {   
            *((HRESULT*)Output) = SoraKAcquireTXLock(&UExtKeObj->QueuedRxTxLock);
            ret = sizeof(HRESULT);
        }
        else
        {
            DbgPrint("[UExtK] illegal IO control\n");
        }
        break;
    case UEXT_CMD(RX_LOCK_RELEASE):
        DbgPrint("SoraKHandleUExtReq: RX_LOCK_RELEASE\n");
        SoraKReleaseRXLock(&UExtKeObj->QueuedRxTxLock); 
        break;
    case UEXT_CMD(TX_LOCK_RELEASE):
        SoraKReleaseTXLock(&UExtKeObj->QueuedRxTxLock); 
        break;

    case UEXT_CMD(SET_CEN_FREQ):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(SET_VALUE_IN))
        {
            DbgPrint("[UExtK]Set central freq: %d kHz\n", ((PSET_VALUE_IN)Input)->u.uValue);
			PSET_VALUE_IN set_value_in = (PSET_VALUE_IN)Input;
			if (set_value_in->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[set_value_in->RadioNo]) {
					SoraHwSetCentralFreq(Radios2[set_value_in->RadioNo], ((PSET_VALUE_IN)Input)->u.uValue,  0);
					*((HRESULT*)Output) = S_OK;
				}
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;
                        
            ret = sizeof(HRESULT);
        }
        break;
    case UEXT_CMD(SET_FREQ_OFF):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(SET_VALUE_IN))
        {
            DbgPrint("[UExtK]Set freq offset: %d Hz\n", ((PSET_VALUE_IN)Input)->u.iValue);
			PSET_VALUE_IN set_value_in = (PSET_VALUE_IN)Input;
			if (set_value_in->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[set_value_in->RadioNo]) {
					SoraHwSetFreqCompensation(Radios2[set_value_in->RadioNo], ((PSET_VALUE_IN)Input)->u.uValue);
					*((HRESULT*)Output) = S_OK;
				}
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;

            ret = sizeof(HRESULT);
        }
        break;
    case UEXT_CMD(SET_RX_PA):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(SET_VALUE_IN))
        {
            DbgPrint("[UExtK]Set RX gain level: 0x%04x\n", ((PSET_VALUE_IN)Input)->u.uValue);
			PSET_VALUE_IN set_value_in = (PSET_VALUE_IN)Input;
			if (set_value_in->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[set_value_in->RadioNo]) {
					SoraHwSetRXPA(Radios2[set_value_in->RadioNo], ((PSET_VALUE_IN)Input)->u.uValue);
					*((HRESULT*)Output) = S_OK;
				}
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;

            ret = sizeof(HRESULT);
		}
        break;
    case UEXT_CMD(SET_RX_GAIN):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(SET_VALUE_IN))
        {
            DbgPrint("[UExtK]Set RX gain: 0x%04x\n", ((PSET_VALUE_IN)Input)->u.uValue);
			PSET_VALUE_IN set_value_in = (PSET_VALUE_IN)Input;
			if (set_value_in->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[set_value_in->RadioNo]) {
					SoraHwSetRXVGA1(Radios2[set_value_in->RadioNo], ((PSET_VALUE_IN)Input)->u.uValue);
					*((HRESULT*)Output) = S_OK;
				}
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;

            ret = sizeof(HRESULT);			
        }
        break;
    case UEXT_CMD(SET_TX_GAIN):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(SET_VALUE_IN))
        {
            DbgPrint("[UExtK]Set TX gain: 0x%04x\n", ((PSET_VALUE_IN)Input)->u.uValue);
			PSET_VALUE_IN set_value_in = (PSET_VALUE_IN)Input;
			if (set_value_in->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[set_value_in->RadioNo]) {
					SoraHwSetTXVGA1(Radios2[set_value_in->RadioNo], ((PSET_VALUE_IN)Input)->u.uValue);
					*((HRESULT*)Output) = S_OK;
				}
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;

            ret = sizeof(HRESULT);			
        }
        break;
    case UEXT_CMD(SET_SAMPLE_RATE):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(SET_VALUE_IN))
        {
            DbgPrint("[UExtK]Set Sample rate: %d MHz\n", ((PSET_VALUE_IN)Input)->u.uValue);
			PSET_VALUE_IN set_value_in = (PSET_VALUE_IN)Input;
			if (set_value_in->RadioNo < MAX_RADIO_NUMBER)
				if (Radios2[set_value_in->RadioNo]) {
					SoraHwSetSampleClock(Radios2[set_value_in->RadioNo], ((PSET_VALUE_IN)Input)->u.uValue * 1000 * 1000);
					*((HRESULT*)Output) = S_OK;
				}
				else
					*((HRESULT*)Output) = E_RADIO_NOT_CONFIGURED;
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;

            ret = sizeof(HRESULT);			
        }
        break;
    case UEXT_CMD(RADIO_START):
        if (OutSize == sizeof(HRESULT) && InSize == sizeof(RADIO_START_IN))
        {
            DbgPrint("[UExtK]Physical Radio starting\n");
			PRADIO_START_IN radio_start_in = (PRADIO_START_IN)Input;
			if (radio_start_in->RadioNo < MAX_RADIO_NUMBER) 
				if (Radios2[radio_start_in->RadioNo]) {
					HRESULT hr;
					hr = SoraRadioStart(Radios2[radio_start_in->RadioNo], SORA_RADIO_DEFAULT_RX_GAIN, SORA_RADIO_DEFAULT_TX_GAIN, NULL);
					if (hr == S_OK)
						SORA_HW_ENABLE_RX(Radios2[radio_start_in->RadioNo]);
					*((HRESULT*)Output) = hr;
				}
				else {
					NTSTATUS Status;
					LIST_ENTRY radio_list;
					Status = E_RADIO_NOT_CONFIGURED;
					InitializeListHead(&radio_list);
					
					do {						
						Status = SoraAllocateRadioFromRCBDevice(
							&radio_list,
							1 << radio_start_in->RadioNo,
							HWTEST_USERNAME);

						if (!IsListEmpty(&radio_list)) {
							PSORA_RADIO radio = CONTAINING_RECORD(radio_list.Flink, SORA_RADIO, RadiosList);

							if (!radio->__initialized) {
								Status = SoraRadioInitialize(
											radio,
											NULL, //reserved
											MODULATE_BUFFER_SIZE,
											RX_BUFFER_SIZE);
								if (Status != S_OK)
									break;
								
								radio->__initialized = TRUE;
							}

							Status = SoraRadioStart(radio, SORA_RADIO_DEFAULT_RX_GAIN, SORA_RADIO_DEFAULT_TX_GAIN, NULL);
							if (Status != S_OK)
								break;
							
							SORA_HW_ENABLE_RX(radio);
							
							Radios2[radio_start_in->RadioNo] = radio;
						}
					} while(0);					
					
					if (Status != S_OK) {
						if (!IsListEmpty(&radio_list)) {
							PSORA_RADIO radio = CONTAINING_RECORD(radio_list.Flink, SORA_RADIO, RadiosList);							
							SoraReleaseRadios(&radio_list);
						}
						Radios2[radio_start_in->RadioNo] = NULL;
					}					
					*((HRESULT*)Output) = Status;

				}
			else
				*((HRESULT*)Output) = E_INVALID_PARAMETER;
			
            ret = sizeof(HRESULT);
        }
        break;
	case UEXT_CMD(WRITE_RADIO_REG): {
			if (InSize  >= sizeof(RADIO_REGISTER_IO) &&
				OutSize >= sizeof(RADIO_REGISTER_IO)) {				
				extern VOID _SoraRadioWriteRFReg(PSORA_RADIO pRadio, s_uint32 addr, s_uint32 value);				
				RADIO_REGISTER_IO* regio_out;
				RADIO_REGISTER_IO* regio_in;
				regio_out = (RADIO_REGISTER_IO*)Output;
				regio_in = (RADIO_REGISTER_IO*)Input;				
				if (regio_in->radio < MAX_RADIO_NUMBER)
					if (Radios2[regio_in->radio]) {
						_SoraRadioWriteRFReg(Radios2[regio_in->radio], regio_in->addr, regio_in->value.in);
						regio_out->hr = S_OK;
					}
					else 
						regio_out->hr = E_RADIO_NOT_CONFIGURED;
				else
					regio_out->hr = E_INVALID_PARAMETER;
								
				ret = sizeof(RADIO_REGISTER_IO);
			}
		}
		break;
	case UEXT_CMD(READ_RADIO_REG): {
			if (InSize	>= sizeof(RADIO_REGISTER_IO) &&
				OutSize >= sizeof(RADIO_REGISTER_IO)) {
				extern HRESULT _SoraRadioReadRFReg(PSORA_RADIO pRadio, s_uint32 addr, s_uint32* value);
				RADIO_REGISTER_IO* regio_out;
				RADIO_REGISTER_IO* regio_in;
				regio_out = (RADIO_REGISTER_IO*)Output;
				regio_in = (RADIO_REGISTER_IO*)Input;
				if (regio_in->radio < MAX_RADIO_NUMBER)
					if (Radios2[regio_in->radio])
						regio_out->hr = _SoraRadioReadRFReg(Radios2[regio_in->radio], regio_in->addr, &regio_out->value.out);
					else
						regio_out->hr = E_RADIO_NOT_CONFIGURED;
				else
					regio_out->hr = E_INVALID_PARAMETER;
				
				ret = sizeof(RADIO_REGISTER_IO);
			}
		}
		break;
	case UEXT_CMD(ALLOC_KERNEL_BUFFER): {
			if (InSize >= sizeof(ALLOC_KERNEL_BUFFER_IN) &&
				OutSize >= sizeof(ALLOC_KERNEL_BUFFER_OUT)) {
				PALLOC_KERNEL_BUFFER_IN alloc_kb_in;
				PALLOC_KERNEL_BUFFER_OUT alloc_kb_out;
				alloc_kb_in = (PALLOC_KERNEL_BUFFER_IN)Input;
				alloc_kb_out = (PALLOC_KERNEL_BUFFER_OUT)Output;
				if (alloc_kb_in->Size) {					
					struct ALLOC_KERNEL_BUFFER_DATA alloc_kb;
					if (SoraKUExtKAllocKernelBuffer(UExtKeObj,
						alloc_kb_in->Size,
						alloc_kb_in->force_Continuous,
						&alloc_kb) != S_OK) {
						alloc_kb_out->Buff = NULL;
						alloc_kb_out->HiPhyBuff = 0;
						alloc_kb_out->LoPhyBuff = 0;
						alloc_kb_out->is_Continuous = FALSE;
						ret = sizeof(ALLOC_KERNEL_BUFFER_OUT);
					}
					else {
						PHYSICAL_ADDRESS phy_addr;
						phy_addr = MmGetPhysicalAddress(alloc_kb.m_kbuf);
						alloc_kb_out->Buff = alloc_kb.m_ubuf;
						alloc_kb_out->HiPhyBuff = phy_addr.HighPart;
						alloc_kb_out->LoPhyBuff = phy_addr.LowPart;
						alloc_kb_out->is_Continuous = alloc_kb.m_is_physical_continous;
						ret = sizeof(ALLOC_KERNEL_BUFFER_OUT);
						DoCollectEx(UExtKeObj->Monitor,
							UExtKeObj,
							NULL,
							code,
							&alloc_kb,
							sizeof(struct ALLOC_KERNEL_BUFFER_DATA),
							free_alloc_kernel_buffer_resource_object_data,
							NULL, 
							FALSE);
					}
				}
			}
		}
		break;
	case UEXT_CMD(RELEASE_KERNEL_BUFFER): {
			if (InSize >= sizeof(RELEASE_KERNEL_BUFFER_IN)) {
				struct RESOURCE_OBJECT_DATA* rdata;
				KIRQL irql;
				KeAcquireSpinLock(&UExtKeObj->Monitor->m_monitor_lock, &irql);
				rdata = Get_RELEASE_KERNEL_BUFFER(UExtKeObj->Monitor,
					Input,
					InSize);
				if (rdata)
					RemoveEntryList(&rdata->m_resource_entry);
				KeReleaseSpinLock(&UExtKeObj->Monitor->m_monitor_lock, irql);
				
				if (rdata) {
					if (rdata->m_free_routine)
						rdata->m_free_routine(rdata->m_free_context, 
							rdata);
					ExFreePool(rdata);
				}
			}
		}
		break;		
	case UEXT_CMD(REDIRECT_IRP_MJ_CREATE): {
			KIRQL irql;
			KeAcquireSpinLock(&UExtKeObj->Monitor->m_monitor_lock, &irql);
			{
				struct PROCESS_OBJECT_DATA* pdata;
				pdata = GetProcessObjectDataByProcessId(&UExtKeObj->Monitor->m_process_list, PsGetCurrentProcessId());
				if (pdata)
					InterlockedIncrement(&pdata->m_reference);
				else {
					pdata = AllocateProcessObjectData();
					pdata->m_process_id = PsGetCurrentProcessId();
					pdata->m_process_object = PsGetCurrentProcess();						
					ObReferenceObject(pdata->m_process_object);
					InsertTailList(&UExtKeObj->Monitor->m_process_list.m_process_head, &pdata->m_process_entry);
					KeSetEvent(&UExtKeObj->Monitor->m_control,
						IO_NO_INCREMENT,
						FALSE);
				}
			}
			KeReleaseSpinLock(&UExtKeObj->Monitor->m_monitor_lock, irql);			
		}
		break;
	case UEXT_CMD(REDIRECT_IRP_MJ_CLOSE): {
			KIRQL irql;
			KeAcquireSpinLock(&UExtKeObj->Monitor->m_monitor_lock, &irql);
			{
				struct PROCESS_OBJECT_DATA* pdata;
				pdata = GetProcessObjectDataByProcessId(&UExtKeObj->Monitor->m_process_list, PsGetCurrentProcessId());
				if (pdata)
					switch(InterlockedDecrement(&pdata->m_reference)) {
					case 0:
						KeSetEvent(&UExtKeObj->Monitor->m_control,
							IO_NO_INCREMENT,
							FALSE);
						break;
					default:
						break;
					}
			}
			KeReleaseSpinLock(&UExtKeObj->Monitor->m_monitor_lock, irql);
		}
		break;
	default:
        DbgPrint("[UExtK] illegal IO control\n");
        break;
    }
    return ret;
}

NTSTATUS
SoraKAllocateTransferObj(
	HANDLE* TransferObj) {

	NTSTATUS status = STATUS_SUCCESS;
	PTRANSFER_OBJ transfer_obj = NULL;
	transfer_obj = (PTRANSFER_OBJ)ExAllocatePoolWithTag(NonPagedPool, sizeof(TRANSFER_OBJ), 'jbot');
	if (!transfer_obj) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto error_exit;
	}
	RtlZeroMemory(transfer_obj, sizeof(TRANSFER_OBJ));

	status = SoraAcquireRadioManager(&transfer_obj->RadioManager);
	if (status != STATUS_SUCCESS)
		goto error_exit;

	transfer_obj->TransferResMgr = &transfer_obj->RadioManager->__TX_RESMGR;
	KeInitializeSpinLock(&transfer_obj->TransferLock);
	transfer_obj->TransferReg = transfer_obj->RadioManager->__RegsBase->RadioRegs;
	transfer_obj->SoraSysReg = transfer_obj->RadioManager->__RegsBase;
	transfer_obj->SoraHWOpRadio = &transfer_obj->RadioManager->__radio_pool[0];

	{			
		PHYSICAL_ADDRESS	PhysicalAddress 	 = {0, 0}; 
		PHYSICAL_ADDRESS	PhysicalAddressLow	 = {0, 0};
		PHYSICAL_ADDRESS	PhysicalAddressHigh  = {0x80000000, 0};
		HRESULT hr = E_NOT_ENOUGH_CONTINUOUS_PHY_MEM;
		ULONG SampleBufferSize = MODULATE_BUFFER_SIZE;
		
		ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(NULL, SampleBufferSize);

		transfer_obj->__TxBufLock = 0;

		if (!transfer_obj->__TxSampleBufVa)
		{
			transfer_obj->__TxSampleBufVa = 
					(PCOMPLEX8) MmAllocateContiguousMemorySpecifyCache(
								SampleBufferSize, 
								PhysicalAddressLow,
								PhysicalAddressHigh,
								PhysicalAddress,
								MmWriteCombined);
			if (transfer_obj->__TxSampleBufVa)
			{
				transfer_obj->__TxSampleBufPa	 = MmGetPhysicalAddress(transfer_obj->__TxSampleBufVa);
				ASSERT(transfer_obj->__TxSampleBufPa.u.LowPart % 0x00001000 == 0); //should always 4K aligned
				transfer_obj->__TxSampleBufSize  = SampleBufferSize;
			}
			else {
				status = STATUS_INSUFFICIENT_RESOURCES;
				goto error_exit;
			}
		}
	}
	*TransferObj = (HANDLE)transfer_obj;
	
	return status;

error_exit:

	if (transfer_obj) {
		if (transfer_obj->__TxSampleBufVa) {
			MmFreeContiguousMemorySpecifyCache(
				transfer_obj->__TxSampleBufVa, 				
				transfer_obj->__TxSampleBufSize,				
				MmWriteCombined);			
			transfer_obj->__TxSampleBufVa    = NULL;			
			transfer_obj->__TxSampleBufSize  = 0;		
		}
		
		if (transfer_obj->RadioManager) {
			SoraReleaseRadioManager();
			transfer_obj->RadioManager = NULL;
		}		
		ExFreePoolWithTag(transfer_obj, 'jbot');
	}
	*TransferObj = NULL;
	
	return status;

}

void
SoraKFreeTransferObj(
	HANDLE TransferObj) {

	PTRANSFER_OBJ transfer_obj = (PTRANSFER_OBJ)TransferObj;

	if (transfer_obj) {
		if (transfer_obj->__TxSampleBufVa) {
			MmFreeContiguousMemorySpecifyCache(
				transfer_obj->__TxSampleBufVa,				
				transfer_obj->__TxSampleBufSize,				
				MmWriteCombined);			
			transfer_obj->__TxSampleBufVa	 = NULL;			
			transfer_obj->__TxSampleBufSize  = 0;		
		}
		
		if (transfer_obj->RadioManager) {
			SoraReleaseRadioManager();
			transfer_obj->RadioManager = NULL;
		}		
		ExFreePoolWithTag(transfer_obj, 'jbot');
	}	
}

#endif 
