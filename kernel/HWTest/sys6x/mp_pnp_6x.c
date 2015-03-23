#include "mp_6x.h"

NDIS_STATUS
MpSetRegistrationAttributes(
    __in  PHWT_ADAPTER                Adapter
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_ADAPTER_ATTRIBUTES    miniportAttributes;

    //
    // First we we set the registration attributes
    //
    NdisZeroMemory(&miniportAttributes, sizeof(miniportAttributes));
    miniportAttributes.RegistrationAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES;
    miniportAttributes.RegistrationAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_1;
    miniportAttributes.RegistrationAttributes.Header.Size = sizeof(NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES);
    
    miniportAttributes.RegistrationAttributes.MiniportAdapterContext = Adapter;

    miniportAttributes.RegistrationAttributes.AttributeFlags = NDIS_MINIPORT_ATTRIBUTES_BUS_MASTER |
    														   NDIS_MINIPORT_ATTRIBUTES_NO_HALT_ON_SUSPEND |
    														   NDIS_MINIPORT_ATTRIBUTES_NDIS_WDM;
    miniportAttributes.RegistrationAttributes.CheckForHangTimeInSeconds = 6;
    miniportAttributes.RegistrationAttributes.InterfaceType = 0;

    ndisStatus = NdisMSetMiniportAttributes(
                    Adapter->AdapterHandle, 
                    &miniportAttributes);
    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("Failed to set miniport registration attributes. Status = 0x%08x\n", ndisStatus);
    }

    return ndisStatus;
}

NDIS_STATUS
MpSetMiniportAttributes(
    __in  PHWT_ADAPTER                Adapter
    )
{
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_ADAPTER_ATTRIBUTES    miniportAttributes;
    NDIS_PM_CAPABILITIES        pmCapabilities;    

    NdisZeroMemory(&miniportAttributes, sizeof(miniportAttributes));
        
    miniportAttributes.GeneralAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES;
    miniportAttributes.GeneralAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    miniportAttributes.GeneralAttributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    
    miniportAttributes.GeneralAttributes.MediaType = NdisMedium802_3;
    miniportAttributes.GeneralAttributes.PhysicalMediumType = NdisPhysicalMediumUnspecified;    
    
    miniportAttributes.GeneralAttributes.MtuSize = 1500;
    miniportAttributes.GeneralAttributes.MaxXmitLinkSpeed = XMIT_SPEED;
    miniportAttributes.GeneralAttributes.MaxRcvLinkSpeed = RECV_SPEED;
    miniportAttributes.GeneralAttributes.XmitLinkSpeed = RECV_SPEED;
    miniportAttributes.GeneralAttributes.RcvLinkSpeed = RECV_SPEED;
    miniportAttributes.GeneralAttributes.MediaConnectState = MediaConnectStateConnected;
    miniportAttributes.GeneralAttributes.MediaDuplexState = MediaDuplexStateFull;
    miniportAttributes.GeneralAttributes.LookaheadSize = 1500;
    
    NdisZeroMemory(&pmCapabilities, sizeof(NDIS_PM_CAPABILITIES));
    pmCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    pmCapabilities.Header.Revision = NDIS_PM_CAPABILITIES_REVISION_1;
    pmCapabilities.Header.Size = NDIS_SIZEOF_NDIS_PM_CAPABILITIES_REVISION_1;
    miniportAttributes.GeneralAttributes.PowerManagementCapabilitiesEx = &pmCapabilities;

    miniportAttributes.GeneralAttributes.MacOptions = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
                                                            NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                                                            NDIS_MAC_OPTION_NO_LOOPBACK;

    miniportAttributes.GeneralAttributes.SupportedPacketFilters = NDIS_PACKET_TYPE_DIRECTED |
                                                            NDIS_PACKET_TYPE_MULTICAST |
                                                            NDIS_PACKET_TYPE_ALL_MULTICAST |
                                                            NDIS_PACKET_TYPE_BROADCAST;

    miniportAttributes.GeneralAttributes.MaxMulticastListSize = 32;
    miniportAttributes.GeneralAttributes.MacAddressLength = sizeof(DOT11_MAC_ADDRESS);

    NdisMoveMemory(
        &miniportAttributes.GeneralAttributes.PermanentMacAddress,
        Adapter->CurrentAddress,
        sizeof(DOT11_MAC_ADDRESS));

    NdisMoveMemory(
        &miniportAttributes.GeneralAttributes.CurrentMacAddress,
        Adapter->CurrentAddress, 
        sizeof(DOT11_MAC_ADDRESS)
        );
    miniportAttributes.GeneralAttributes.RecvScaleCapabilities = NULL;
    miniportAttributes.GeneralAttributes.AccessType = NET_IF_ACCESS_BROADCAST;
    miniportAttributes.GeneralAttributes.DirectionType = NET_IF_DIRECTION_SENDRECEIVE;
    miniportAttributes.GeneralAttributes.IfType = IF_TYPE_ETHERNET_CSMACD;
    miniportAttributes.GeneralAttributes.IfConnectorPresent = TRUE;
    miniportAttributes.GeneralAttributes.DataBackFillSize = 8;

    MpQuerySupportedOidsList(
        &miniportAttributes.GeneralAttributes.SupportedOidList, 
        &miniportAttributes.GeneralAttributes.SupportedOidListLength);

    ndisStatus = NdisMSetMiniportAttributes(
            Adapter->AdapterHandle,
            &miniportAttributes
            );
    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("Failed to set general attributes");
    }
    return ndisStatus;
}

VOID 
VNicFreeNdisPort(PHWT_ADAPTER Adapter)
{
    if (Adapter->PortNumber)
    {
        NdisMFreePort(Adapter->AdapterHandle, Adapter->PortNumber);
		Adapter->PortNumber = 0;
    }
}

NDIS_STATUS 
VNicAllocateNdisPort(
    __in NDIS_HANDLE AdapterHandle, 
    __out PNDIS_PORT_NUMBER AllocatedPortNumber)
{
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_PORT_CHARACTERISTICS   portChar;

    // Call NDIS to allocate the port
    NdisZeroMemory(&portChar, sizeof(NDIS_PORT_CHARACTERISTICS));

    HWT_NDIS_OBJ_INIT(
        &portChar.Header, 
        NDIS_OBJECT_TYPE_DEFAULT, 
        NDIS_PORT_CHARACTERISTICS_REVISION_1, 
        sizeof(NDIS_PORT_CHARACTERISTICS));

    portChar.Flags = NDIS_PORT_CHAR_USE_DEFAULT_AUTH_SETTINGS;
    portChar.Type = NdisPortTypeUndefined;
    portChar.MediaConnectState = MediaConnectStateConnected;
    portChar.XmitLinkSpeed = XMIT_SPEED;
    portChar.RcvLinkSpeed = RECV_SPEED;
    portChar.Direction = NET_IF_DIRECTION_SENDRECEIVE;
    portChar.SendControlState = NdisPortControlStateUnknown;
    portChar.RcvControlState = NdisPortControlStateUnknown;
    portChar.SendAuthorizationState = NdisPortControlStateUncontrolled; // Ignored
    portChar.RcvAuthorizationState = NdisPortControlStateUncontrolled; // Ignored

    ndisStatus = NdisMAllocatePort(AdapterHandle, &portChar);
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *AllocatedPortNumber = portChar.PortNumber;
    }
    return ndisStatus;
}

NDIS_STATUS 
MPInitializeEx(
    __in  NDIS_HANDLE             AdapterHandle,
    __in  NDIS_HANDLE             MiniportDriverContext,
    __in  PNDIS_MINIPORT_INIT_PARAMETERS     MiniportInitParameters
    )
{
    PHWT_ADAPTER    newAdapter = NULL;
    NDIS_STATUS     ndisStatus;
    UNREFERENCED_PARAMETER(MiniportDriverContext);

    DbgPrint("[HWTest]: MPInitializeEx -->\n");
    do
    {
        
        ndisStatus = MpAllocateAdapter(AdapterHandle, &newAdapter);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DbgPrint("[HWTest]: MpAllocateAdapter return %08x\n", ndisStatus);
            break;
        }
        
        ndisStatus = NICInitializeAdapter(newAdapter, AdapterHandle);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DbgPrint("[HWTest]: NICInitializeAdapter return %08x\n", ndisStatus);
            ndisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        ndisStatus = MpSetRegistrationAttributes(newAdapter);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DbgPrint("[HWTest]: MpSetRegistrationAttributes return %08x\n", ndisStatus);
            break;
        }

        ndisStatus = MpSetMiniportAttributes(newAdapter);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DbgPrint("[HWTest]: MpSetMiniportAttributes return %08x\n", ndisStatus);
            break;
        }

        ndisStatus = VNicAllocateNdisPort(AdapterHandle, &newAdapter->PortNumber);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            DbgPrint("[HWTest]: VNicAllocateNdisPort return %08x\n", ndisStatus);
            break;
        }

    } while (FALSE);
    
    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        if (newAdapter != NULL)
        {
            VNicFreeNdisPort(newAdapter);
            NICCleanAdapter(newAdapter);
            MpFreeAdapter(newAdapter);
            NdisWriteErrorLogEntry(AdapterHandle, NDIS_ERROR_CODE_HARDWARE_FAILURE, 0);
        }
    }
    else
    {
        NICAttachAdapter(newAdapter);
        NICRegisterDevice();
    }

    DbgPrint("[HWTest]: MPInitializeEx return %08x\n", ndisStatus);
    return ndisStatus;
}

VOID 
MPHalt(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  NDIS_HALT_ACTION        HaltAction
    )
{
    PHWT_ADAPTER    Adapter = (PHWT_ADAPTER)MiniportAdapterContext;
    
    UNREFERENCED_PARAMETER(HaltAction);

	Adapter->Removed = 1;
	KeWaitForSingleObject(&Adapter->Exit,
		Executive,
		KernelMode,
		FALSE,
		NULL);
	
    NICCleanAdapter(Adapter);
    
    NICDeregisterDevice();

	NICDettachAdapter(Adapter);
    VNicFreeNdisPort(Adapter);
    MpFreeAdapter(Adapter);

    DbgPrint("[HWTest] MPHalt return \n");
}


NDIS_STATUS 
MPRestart(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  PNDIS_MINIPORT_RESTART_PARAMETERS   MiniportRestartParameters
    )
{
    PHWT_ADAPTER                    adapter = (PHWT_ADAPTER)MiniportAdapterContext;
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS 
MPPause(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  PNDIS_MINIPORT_PAUSE_PARAMETERS     MiniportPauseParameters
    )
{
    PHWT_ADAPTER    adapter = (PHWT_ADAPTER)MiniportAdapterContext;
    
    UNREFERENCED_PARAMETER(MiniportPauseParameters); 
    
    return NDIS_STATUS_FAILURE;
}

VOID 
MPShutdownEx(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  NDIS_SHUTDOWN_ACTION    ShutdownAction
    )
{
    PHWT_ADAPTER                Adapter = (PHWT_ADAPTER)MiniportAdapterContext;

    DEBUGP(MP_TRACE, ("[HWTest]---> MPShutdown\n"));

	{
		int i;
		for(i=0; i < MAX_RADIO_NUMBER; i++)
			if (Adapter->Radios2[i])
				SoraStopRadio2(Adapter->Radios2[i]);
	}
	
    DEBUGP(MP_TRACE, ("[HWTest]<--- MPShutdown\n"));
    return;
}

VOID
MPDevicePnPEvent(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  PNET_DEVICE_PNP_EVENT   NetDevicePnPEvent
    )
{
    PHWT_ADAPTER                adapter = (PHWT_ADAPTER)MiniportAdapterContext;
    NDIS_DEVICE_PNP_EVENT       devicePnPEvent = NetDevicePnPEvent->DevicePnPEvent;

    return;
}

