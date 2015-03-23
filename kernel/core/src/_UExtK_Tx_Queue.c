#include "sora.h"
#include "__tx_res.h"
#include "__transfer_obj.h"

#ifndef USER_MODE

ULONG AssignTxID(PTX_SIGNAL_QUEUE TxQueue)
{
    ULONG TxID;
    NdisAcquireSpinLock(&TxQueue->QueueLock);
    TxID = TxQueue->IDSeqCtrl++;
    NdisReleaseSpinLock(&TxQueue->QueueLock);
    return TxID;
}

PTXCB GetTxCBByID(PTX_SIGNAL_QUEUE TxQueue, ULONG sid)
{
    PLIST_ENTRY Entry;
    PTXCB       Tcb = NULL;

    DbgPrint("[UExtK] search NO. %d signal\n", sid);
    NdisAcquireSpinLock(&TxQueue->QueueLock);
    FOREACH(&TxQueue->CanTxQueue, Entry)
    {
        Tcb = CONTAINING_RECORD(Entry, TXCB, TXCBList);
        if (Tcb->PacketID != sid)
            Tcb = NULL;
        else
            break;
    }
    NdisReleaseSpinLock(&TxQueue->QueueLock);
    return Tcb;
}

PTXCB RemoveTxCBByID(PTX_SIGNAL_QUEUE TxQueue, ULONG sid)
{
    PLIST_ENTRY Entry;
    PTXCB       Tcb = NULL;
    DbgPrint("[UExtK] removing NO. %d signal\n", sid);
    NdisAcquireSpinLock(&TxQueue->QueueLock);
    FOREACH(&TxQueue->CanTxQueue, Entry)
    {
        Tcb = CONTAINING_RECORD(Entry, TXCB, TXCBList);
        if (Tcb->PacketID != sid)
            Tcb = NULL;
        else
        {
            RemoveEntryList(&Tcb->TXCBList);
            break;
        }
    }
    NdisReleaseSpinLock(&TxQueue->QueueLock);
    return Tcb;
}

void SoraKTxQueueInit(IN  PTX_SIGNAL_QUEUE  TxQueue)
{
    NdisAllocateSpinLock(&TxQueue->QueueLock);
    InitializeListHead(&TxQueue->CanTxQueue);
    TxQueue->IDSeqCtrl = 0;

    TxQueue->fStopTX = 0;

    NdisInitializeNPagedLookasideList(
        &TxQueue->TxCBLookaside, NULL, NULL, 0, sizeof(TXCB), __SORA_TAG, 0);

	TxQueue->Inited= 1;
}

HRESULT 
SoraKTxSignalByID(
    PTX_SIGNAL_QUEUE TxQueue, 
    PSORA_RADIO Radio, 
    ULONG sid, 
    ULONG Times)
{
    HRESULT hr;
    PTXCB       Tcb;
    ULONG       i;
    
    Tcb = GetTxCBByID(TxQueue, sid);
    if (Tcb == NULL)
    {
        hr = E_UNKOWN_ID;
    }
    else
    {
        for (i = 0; i < Times; i++)
        {
            hr = SORA_HW_BEGIN_TX(Radio, &Tcb->Base);
            FAILED_BREAK(hr);
            if (TxQueue->fStopTX) 
            {
                TxQueue->fStopTX = 0; 
                break;
            }
        }
    }
    
    return hr;
}

HRESULT 
SoraKMimoTxSignalByID(
    PTX_SIGNAL_QUEUE TxQueue, 
    PSORA_RADIO Radios2[MAX_RADIO_NUMBER], 
    ULONG* radio,
    ULONG* sid, 
    ULONG count,
    ULONG Times)
{
	ULONG mask = 0;
	ULONG i;
	ULONG t;
	LONG r;
	PTXCB tcb[MAX_RADIO_NUMBER];
	KIRQL irql[MAX_RADIO_NUMBER];	
	HRESULT hr;
	
	for(i=0; i < count; i++) {
		if (radio[i] < MAX_RADIO_NUMBER) {
			if (Radios2[radio[i]]) {
				ULONG id;
				id = 1 << radio[i];
				if (mask & id)
					return E_INVALID_PARAMETER;
				mask |= id;
				tcb[i] = GetTxCBByID(TxQueue, sid[i]);
				if (tcb[i])
					continue;					
				else
					return E_UNKOWN_ID;
			}
			else
				return E_RADIO_NOT_CONFIGURED;
		}
		else
			return E_INVALID_PARAMETER;
	}

	for (t = 0; t < Times; t++) {
		for(i=0; i < count; i++) {
			KeAcquireSpinLock(&Radios2[radio[i]]->__HWOpLock, &irql[i]);
			SoraHwSetTxBufferRegs(Radios2[radio[i]], &tcb[i]->Base);
		}
        hr = SoraHwSyncTx(Radios2[radio[0]], mask);
		
		for(r=count-1; r >= 0; r--)
			KeReleaseSpinLock(&Radios2[radio[r]]->__HWOpLock, irql[r]);

		if (hr != S_OK)
			break;
		
        if (TxQueue->fStopTX) {
            TxQueue->fStopTX = 0; 
            break;
        }		
    }
	return hr;
}


HRESULT SoraKTxDoneByID(PTX_SIGNAL_QUEUE TxQueue, HANDLE TransferObj, ULONG sid)
{
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
    HRESULT hr = S_OK;
    PTXCB       Tcb;
    
    Tcb = RemoveTxCBByID(TxQueue, sid);
    if (Tcb == NULL)
        hr = E_UNKOWN_ID;
    else
    {
        DbgPrint("[UExtK] removed %d Tx packet\n", sid);
        SoraPacketFreeTxResource(pTransferObj, &Tcb->Base);
        NdisFreeToNPagedLookasideList(&TxQueue->TxCBLookaside, Tcb);
    }
    
    return hr;
}

void 
SoraKUExtKTxResAlloc(
    HANDLE TransferObj,
    PTX_SIGNAL_QUEUE TxQueue,
    ULONG SampleSize, 
    OUT PTX_RES_ALLOC_OUT Output)
{
	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
    PTXCB Tcb = NULL;
    HRESULT hr = S_OK;
	
    do
    {
        Tcb = (PTXCB) NdisAllocateFromNPagedLookasideList(&TxQueue->TxCBLookaside);
        if (Tcb == NULL) 
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }
        DbgPrint("[UExtK] TxCB allocated\n");
        SoraPacketInitialize(&Tcb->Base);
        hr = SoraPacketGetTxResource(pTransferObj, &Tcb->Base);
        DbgPrint("[UExtK] Tx resource allocated\n");
        FAILED_BREAK(hr);
        
        SoraPacketSetSignalLength(&Tcb->Base, SampleSize);
        DbgPrint("[UExtK] Tx size %d\n", SampleSize);
        hr = SORA_HW_TX_TRANSFER(pTransferObj, &Tcb->Base);
        FAILED_BREAK(hr);

        Tcb->PacketID = Tcb->Base.pTxDesc->__RCBDestAddr;
        #pragma warning(disable: 4995)
        wcscpy(Tcb->SymFileName, L"Application modulated symbols");
        #pragma warning(default: 4995)        
        NdisInterlockedInsertTailList(&TxQueue->CanTxQueue, &Tcb->TXCBList, &TxQueue->QueueLock);
        Output->TxID = Tcb->PacketID;
        
        Tcb = NULL; //Tcb used.
    } while(FALSE);

    Output->hResult = hr;

    if (Tcb) //Tcb not used.
    {
        SoraPacketFreeTxResource(pTransferObj, &Tcb->Base);
        NdisFreeToNPagedLookasideList(&TxQueue->TxCBLookaside, Tcb);
    }
    
    //DbgPrint("[UExtK] %s\n", (PUCHAR)(SoraRadioGetModulateBuffer(Radio)));

    return;
}

PTXCB
SoraKAllocTXCB(
	HANDLE TransferObj,
	PTX_SIGNAL_QUEUE TxQueue,
	ULONG SampleSize) {

	PTRANSFER_OBJ pTransferObj = (PTRANSFER_OBJ)TransferObj;
	PTXCB txcb;
	txcb = (PTXCB)NdisAllocateFromNPagedLookasideList(&TxQueue->TxCBLookaside);
	do {
		if (txcb) {
			ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(NULL, 
				SampleSize);
			SoraPacketInitialize(&txcb->Base);
	        if (SoraPacketGetTxResource(pTransferObj, 
				&txcb->Base) != STATUS_SUCCESS)
				break;
			PRCB_MD rcb_md;
			rcb_md = SoraAllocateRCBMem(pTransferObj->TransferResMgr,
				SampleSize);
			if (!rcb_md)
				break;
			txcb->Base.pTxDesc->__RCBDestAddr = rcb_md->Start;
			txcb->Base.pTxDesc->pRMD = rcb_md;
			return txcb;
		}
	} while(0);
	
	if (txcb) {
        SoraPacketFreeTxResource(pTransferObj, 
			&txcb->Base);
        NdisFreeToNPagedLookasideList(&TxQueue->TxCBLookaside, 
			txcb);
	}
	return NULL;
}

#endif
