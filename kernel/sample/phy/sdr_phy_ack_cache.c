/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_phy_ack_cache.c

Abstract: Implementation for 802.11b software physical layer's ACK symbol cache manager. 
          
History: 
          7/22/2009: Created by senxiang
--*/
#include "sdr_phy_precomp.h"
#include "CRC32.h"

static A16 COMPLEX8 s_AckIntermidiateOutput[MAX_PHY_ACK_SIZE/sizeof(COMPLEX8)];

HRESULT SdrPhyInitAckCache(
            OUT PACK_CACHE_MAN pAckCacheMan, 
            IN PDEVICE_OBJECT pDeviceObj,
            IN PPHY pOwnerPhy,
            IN PSORA_RADIO pRadio, 
            IN ULONG MaxAckSize, 
            IN ULONG MaxAckNum
            )
{
    HRESULT hr;
    
    PHYSICAL_ADDRESS    PhysicalAddress      = {0, 0};
    PHYSICAL_ADDRESS    PhysicalAddressLow   = {0, 0};
    PHYSICAL_ADDRESS    PhysicalAddressHigh  = {0x80000000, 0};

    NdisZeroMemory(pAckCacheMan, sizeof (ACK_CACHE_MAN)); //constructor
    NdisInitializeEvent(&pAckCacheMan->RemoveEvent); 
    NdisAllocateSpinLock(&pAckCacheMan->ReqQueueLock);
    pAckCacheMan->pOwnerPhy = pOwnerPhy;
    do {
        hr = SoraInitSignalCache ( &pAckCacheMan->AckCache, 
								   pOwnerPhy->TransferObj,	
                                   pRadio, 
                                   MaxAckSize, 
                                   MaxAckNum);
        FAILED_BREAK(hr);

        pAckCacheMan->pAckModulateBuffer
            = MmAllocateContiguousMemorySpecifyCache(
                    MaxAckSize , 
                    PhysicalAddressLow, 
                    PhysicalAddressHigh, 
                    PhysicalAddress, 
                    MmNonCached
                    );
        if (pAckCacheMan->pAckModulateBuffer == NULL)
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }
        pAckCacheMan->AckModulateBufferPA = 
            MmGetPhysicalAddress((PVOID)pAckCacheMan->pAckModulateBuffer);
        pAckCacheMan->AckModulateBufferSize = MaxAckSize;

        pAckCacheMan->pSysThreadWorkItem = IoAllocateWorkItem(pDeviceObj);
        if (pAckCacheMan->pSysThreadWorkItem == NULL)
        {
            hr = E_NOT_ENOUGH_RESOURCE;
            break;
        }
        //IoInitializeWorkItem(pDeviceObj, pAckCacheMan->pSysThreadWorkItem);        
        
    }while(FALSE);
    
    MP_INC_REF(pAckCacheMan);

    if (FAILED(hr))
    {
        SdrPhyCleanupAckCache(pAckCacheMan);
    }
    
    return hr;
}

VOID SdrPhyCleanupAckCache(
            OUT PACK_CACHE_MAN pAckCacheMan)
{
    if (pAckCacheMan->RefCount == 0) //it is not init yet.
        return; 

    MP_DEC_REF(pAckCacheMan);
    NdisWaitEvent(&pAckCacheMan->RemoveEvent, 0); 
    //make sure no one refence the cache, since there is async worker thread
    
    SoraCleanSignalCache(&pAckCacheMan->AckCache);
    
    if (pAckCacheMan->pAckModulateBuffer )
    {
        MmFreeContiguousMemorySpecifyCache(
            pAckCacheMan->pAckModulateBuffer,
            pAckCacheMan->AckModulateBufferSize,
            MmNonCached);
        pAckCacheMan->pAckModulateBuffer = NULL;
    }

    if (pAckCacheMan->pSysThreadWorkItem)
    {
        //IoUninitializeWorkItem(pAckCacheMan->pSysThreadWorkItem);
        IoFreeWorkItem(pAckCacheMan->pSysThreadWorkItem);
        pAckCacheMan->pSysThreadWorkItem = NULL;
    }

    NdisZeroMemory(pAckCacheMan, sizeof(ACK_CACHE_MAN));
    return;
}

HRESULT __Enqueue(
            IN PACK_CACHE_MAN pAckCacheMan, 
            IN PMAC_ADDRESS pMAC_Addr)
{
    HRESULT hr = E_FAIL;
    NdisAcquireSpinLock(&pAckCacheMan->ReqQueueLock);
    if (!pAckCacheMan->ReqQueue[pAckCacheMan->QueueHeadIndex].fOccupied)
    {//enqueue
        int i;
        hr = S_OK;
        pAckCacheMan->ReqQueue[pAckCacheMan->QueueHeadIndex].MacAddress.QuadKey.u.LowPart = 0;
        pAckCacheMan->ReqQueue[pAckCacheMan->QueueHeadIndex].MacAddress.QuadKey.u.HighPart = 0;
        for (i = 0; i < MAC_ADDRESS_LENGTH; i++)
            pAckCacheMan->ReqQueue[pAckCacheMan->QueueHeadIndex].MacAddress.KeyBytes[i] = pMAC_Addr->Address[i];
        pAckCacheMan->ReqQueue[pAckCacheMan->QueueHeadIndex].fOccupied = TRUE;
        pAckCacheMan->QueueHeadIndex++;
        pAckCacheMan->QueueHeadIndex %= MAX_ACK_MAKE_REQ_NUM;
    }
    NdisReleaseSpinLock(&pAckCacheMan->ReqQueueLock);
    return hr;
}

void __Dequeue(
        IN PACK_CACHE_MAN pAckCacheMan, 
        OUT PMAC_ADDRESS pMAC_Addr)
{
    int i;
    NdisAcquireSpinLock(&pAckCacheMan->ReqQueueLock);
    ASSERT(pAckCacheMan->ReqQueue[pAckCacheMan->QueueTailIndex].fOccupied);
    for (i = 0; i < MAC_ADDRESS_LENGTH; i++)
        pMAC_Addr->Address[i] = pAckCacheMan->ReqQueue[pAckCacheMan->QueueTailIndex].MacAddress.KeyBytes[i];
    pAckCacheMan->ReqQueue[pAckCacheMan->QueueTailIndex].fOccupied = FALSE;

    pAckCacheMan->QueueTailIndex++;
    pAckCacheMan->QueueTailIndex %= MAX_ACK_MAKE_REQ_NUM;
    
    NdisReleaseSpinLock(&pAckCacheMan->ReqQueueLock);
}

void Save(PUCHAR buf, ULONG size)
{
    HANDLE hFile = __CreateDumpFile(L"\\Device\\HarddiskVolume1\\Ack");
    if(hFile != NULL)
    {
        __DumpBuffer2File(hFile, buf, size);
        __CloseDumpFile(hFile);
    }
}

ULONG SdrPhyModulateACK11A(PPHY Phy, MAC_ADDRESS RecvMacAddress, PVOID PhyACKBuffer)
{
	return BB11AModulateACK(Phy->BBContextFor11A.TxVector.SampleRate, &RecvMacAddress, PhyACKBuffer);
}

ULONG SdrPhyModulateACK11B(PPHY Phy, MAC_ADDRESS RecvMacAddress, PVOID PhyACKBuffer)
{
    ULONG crc32;
    DOT11_MAC_ACK_FRAME  AckFrame       = {0};
    DOT11B_PLCP_TXVECTOR Dot11BTxVector = {0};
    ULONG OutputLengthFIR               = 0;
    UINT OutputLength                   = 0;
    
    UNREFERENCED_PARAMETER(Phy);
    AckFrame.FrameControl.Subtype   = SUBT_ACK;
    AckFrame.FrameControl.Type      = FRAME_CTRL;
    AckFrame.RecvAddress            = RecvMacAddress;
    AckFrame.Duration               = 0;
    AckFrame.FCS                    = CalcCRC32((PUCHAR)&AckFrame, sizeof(DOT11_MAC_ACK_FRAME) - sizeof(ULONG));

    Dot11BTxVector.PreambleType         = DOT11B_PLCP_IS_SHORT_PREAMBLE;
    Dot11BTxVector.DateRate             = DOT11B_PLCP_DATA_RATE_2M;
    Dot11BTxVector.ModSelect            = DOT11B_PLCP_IS_CCK;

    BB11BPMDBufferTx4XWithShortHeader(
        &Dot11BTxVector, 
        (PUCHAR)&AckFrame, 
        sizeof(DOT11_MAC_ACK_FRAME) - 4,
        (PUCHAR)s_AckIntermidiateOutput, &OutputLength);//OutputLength is COMPLEX8 count
    crc32 = CalcCRC32((PUCHAR)s_AckIntermidiateOutput, OutputLength * sizeof(COMPLEX8));
    memset(((PUCHAR)s_AckIntermidiateOutput) + OutputLength * sizeof(COMPLEX8), 0, 64); //tailed 0 for FIR
    if (crc32 != 0xaca87240)
    {
        DbgPrint("[TEMP1]crc32 for FIR input is different, 0x%08x", crc32);
    }

    BB11BPMDSpreadFIR4SSE(
        s_AckIntermidiateOutput, 
        OutputLength, 
        (PCOMPLEX8)PhyACKBuffer, &OutputLengthFIR);
    crc32 = CalcCRC32((PUCHAR)PhyACKBuffer, OutputLength * sizeof(COMPLEX8));
    if (crc32 != 0xfdf0c0fc)
    {
        DbgPrint("[TEMP1]crc32 for FIR output is different, 0x%08x\n", crc32);
        Save((PUCHAR)PhyACKBuffer, OutputLength * sizeof(COMPLEX8));
    }

    return OutputLength * sizeof(COMPLEX8);
}

VOID AckCacheMakeThread( 
        IN PDEVICE_OBJECT  DeviceObject,
        IN PVOID  Context )
{
    PACK_CACHE_MAN pAckCacheMan = (PACK_CACHE_MAN)Context;
    MAC_ADDRESS MacAddr;
    CACHE_KEY Key;
    ULONG Length = 0;
    HRESULT hr;

    UNREFERENCED_PARAMETER(DeviceObject);

    MP_INC_REF(pAckCacheMan);
    do
    {
        int i;
        __Dequeue(pAckCacheMan, &MacAddr);
        Key.QuadKey.u.HighPart = 0;
        Key.QuadKey.u.LowPart = 0;
        for (i = 0; i < MAC_ADDRESS_LENGTH; i++)
        {
            Key.KeyBytes[i] = MacAddr.Address[i];
        }
        Length = (*(pAckCacheMan->pOwnerPhy->FnPHY_ModAck))(
                    pAckCacheMan->pOwnerPhy,
                    MacAddr, 
                    pAckCacheMan->pAckModulateBuffer); // SdrPhyModulateACK dynamic bind to SdrPhyModulateACK11A,B;

        hr = SoraInsertSignal(
                &pAckCacheMan->AckCache,
                pAckCacheMan->pAckModulateBuffer, 
                &pAckCacheMan->AckModulateBufferPA, 
                Length, 
                Key);
        if (hr == E_TX_TRANSFER_FAIL)
        {
            DbgPrint("[TEMP1] Ack insert cache failed, return 0x%08x\n", hr);
            InterlockedIncrement(&pAckCacheMan->pOwnerPhy->HwErrorNum);
        }
        else
        {
            DbgPrint("[TEMP1] Ack insert cache succ, return 0x%08x\n", hr);
        }
    }while(InterlockedDecrement(&pAckCacheMan->PendingReqNum) != 0);

    MP_DEC_REF(pAckCacheMan);
    return;
}

HRESULT SdrPhyAsyncMakeAck(
            IN PACK_CACHE_MAN pAckCacheMan, 
            IN PMAC_ADDRESS pMAC_Addr)
{
    HRESULT hr;
    int i;

    DbgPrint("[TEMP1] make ack for %02x-%02x-%02x-%02x-%02x-%02x\n", 
            pMAC_Addr->Address[0], pMAC_Addr->Address[1], pMAC_Addr->Address[2], 
            pMAC_Addr->Address[3], pMAC_Addr->Address[4], pMAC_Addr->Address[5]);
    hr = __Enqueue(pAckCacheMan, pMAC_Addr);

    if (InterlockedIncrement(&pAckCacheMan->PendingReqNum) == 1) //queue is empty before, worker thread exited.
    {
        IoQueueWorkItem(
            pAckCacheMan->pSysThreadWorkItem, 
            AckCacheMakeThread, 
            DelayedWorkQueue, 
            pAckCacheMan);
    }
    return hr;
}


