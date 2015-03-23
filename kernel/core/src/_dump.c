#include "sora.h"

#define __MAX_DUMP_FILE_NAME_LENGTH_W   128 //128 WCHARs

#ifndef USER_MODE
#include "ntstrsafe.h"

__inline PUCHAR 
SoraGetRadioNextNPeekPoint(
    IN PSORA_RADIO_RX_STREAM pRxStream,
    IN PUCHAR pCurrentScanPoint, 
    IN ULONG  NextNBlock)
{
    PUCHAR ret = pCurrentScanPoint + SORA_GET_RX_DESC_SIZE(pCurrentScanPoint) * NextNBlock;
    if (ret >= pRxStream->__pEndPt)
    {
        ret = pRxStream->__pStartPt + (ret - pRxStream->__pEndPt) % (pRxStream->__nRxBufSize);
    }
    return ret;
}

HRESULT __WaitNewSignals(
                    IN PRX_BLOCK       pScanPoint, 
                    IN USHORT          uRetries,
                    IN ULONG           VStreamMask,
                    OUT FLAG           *fReachEnd);

HANDLE __CreateDumpFile(WCHAR *wszFileName)
{
    NTSTATUS            Status      = STATUS_SUCCESS;
    IO_STATUS_BLOCK     IoStatus;
    LARGE_INTEGER       CurrentTime = {0};
    UNICODE_STRING      wstrFileName;
    OBJECT_ATTRIBUTES   FileObjAttr;
    HANDLE              hFileHandle = NULL;
    WCHAR               wszTimeStamped[__MAX_DUMP_FILE_NAME_LENGTH_W];
    
    KeQuerySystemTime(&CurrentTime);
    Status = RtlStringCbPrintfW(
                wszTimeStamped, 
                __MAX_DUMP_FILE_NAME_LENGTH_W * sizeof(WCHAR), 
                L"%s_%d_%u.dmp", 
                wszFileName,
                CurrentTime.HighPart,
                CurrentTime.LowPart);
    if (STATUS_SUCCESS != Status)
        return hFileHandle;

    RtlInitUnicodeString(&wstrFileName, wszTimeStamped);
    InitializeObjectAttributes(
        &FileObjAttr,
        &wstrFileName, 
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    Status = ZwCreateFile(
                &(hFileHandle),
                FILE_WRITE_DATA | SYNCHRONIZE,
                &FileObjAttr,
                &IoStatus,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OVERWRITE_IF,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0);
    return hFileHandle;
}

NTSTATUS __DumpBuffer2File(HANDLE hFile, PUCHAR pBuffer, ULONG Size)
{
    NTSTATUS         Status    = STATUS_SUCCESS;
    IO_STATUS_BLOCK  IoStatus;

    Status = ZwWriteFile(hFile, NULL, NULL, NULL, &IoStatus, pBuffer, Size, NULL, NULL);

    return Status;
}

NTSTATUS __CloseDumpFile(HANDLE hFile)
{
    return ZwClose(hFile);
}

VOID __ClearNewRxBuffer(PSORA_RADIO pRadio) {

	PSORA_RADIO_RX_STREAM pRxStream;
	PRX_BLOCK p;
	FLAG fReachEnd;
	pRxStream = SORA_GET_RX_STREAM(pRadio);
	p = SoraRadioGetRxStreamPos(pRxStream);
	while(1) {
		HRESULT hr;
		hr = __WaitNewSignals(p, 1, SoraGetStreamVStreamMask(pRxStream), &fReachEnd);
		if (FAILED(hr))
			break;
		SORA_C_INVALIDATE_SIGNAL_BLOCK_EX(p, SoraGetStreamVStreamMask(pRxStream));
		p = __SoraRadioIncRxStreamPointer(pRxStream, p);
	}
	SoraRadioSetRxStreamPos(pRxStream, p);	
}

//base band scan should be stopped.
NTSTATUS SoraDumpNewRxBuffer(PSORA_RADIO pRadio, PUCHAR pBuffer, ULONG Size, PULONG pWritten)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSORA_RADIO_RX_STREAM pRxStream = SORA_GET_RX_STREAM(pRadio);
    PRX_BLOCK p = SoraRadioGetRxStreamPos(pRxStream);
    FLAG fReachEnd = 0;
    HRESULT hr = S_OK;
	
	*pWritten = 0;
    while ((*pWritten) + sizeof(RX_BLOCK) <= Size)
    {
        hr = __WaitNewSignals(p, 2000, SoraGetStreamVStreamMask(pRxStream), &fReachEnd);
        if (FAILED(hr))
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
        __DumpSignalBlockWithDesc(p, pBuffer + (*pWritten));
        SORA_C_INVALIDATE_SIGNAL_BLOCK_EX(p, SoraGetStreamVStreamMask(pRxStream));
        *pWritten += sizeof(RX_BLOCK);
        p = __SoraRadioIncRxStreamPointer(pRxStream, p);
    }
    
    SoraRadioSetRxStreamPos(pRxStream, p);
    return Status;
}

/*
IRQL: == DPC_LEVEL
*/
PUCHAR SoraSearchHWWritePoint(PSORA_RADIO pRadio)
{
    PSORA_RADIO_RX_STREAM pRxStream = SORA_GET_RX_STREAM(pRadio);
                    
    PRX_BLOCK p = NULL;
    int timeout = 4096 * 16;

    p = SoraRadioGetRxStreamPos(pRxStream);

    while(!SORA_C_RXBUF_SLOT_IS_OLDEST(p))
    {
        timeout--;
        if (timeout < 0)
        {
            p = NULL;
            break;
        }        
        p = (PRX_BLOCK)SoraGetRadioNextNPeekPoint(pRxStream, (PUCHAR)p, 1);
    }

    if (timeout >= 0) //we get HW write point
    {
        p = (PRX_BLOCK)SoraGetRadioNextNPeekPoint(pRxStream, (PUCHAR)p, 2); //skip 2
    }
    return (PUCHAR)p;
}

SORA_EXTERN_C
NTSTATUS __CopyOldRxBuffer(PSORA_RADIO pRadio, PUCHAR pBuffer, ULONG Size, PULONG pWritten);

NTSTATUS __CopyOldRxBuffer(PSORA_RADIO pRadio, PUCHAR pBuffer, ULONG Size, PULONG pWritten)
{
    PSORA_RADIO_RX_STREAM pRxStream = SORA_GET_RX_STREAM(pRadio);
                    
    PRX_BLOCK p;
    
    KIRQL OldIrql;
    //ULONGLONG StartTS, EndTS;
    //LARGE_INTEGER Freq;
    //ULONG DurationInMS;
    NTSTATUS Status = STATUS_SUCCESS;
    
    //KeQueryPerformanceCounter(&Freq);
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    
    p = (PRX_BLOCK)SoraSearchHWWritePoint(pRadio);
    //StartTS = __rdtsc();
    if (p)
    {
        *pWritten = 0;
        while((*pWritten) + sizeof(RX_BLOCK) <= Size)
        {
            if (!SORA_C_SIGNAL_BLOCK_IS_SCANNED(p, 0x1))
                break;
            __DumpSignalBlockWithDesc(p, pBuffer + (*pWritten));
            
            *pWritten += sizeof(RX_BLOCK);
            p = __SoraRadioIncRxStreamPointer(pRxStream, p);
        }
    }
    else
    {
        DbgPrint("[Dump] Can't find HW write pointer\n");
        Status = STATUS_UNSUCCESSFUL;
    }
    
    //EndTS = __rdtsc();
    KeLowerIrql(OldIrql);
    //DurationInMS = (ULONG)((EndTS - StartTS) * 1000 / Freq.QuadPart);

    //DbgPrint("[Perf] Dump 16MB cost %d ms\n", DurationInMS);
    return Status;
}

#endif
