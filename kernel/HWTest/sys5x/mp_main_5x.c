/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

   Miniport.C

Revision History:
    Created by senxiang, 2009/2/10

Notes:

--*/


#include "mp_5x.h"

#pragma NDIS_INIT_FUNCTION(DriverEntry)
#pragma NDIS_PAGEABLE_FUNCTION(MPInitialize)
#pragma NDIS_PAGEABLE_FUNCTION(MPHalt)
#pragma NDIS_PAGEABLE_FUNCTION(MPUnload)

#ifdef NDIS51_MINIPORT   
#pragma NDIS_PAGEABLE_FUNCTION(MPPnPEventNotify)
#endif

NDIS_HANDLE     NdisWrapperHandle;

void __ConsPacket(OUT PPACKET_BASE pPacket)
{
    PVOID               Va;
    PMDL                pMdl;

    Va = ExAllocatePoolWithTag(
            NonPagedPool,
            MDL_BUFFER_SIZE, 
            (ULONG)'TTWH');
    
    RtlZeroMemory(Va, MDL_BUFFER_SIZE);
    pMdl = IoAllocateMdl(Va, MDL_BUFFER_SIZE, FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool(pMdl);
    pPacket->pMdl = pMdl;

}

void __DestrPacket(OUT PPACKET_BASE pPacket)
{
    if (pPacket->pMdl)
    {
        PVOID Va = MmGetSystemAddressForMdlSafe(pPacket->pMdl, LowPagePriority);
        ExFreePoolWithTag(Va, (ULONG)'TTWH');
        IoFreeMdl(pPacket->pMdl);
    }
}

NDIS_STATUS 
DriverEntry(
            PVOID DriverObject,
            PVOID RegistryPath)
{
    HRESULT                         hr;
    NDIS_STATUS                        status;
    NDIS_MINIPORT_CHARACTERISTICS    mpChar;
    
    DEBUGP(MP_TRACE, ("[HT_STARTUP]---> DriverEntry built on "__DATE__" at "__TIME__ 
        "MP_NDIS_MAJOR_VERSION=%d, MP_NDIS_MINOR_VERSION=%d\n", 
        MP_NDIS_MAJOR_VERSION, MP_NDIS_MINOR_VERSION));

    //Associate miniport driver with NDIS
    NdisMInitializeWrapper(
            &NdisWrapperHandle,
            DriverObject,
            RegistryPath,
            NULL
            );
    
    if(!NdisWrapperHandle){
        DEBUGP(MP_ERROR, ("NdisMInitializeWrapper failed\n"));
        return NDIS_STATUS_FAILURE;
    }

    NdisZeroMemory(&mpChar, sizeof(mpChar));
    mpChar.MajorNdisVersion          = MP_NDIS_MAJOR_VERSION;
    mpChar.MinorNdisVersion          = MP_NDIS_MINOR_VERSION;
    mpChar.InitializeHandler         = MPInitialize;
    mpChar.HaltHandler               = MPHalt;
    
    mpChar.SetInformationHandler     = MPSetInformation;
    mpChar.QueryInformationHandler   = MPQueryInformation;
    
    mpChar.SendPacketsHandler        = MPSendPackets;
    mpChar.ReturnPacketHandler       = MPReturnPacket;

#ifdef NDIS51_MINIPORT
    mpChar.CancelSendPacketsHandler = MPCancelSendPackets;
    mpChar.PnPEventNotifyHandler    = MPPnPEventNotify;
    mpChar.AdapterShutdownHandler   = MPShutdown;
#endif

    DEBUGP(MP_LOUD, ("[HT_STARTUP] Calling NdisMRegisterMiniport...\n"));
    status = NdisMRegisterMiniport(
                    NdisWrapperHandle,
                    &mpChar,
                    sizeof(NDIS_MINIPORT_CHARACTERISTICS));
    if (status != NDIS_STATUS_SUCCESS) {
        
        DEBUGP(MP_ERROR, ("Status = 0x%08x\n", status));
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
        
    }
    else
    {
        NdisAllocateSpinLock(&GlobalData.Lock);
        NdisInitializeListHead(&GlobalData.AdapterList);  
        NdisMRegisterUnloadHandler(NdisWrapperHandle, MPUnload);
    }
    DEBUGP(MP_TRACE, ("[HT_STARTUP]<--- DriverEntry, Status=0x%08x\n", status));

    return status;
}




NDIS_STATUS 
MPInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext)
{
    NDIS_STATUS                Status = NDIS_STATUS_SUCCESS;
    PHWT_ADAPTER                Adapter=NULL;
    UINT                    index;
    
    DEBUGP(MP_TRACE, ("[HT_STARTUP]---> MPInitialize\n"));

    PAGED_CODE();
    do {
        for(index = 0; index < MediumArraySize; ++index)
        {
            if(MediumArray[index] == NIC_MEDIA_TYPE) {
                break;
            }
        }
        if(index == MediumArraySize)
        {
            DEBUGP(MP_ERROR, ("[HT_STARTUP] Expected media is not in MediumArray.\n"));
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }
        *SelectedMediumIndex = index;
        DEBUGP(MP_INFO, ("[HT_STARTUP] Expected media is in MediumArray[%d].\n", index));

        Status = MpAllocateAdapter(MiniportAdapterHandle, &Adapter);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        
        Status = NICInitializeAdapter(Adapter, WrapperConfigurationContext);
        
        if(Status != NDIS_STATUS_SUCCESS) {
            Status = NDIS_STATUS_FAILURE;
            MpFreeAdapter(Adapter);
            break;
        }
        
        NdisMGetDeviceProperty(MiniportAdapterHandle,
                           &Adapter->Pdo,
                           &Adapter->Fdo,
                           &Adapter->NextDeviceObject,
                           NULL,
                           NULL);
        
        NdisMSetAttributesEx(
            MiniportAdapterHandle,
            (NDIS_HANDLE) Adapter,
            0,
#ifdef NDIS50_MINIPORT
            NDIS_ATTRIBUTE_DESERIALIZE|
            NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS |
            NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
#else 
            NDIS_ATTRIBUTE_DESERIALIZE |
            NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
#endif               
            NIC_INTERFACE_TYPE);
    }while(FALSE);
    
    if(Status == NDIS_STATUS_SUCCESS) {
        NICAttachAdapter(Adapter);
        NICRegisterDevice();
    }
    DEBUGP(MP_TRACE, ("[HT_STARTUP]<--- MPInitialize Status = 0x%08x%\n", Status));
    return Status;
}

VOID 
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext
    )
{
    PHWT_ADAPTER Adapter = (PHWT_ADAPTER) MiniportAdapterContext;
    PAGED_CODE();
    DEBUGP(MP_TRACE, ("[HT_EXIT]---> MPHalt\n"));

	Adapter->Removed = 1;
	KeWaitForSingleObject(&Adapter->Exit,
		Executive,
		KernelMode,
		FALSE,
		NULL);
	
    NICCleanAdapter(Adapter);
    NICDettachAdapter(Adapter);
    MpFreeAdapter(Adapter);

    NICDeregisterDevice();
    DEBUGP(MP_TRACE, ("[HT_EXIT]<--- MPHalt\n"));
}

/* For global data cleanup, whereas MiniportHalt is only for one driver instance*/
VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
{
    DEBUGP(MP_TRACE, ("[HT_EXIT]--> MPUnload\n"));
    PAGED_CODE();
    NdisFreeSpinLock(&GlobalData.Lock);
    DEBUGP(MP_TRACE, ("[HT_EXIT]<--- MPUnload\n"));   
}

#ifdef NDIS51_MINIPORT
VOID MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    )
{
    //DEBUGP(MP_TRACE, ("---> MPPnPEventNotify, PnPEvent=%d\n, InformationBufferLength=%d", PnPEvent, InformationBufferLength));
    PAGED_CODE();
    //DEBUGP(MP_TRACE, ("<--- MPPnPEventNotify\n"));
}

VOID 
MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId
    )
{
    DEBUGP(MP_TRACE, ("---> MPCancelSendPackets\n"));
    DEBUGP(MP_TRACE, ("<--- MPCancelSendPackets\n"));
}

VOID 
MPShutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PHWT_ADAPTER Adapter = (PHWT_ADAPTER) MiniportAdapterContext;

    DEBUGP(MP_TRACE, ("---> MPShutdown\n"));

	{
		int i;
		for(i=0; i < MAX_RADIO_NUMBER; i++)
			if (Adapter->Radios2[i])
				SoraStopRadio2(Adapter->Radios2[i]);
	}
	
    DEBUGP(MP_TRACE, ("<--- MPShutdown\n"));
}

#pragma region "device registration"

NDIS_STATUS NICRegisterDevice(VOID)
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING      DeviceName;
    UNICODE_STRING      DeviceLinkUnicodeString;
    PDRIVER_DISPATCH    DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];
    PAGED_CODE();
    DEBUGP(MP_TRACE, ("[HT_STARTUP]-->NICRegisterDevice\n"));
    NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION+1) * sizeof(PDRIVER_DISPATCH));

    DispatchTable[IRP_MJ_CREATE]            = NICDispatch;
    DispatchTable[IRP_MJ_CLEANUP]           = NICDispatch;
    DispatchTable[IRP_MJ_CLOSE]             = NICDispatch;
    DispatchTable[IRP_MJ_DEVICE_CONTROL]    = NICDispatch;
    
    NdisInitUnicodeString(&DeviceName, NTDEVICE_STRING);
    NdisInitUnicodeString(&DeviceLinkUnicodeString, LINKNAME_STRING);

    Status = NdisMRegisterDevice(
                NdisWrapperHandle, 
                &DeviceName,
                &DeviceLinkUnicodeString,
                &DispatchTable[0],
                &g_ControlDeviceObject,
                &g_NdisDeviceHandle
            );

    DEBUGP(MP_TRACE, ("[HT_STARTUP]<--NICRegisterDevice: %x\n", Status));
    return Status;
}

NDIS_STATUS NICDeregisterDevice(VOID)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    PAGED_CODE();
    
    if (g_NdisDeviceHandle != NULL)
    {
        Status              = NdisMDeregisterDevice(g_NdisDeviceHandle);
        g_NdisDeviceHandle    = NULL;
    }
    
    return Status;
}

void CleanQueuedSendPacket(PHWT_ADAPTER Adapter) {

	KIRQL irql;
	KeAcquireSpinLock(&Adapter->UExtKeObj.SendPacketLock,
		&irql);
	if (Adapter->UExtKeObj.SendPacketControl) {
		ObDereferenceObject(Adapter->UExtKeObj.SendPacketControl);
		Adapter->UExtKeObj.SendPacketControl = NULL;
	}
	while(1) {
		PNDIS_PACKET Packet;
		if (remove_thread_safe_enlist_head(Adapter->UExtKeObj.SendPacketList, 
			&Packet,
			1)) {			
			NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_FAILURE);
			NdisMSendComplete(Adapter->AdapterHandle,
				Packet,
				NDIS_STATUS_FAILURE);
			continue;
		}
		break;
	}
	KeReleaseSpinLock(&Adapter->UExtKeObj.SendPacketLock,
		irql);
}

#pragma endregion

#endif 
