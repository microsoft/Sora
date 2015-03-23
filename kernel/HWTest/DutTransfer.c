#include "Ntifs.h"
#include "hwtest_miniport.h"

#include <ntstrsafe.h>
#include "stdlib.h"

ULONG __ReadFile(PCWSTR wszFileName, PVOID Buffer, ULONG Size)
{
    OBJECT_ATTRIBUTES   FileObjAttr;
    UNICODE_STRING      wstrFileName;
    HANDLE              hFileHandle = NULL;
    NTSTATUS            Status      = STATUS_SUCCESS;
    IO_STATUS_BLOCK     IoStatus;
    ULONG               nRead = 0;

    RtlInitUnicodeString(&wstrFileName, wszFileName);
    InitializeObjectAttributes(
        &FileObjAttr,
        &wstrFileName, 
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);
    Status = ZwCreateFile(
                &(hFileHandle),
                GENERIC_READ,
                &FileObjAttr,
                &IoStatus,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0);
    if (hFileHandle)
    {
        Status = ZwReadFile(
                    hFileHandle, 
                    NULL, 
                    NULL,
                    NULL,
                    &IoStatus,
                    Buffer, 
                    Size,
                    NULL,
                    NULL);
        if(!NT_SUCCESS(Status))
        {
            DbgPrint("[HWT] File read failed\n");
        }
        else
        {
            nRead = (ULONG)IoStatus.Information;
        }
        ZwClose(hFileHandle);
    }   
    else
    {
        DbgPrint("[HWT] File Open failed, nRead=%d\n", nRead);
    }
    return nRead;

}

HRESULT __FillWithSymbols(PVOID Buffer, ULONG Size, PULONG nRead, PCWSTR wszFileName)
{
    HRESULT hr = S_OK;
    *nRead = __ReadFile(wszFileName, Buffer, Size);
    if (*nRead == 0)
        hr = E_FILE_UNKNOWN;
    else
        ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(Buffer, *nRead);

    return hr;
}

HRESULT 
DutQueueTransferSigFile(
    PHWT_ADAPTER Adapter, 
    PTRANSFER_OBJ pTransferObj, 
    PCWSTR wszSigFile)
{
    HRESULT hr = S_OK;
    PTXCB Tcb = NULL;
    PTXSAMPLE Buffer;
    ULONG  Size, SampleLen = 0;
	
    do
    {
        
        Tcb = (PTXCB) NdisAllocateFromNPagedLookasideList(
            &Adapter->UExtKeObj.TxQueue.TxCBLookaside);
        if (Tcb == NULL) 
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }
        DbgPrint("[HWT] DutQueueTransferSigFile: TCB is allocated\n");
        SoraPacketInitialize(&Tcb->Base);
        hr = SoraPacketGetTxResource(pTransferObj, &Tcb->Base);
        FAILED_BREAK(hr);
        SoraPacketGetTxSampleBuffer(&Tcb->Base, &Buffer, &Size);
        DbgPrint("[HWT] DutQueueTransferSigFile: TX resource obtained with %08x\n", hr);
        hr = __FillWithSymbols(Buffer, Size, &SampleLen, wszSigFile);
        FAILED_BREAK(hr);
        DbgPrint("[HWT] DutQueueTransferSigFile: %08x bytes are ready\n", SampleLen);

        SoraPacketSetSignalLength(&Tcb->Base, SampleLen);
        hr = SORA_HW_TX_TRANSFER(pTransferObj, &Tcb->Base);
        FAILED_BREAK(hr);

        Tcb->PacketID = Tcb->Base.pTxDesc->__RCBDestAddr;
#pragma warning(disable: 4995)
        wcscpy(Tcb->SymFileName, wszSigFile);
#pragma warning(default: 4995)        
        //DbgPrint("[HWT] %08x with ID=%d inserted\n", Tcb, Tcb->PacketID);
        NdisInterlockedInsertTailList(
            &Adapter->UExtKeObj.TxQueue.CanTxQueue, 
            &Tcb->TXCBList, 
            &Adapter->UExtKeObj.TxQueue.QueueLock);

        Tcb = NULL;
    } while(FALSE);
    
    if (FAILED(hr))
    {
        SoraPacketFreeTxResource(pTransferObj, &Tcb->Base);
    }

    if (Tcb)
    {
        NdisFreeToNPagedLookasideList(&Adapter->UExtKeObj.TxQueue.TxCBLookaside, Tcb);
    }

    return hr;
}

ULONG 
SerialTXCacheInfo(
    PID_FILE_PAIR PairBuffer, 
    ULONG Count, 
    PHWT_ADAPTER Adapter)
{
    ULONG Ret = 0;
    PLIST_ENTRY Entry;
    PTXCB       Tcb;
    NdisAcquireSpinLock(&Adapter->UExtKeObj.TxQueue.QueueLock);
    FOREACH(&Adapter->UExtKeObj.TxQueue.CanTxQueue, Entry)
    {
        Tcb = CONTAINING_RECORD(Entry, TXCB, TXCBList);
        if (Ret >= Count) { 
            DEBUGP(MP_WARNING, ("[HWT] User buffer is too small for cache entries"));
            break;
        }

        PairBuffer[Ret].PacketID = Tcb->PacketID;
#pragma warning(disable: 4995)
        wcscpy(PairBuffer[Ret].SymFileName, Tcb->SymFileName);
#pragma warning(default: 4995)
        Ret++;
    }
    NdisReleaseSpinLock(&Adapter->UExtKeObj.TxQueue.QueueLock);
    return Ret;
}