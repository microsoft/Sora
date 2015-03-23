#include "mp_6x.h"

NDIS_HANDLE                     GlobalDriverContext = NULL;
NDIS_HANDLE                     GlobalDriverHandle = NULL;

ULONG                           FailDriverEntry = 0;

DRIVER_INITIALIZE DriverEntry;

NTSTATUS 
DriverEntry(
    __in    PDRIVER_OBJECT        DriverObject,
    __in    PUNICODE_STRING       RegistryPath
    )
{
    NDIS_STATUS                 Status = NDIS_STATUS_FAILURE;
    NDIS_MINIPORT_DRIVER_CHARACTERISTICS MChars;
    ULONG                       ndisVersion;

    DbgPrint("[HWTest]: Driver built at" __DATE__ " " __TIME__ "\n");

    //DbgBreakPoint();

    if (FailDriverEntry) //only for debugging
    {
        return NDIS_STATUS_FAILURE;
    }
    NdisZeroMemory(&MChars, sizeof(NDIS_MINIPORT_DRIVER_CHARACTERISTICS));

    ndisVersion = NdisGetVersion();
    if (ndisVersion <= MP_NDIS_VERSION_NEEDS_COMPATIBILITY)
    {
        DbgPrint("Unsupported OS \n");
        Status = NDIS_STATUS_FAILURE;
        return Status;
    }
    else
    {
        MChars.Header.Type      = NDIS_OBJECT_TYPE_MINIPORT_DRIVER_CHARACTERISTICS;
        MChars.Header.Size      = NDIS_SIZEOF_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
        MChars.Header.Revision  = NDIS_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
            
        MChars.MajorNdisVersion = MP_MAJOR_NDIS_VERSION;
        MChars.MinorNdisVersion = MP_MINOR_NDIS_VERSION; 
    }

    MChars.MajorDriverVersion   = HWT_MAJOR_DRIVER_VERSION;
    MChars.MinorDriverVersion   = HWT_MINOR_DRIVER_VERSION;

    //Init/PnP handlers
    MChars.InitializeHandlerEx          = MPInitializeEx;
    MChars.RestartHandler               = MPRestart;
    MChars.PauseHandler                 = MPPause;
    MChars.ShutdownHandlerEx            = MPShutdownEx;
    MChars.DevicePnPEventNotifyHandler  = MPDevicePnPEvent;
    MChars.HaltHandlerEx                = MPHalt;
    MChars.UnloadHandler                = DriverUnload;

    //Query/Set/Method requests handlers
    MChars.OidRequestHandler            = MPOidRequest;
    MChars.CancelOidRequestHandler      = MPCancelOidRequest;

    //set optional mp services handler
    MChars.SetOptionsHandler            = MPSetOptions;

    //send/receive handlers
    MChars.SendNetBufferListsHandler    = MPSendNetBufferLists;
    MChars.CancelSendHandler            = MPCancelSendNetBufferLists;
    MChars.ReturnNetBufferListsHandler  = MPReturnNetBufferLists;

    //Fault handling handlers
    MChars.CheckForHangHandlerEx        = MPCheckForHangEx;
    MChars.ResetHandlerEx               = MPResetEx;

    MChars.DirectOidRequestHandler          = MPDirectOidRequest;
    MChars.CancelDirectOidRequestHandler    = MPCancelDirectOidRequest;

    
    Status = NdisMRegisterMiniportDriver(
                DriverObject, 
                RegistryPath, 
                GlobalDriverContext, 
                &MChars,
                &GlobalDriverHandle
                );
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("Failed to register miniport with NDIS. Status = 0x%x", Status);
    }
    
    DbgPrint("[HWTest] DriverEntry return %08x\n", Status);
    NdisInitializeListHead(&GlobalData.AdapterList);
	NdisAllocateSpinLock(&GlobalData.Lock);
    return Status;
}

VOID
DriverUnload(
    PDRIVER_OBJECT          DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject); 
    //
    // Deregister this miniport from NDIS
    //
    NdisMDeregisterMiniportDriver(GlobalDriverHandle);
    NdisFreeSpinLock(&GlobalData.Lock);
}

NDIS_STATUS
MPSetOptions(
    NDIS_HANDLE             NdisMiniportDriverHandle,
    NDIS_HANDLE             MiniportDriverContext
    )
{
    UNREFERENCED_PARAMETER(NdisMiniportDriverHandle);
    UNREFERENCED_PARAMETER(MiniportDriverContext);

    return NDIS_STATUS_SUCCESS;

}

NDIS_STATUS
MPResetEx(
    NDIS_HANDLE             MiniportAdapterContext,
    PBOOLEAN                AddressingReset
    )
{
    //DbgBreakPoint();
    return NDIS_STATUS_FAILURE;
}

BOOLEAN
MPCheckForHangEx(
    NDIS_HANDLE             MiniportAdapterContext
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    return FALSE;
}

#pragma region "Device Registration"

NDIS_STATUS NICRegisterDevice(VOID)
{
    NDIS_STATUS								status;
    UNICODE_STRING							sddlString;
    NDIS_DEVICE_OBJECT_ATTRIBUTES			doAttr;
    UNICODE_STRING							devName;
    UNICODE_STRING							symLinkName;
    PDRIVER_DISPATCH						DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION + 1) * sizeof(PDRIVER_DISPATCH));
	RtlInitUnicodeString(&devName, NTDEVICE_STRING);
	RtlInitUnicodeString(&symLinkName, LINKNAME_STRING);
	DispatchTable[IRP_MJ_CREATE]			= NICDispatch;
	DispatchTable[IRP_MJ_CLEANUP]			= NICDispatch;
	DispatchTable[IRP_MJ_CLOSE]				= NICDispatch;
	DispatchTable[IRP_MJ_DEVICE_CONTROL]	= NICDispatch;

    RtlInitUnicodeString(&sddlString, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;BU)(A;;GA;;;WD)");


	//Fill device object attributes structure
	doAttr.Header.Type			= NDIS_OBJECT_TYPE_DEVICE_OBJECT_ATTRIBUTES;
	doAttr.Header.Revision		= NDIS_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;
	doAttr.Header.Size			= sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES);
	doAttr.DeviceName			= &devName;
	doAttr.SymbolicName			= &symLinkName;
	doAttr.MajorFunctions		= &DispatchTable[0];
	doAttr.ExtensionSize		= 10;
	doAttr.DefaultSDDLString 	= &sddlString;
	doAttr.DeviceClassGuid		= NULL;

	//Register Device
	status = NdisRegisterDeviceEx(GlobalDriverHandle, &doAttr, &g_ControlDeviceObject, &g_NdisDeviceHandle);
	if(status != NDIS_STATUS_SUCCESS)
	{
		DbgPrint("[HWTest] NdisMRegisterDeviceEx failed with 0x%08x\n", status);
	}

    return status;
}

NDIS_STATUS NICDeregisterDevice(VOID)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    PAGED_CODE();
    
    if (g_NdisDeviceHandle != NULL)
    {
        NdisDeregisterDeviceEx(g_NdisDeviceHandle);
        g_NdisDeviceHandle    = NULL;
    }
    
    DbgPrint("[HWTest] deregister device return %08x\n", Status);
    return Status;
}
#pragma endregion

void CleanQueuedSendPacket(PHWT_ADAPTER Adapter) {

	KIRQL irql;
	KeAcquireSpinLock(&Adapter->UExtKeObj.SendPacketLock,
		&irql);
	if (Adapter->UExtKeObj.SendPacketControl) {
		ObDereferenceObject(Adapter->UExtKeObj.SendPacketControl);
		Adapter->UExtKeObj.SendPacketControl = NULL;
	}
	while(1) {
		PNET_BUFFER NetBuffer;
		if (remove_thread_safe_enlist_head(Adapter->UExtKeObj.SendPacketList, 
			&NetBuffer,
			1)) {
			PNET_BUFFER_LIST NetBufferList;
			NetBufferList = NET_BUFFER_BUFFER_LIST(NetBuffer);
			NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_FAILURE;
			switch(InterlockedDecrement(&NET_BUFFER_LIST_BUFFER_COUNT(NetBufferList))) {
			case 0:
				NdisMSendNetBufferListsComplete(Adapter->AdapterHandle,
					NetBufferList,
					0);
				break;
			default:
				break;
			}
			continue;
		}
		break;
	}
	KeReleaseSpinLock(&Adapter->UExtKeObj.SendPacketLock,
		irql);
}
