/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: 
   Miniport.C

Abstract:
    NDIS 5.x Miniport driver for 802.11b Sora Sample. This file 
    defines DriverEntry and other call-back routines for NDIS 
    miniport.

Revision History:
    Created by senxiang, 7/Apr/2009

Notes:

--*/


#include "miniport.h"
#include "sora.h"
#include "trace.h"
#ifdef EVENT_TRACING
#include "miniport.tmh"
#endif
#include "init.h"

#pragma NDIS_INIT_FUNCTION(DriverEntry)
#pragma NDIS_PAGEABLE_FUNCTION(MPInitialize)
#pragma NDIS_PAGEABLE_FUNCTION(MPDeviceIoControl)
#pragma NDIS_PAGEABLE_FUNCTION(MpSetRegistrationAttributes)
#pragma NDIS_PAGEABLE_FUNCTION(MpSetMiniportAttributes)
#pragma NDIS_PAGEABLE_FUNCTION(VNicAllocateNdisPort)
#pragma NDIS_PAGEABLE_FUNCTION(MPHalt)
#pragma NDIS_PAGEABLE_FUNCTION(MPUnload)
#pragma NDIS_PAGEABLE_FUNCTION(MiniportResetEx)
#pragma NDIS_PAGEABLE_FUNCTION(MPPnPEventNotify)
#pragma NDIS_PAGEABLE_FUNCTION(MPAllocatePacketPool)

INT             MPDebugLevel = MP_INFO;
NDIS_HANDLE     g_NdisMiniportDriverHandle;


/*++
In the context of its DriverEntry function, a miniport driver associates
itself with NDIS, specifies the NDIS version that it is using, and 
registers its entry points. 


Parameters:
    PVOID DriverObject - pointer to the driver object. 
    PVOID RegistryPath - pointer to the driver registry path.

Return Value:
    
    NDIS_STATUS_xxx code

IRQL: PASSIVE_LEVEL
--*/

NDIS_STATUS 
DriverEntry(
            IN PVOID DriverObject,
            IN PVOID RegistryPath)
{
    NDIS_STATUS                             status;
    NDIS_MINIPORT_DRIVER_CHARACTERISTICS    mpDChar;

    WPP_INIT_TRACING(DriverObject, (PCUNICODE_STRING)RegistryPath);

    DEBUGP(MP_INFO, 
        ("[STARTUP]---> DriverEntry built on "__DATE__" at "__TIME__ 
        ", MP_NDIS_MAJOR_VERSION=%d, MP_NDIS_MINOR_VERSION=%d\n", 
        MP_NDIS_MAJOR_VERSION, 
        MP_NDIS_MINOR_VERSION));

    //Fill characteristics structure
    NdisZeroMemory(&mpDChar, sizeof(mpDChar));
    mpDChar.Header.Type                 = NDIS_OBJECT_TYPE_MINIPORT_DRIVER_CHARACTERISTICS;
    mpDChar.Header.Size                 = NDIS_SIZEOF_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
    mpDChar.Header.Revision             = NDIS_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;

    mpDChar.MajorNdisVersion            = MP_NDIS_MAJOR_VERSION;
    mpDChar.MinorNdisVersion            = MP_NDIS_MINOR_VERSION;
    mpDChar.MajorDriverVersion          = MP_NDIS_MAJOR_DRIVER_VERSION;
    mpDChar.MinorDriverVersion          = MP_NDIS_MINOR_DRIVER_VERSION;

    mpDChar.InitializeHandlerEx         = MPInitialize;
    mpDChar.HaltHandlerEx               = MPHalt;
    mpDChar.UnloadHandler               = MPUnload;
    mpDChar.OidRequestHandler           = MPOidRequest;
    mpDChar.SendNetBufferListsHandler   = MPSendNetBufferLists;
    mpDChar.CancelSendHandler           = MPCancelSendNetBufferLists;
    mpDChar.ReturnNetBufferListsHandler = MPReturnNetBufferLists;
    mpDChar.ShutdownHandlerEx           = MPShutdown;
    mpDChar.DevicePnPEventNotifyHandler = MPPnPEventNotify;

    mpDChar.PauseHandler                = MiniportPause;
    mpDChar.RestartHandler              = MiniportRestart;
    mpDChar.CheckForHangHandlerEx       = MiniportCheckForHangEx;
    mpDChar.ResetHandlerEx              = MiniportResetEx;
    mpDChar.CancelOidRequestHandler     = MPCancelOidRequest;
	mpDChar.SetOptionsHandler           = MiniportSetOptions;
    mpDChar.DirectOidRequestHandler     = MPDirectOidRequest;
    mpDChar.CancelDirectOidRequestHandler = MPCancelDirectOidRequest;

    //Register miniport driver
    status = NdisMRegisterMiniportDriver((PDRIVER_OBJECT)DriverObject, (PUNICODE_STRING)RegistryPath, NULL, &mpDChar, &g_NdisMiniportDriverHandle);
    if(status != NDIS_STATUS_SUCCESS)
    {
        DEBUGP(MP_ERROR, ("[STARTUP]NdisMRegisterMiniport failed with status 0x%08x\n", status));
        if(g_NdisMiniportDriverHandle)
        {
            NdisMDeregisterMiniportDriver(g_NdisMiniportDriverHandle);
        }
        return status;
    }

    return status;
}

/*++
    The MiniportInitialize function is a required function that sets up a 
    NIC (or virtual NIC) for network I/O operations, claims all hardware 
    resources necessary to the NIC in the registry, and allocates resources
    the driver needs to carry out network I/O operations.

    MiniportInitialize runs at IRQL = PASSIVE_LEVEL.
    
    Parameters:

    Return Value:
    
    NDIS_STATUS_xxx code
    
    IRQL: PASSIVE_LEVEL
--*/
NDIS_STATUS
MPInitialize(
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE MiniportDriverContext,
    IN PNDIS_MINIPORT_INIT_PARAMETERS MiniportInitParameters)
{
    NDIS_STATUS Status  = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER Adapter = NULL;

    DEBUGP(MP_INFO, ("[STARTUP] -->MPInitializeEx \n"));
    PAGED_CODE();

    do
    {
        //Allocate adapter
        Status = NICAllocAdapter(&Adapter);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        Adapter->AdapterHandle = MiniportAdapterHandle;

        //Init adapter
        Status = NICInitializeAdapter(Adapter, MiniportAdapterHandle);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        //Set attributes
        Status = MpSetRegistrationAttributes(Adapter);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        Status = MpSetMiniportAttributes(Adapter);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        //Allocate NIDS port
        Status = VNicAllocateNdisPort(Adapter->AdapterHandle, &Adapter->PortNumber);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DbgPrint("[init]: VNicAllocateNdisPort return %08x\n", Status);
            break;
        }
    }
    while(FALSE);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        if(Adapter != NULL)
        {
            FreeNdisPort(Adapter);
            FreeAdapter(Adapter);
        }
    }
    else
    {
        NICRegisterDevice();
    }

    DEBUGP(MP_INFO, ("[STARTUP] <--MPInitialize with 0x%08x\n", Status));
    return Status;
}

/*
FreeNdisPort
*/
VOID FreeNdisPort(IN PMP_ADAPTER Adapter)
{
    if(Adapter->PortNumber)
    {
        NdisMFreePort(Adapter->AdapterHandle, Adapter->PortNumber);
		Adapter->PortNumber = 0;
    }
}

/*
MiniportPause
*/
NDIS_STATUS
MiniportPause(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_MINIPORT_PAUSE_PARAMETERS MiniportPauseParameters)
{
    PMP_ADAPTER adapter = (PMP_ADAPTER)MiniportAdapterContext;

    UNREFERENCED_PARAMETER(MiniportPauseParameters);

    return NDIS_STATUS_FAILURE;
}

/*
MiniportRestart
*/
NDIS_STATUS
MiniportRestart(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_MINIPORT_RESTART_PARAMETERS MiniportRestartParameters)
{
    return NDIS_STATUS_SUCCESS;
}

/*
MiniportCheckForHangEx
*/
BOOLEAN
MiniportCheckForHangEx(
    IN NDIS_HANDLE MiniportAdapterContext)
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    return FALSE;
}

/*
MiniportResetEx
*/
NDIS_STATUS
MiniportResetEx(
    IN NDIS_HANDLE MiniportAdapterContext,
    OUT PBOOLEAN AddressingReset)
{
    DbgBreakPoint();
    return NDIS_STATUS_SUCCESS;
}

/*
MiniportSetOptions
*/
NDIS_STATUS
MiniportSetOptions(
    IN NDIS_HANDLE NdisDriverHandle,
    IN NDIS_HANDLE DriverContext)
{
    UNREFERENCED_PARAMETER(NdisDriverHandle);
    UNREFERENCED_PARAMETER(DriverContext);
    return NDIS_STATUS_SUCCESS;
}

/*
    MPCancelSendNetBufferLists ackets cancels the transmission of all packets that
    are marked with a specified cancellation identifier. Miniport drivers
    that queue send packets for more than one second should export this
    handler. When a protocol driver or intermediate driver calls the
    NdisCancelSendPackets function, NDIS calls the MiniportCancelSendPackets 
    function of the appropriate lower-level driver (miniport driver or 
    intermediate driver) on the binding.

    Arguments:

    MiniportAdapterContext      Pointer to our adapter
    CancelId                    All the packets with this Id should be cancelled

    Return Value:

    None

    IRQL: PASSIVE_LEVEL

*/
VOID 
MPCancelSendNetBufferLists(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PVOID        CancelId
    )
{
    KdPrint(("--> MPCancelSendNetBufferLists\n"));
    TraceEvents(TRACE_LEVEL_WARNING, DBG_DPC, "--> MPCancelSendNetBufferLists\n");
    TraceEvents(TRACE_LEVEL_WARNING, DBG_DPC, "<-- MPCancelSendNetBufferLists\n");
}

/*
 MPPnPEventNotify  is to handle PnP notification messages.

Parameters:
MiniportAdapterContext:      pointer to our adapter
NetDevicePnPEvent:           Self-explanatory 

Return:

None

IRQL:   PASSIVE_LEVEL
*/
VOID
MPPnPEventNotify(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN PNET_DEVICE_PNP_EVENT    NetDevicePnPEvent)
{
   if(NetDevicePnPEvent == IRP_MN_START_DEVICE) {
        KdPrint(("IRP_MN_START_DEVICE\n"));
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, DBG_PNP, 
        "---> MPPnPEventNotify, NetDevicePnPEvent=%d \n", 
        NetDevicePnPEvent->DevicePnPEvent);

    PAGED_CODE();
    TraceEvents( TRACE_LEVEL_INFORMATION, DBG_PNP, "<--- MPPnPEventNotify\n");
}

/*++
    The unload handler is called during driver unload to free up resources
    acquired in DriverEntry. This handler is registered through 
    NdisMRegisterUnloadHandler. Note that an unload handler differs from 
    a MiniportHalt function in that the unload handler has a more global
    scope, whereas the scope of the MiniportHalt function is restricted 
    to a particular miniport driver instance.

    Runs at IRQL = PASSIVE_LEVEL. 
    
Parameters:

    DriverObject        Not used

Return Value:

    None
    

IRQL: PASSIVE_LEVEL
--*/
VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject)
{   
    DEBUGP(MP_INFO, ("[EXIT]--> MPUnload \n"));
    PAGED_CODE();

    UNREFERENCED_PARAMETER(DriverObject); 

    NdisMDeregisterMiniportDriver(g_NdisMiniportDriverHandle);

    WPP_CLEANUP(DriverObject);
    DEBUGP(MP_INFO, ("[EXIT]<-- MPUnload \n"));
}

/*++
    Halt handler is called when NDIS receives IRP_MN_STOP_DEVICE,
    IRP_MN_SUPRISE_REMOVE or IRP_MN_REMOVE_DEVICE requests from the 
    PNP manager. Here, the driver should free all the resources acquired
    in MiniportInitialize and stop access to the hardware. NDIS will
    not submit any further request once this handler is invoked.

    1) Stop radio requesting thread   
    2) Release Mac object 
    3) Release Phy object
    4) free Adapter object
    5) Delete Device object

    MiniportHalt runs at IRQL = PASSIVE_LEVEL. 


Parameters:

    MiniportAdapterContext  Pointer to the Adapter

Return Value:

    None.


 IRQL: PASSIVE_LEVEL
--*/
VOID 
MPHalt(
    IN NDIS_HANDLE      MiniportAdapterContext,
    IN NDIS_HALT_ACTION HaltAction)
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER)MiniportAdapterContext;

    PAGED_CODE();

    DEBUGP(MP_INFO, ("[EXIT]--> MPHalt \n"));

	Adapter->Phy.BBContextFor11A.fCanWork = FALSE;
	Adapter->Phy.BBContextFor11B.fCanWork = FALSE;
	
    STOP_SORA_THREAD(Adapter->PHYInitThread);

    SdrMacCleanUp(&(Adapter->Mac));
    SdrPhyCleanUp(&(Adapter->Phy));

    FreeNdisPort(Adapter);

    FreeAdapter(Adapter);

    if(g_NdisDeviceHandle)
    {
        NdisDeregisterDeviceEx(g_NdisDeviceHandle);
    }

    DEBUGP(MP_INFO, ("[EXIT]<-- MPHalt \n"));
}

/*
    The MPShutdown handler restores a NIC to its initial state when
    the system is shut down.

    Argument: 
    MiniportAdapterContext: pointer to our context
    ShutdownAction:            the reason why NDIS called the shutdown function

    Return:
    

    IRQL: PASSIVE_LEVEL
*/
VOID 
MPShutdown(
    IN NDIS_HANDLE          MiniportAdapterContext,
    IN NDIS_SHUTDOWN_ACTION ShutdownAction)
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    TraceEvents(TRACE_LEVEL_WARNING, DBG_INTERRUPT, "--> MPShutdown\n");
    if (Adapter)
    {
        SdrPhyStopHardware(&Adapter->Phy);
    }
    TraceEvents(TRACE_LEVEL_WARNING, DBG_INTERRUPT, "<-- MPShutdown\n");
}

/*
Register device
*/
NDIS_STATUS NICRegisterDevice(VOID)
{
    NDIS_STATUS                     status;
    UNICODE_STRING                  sddlString;
    NDIS_DEVICE_OBJECT_ATTRIBUTES   doAttr;
    UNICODE_STRING                  devName;
    UNICODE_STRING                  symLinkName;
    PDRIVER_DISPATCH                DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    NIC_REGISTER_DEVICE_INIT_PARAMETERS(DispatchTable, devName, symLinkName);

    RtlInitUnicodeString(&sddlString, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;BU)(A;;GA;;;WD)");

    //Fill device object attributes structure
    doAttr.Header.Type          = NDIS_OBJECT_TYPE_DEVICE_OBJECT_ATTRIBUTES;
    doAttr.Header.Revision      = NDIS_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;
    doAttr.Header.Size          = sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES);
    doAttr.DeviceName           = &devName;
    doAttr.SymbolicName         = &symLinkName;
    doAttr.MajorFunctions       = &DispatchTable[0];
    doAttr.ExtensionSize        = NIC_EXTENSION_SIZE;
    doAttr.DefaultSDDLString    = &sddlString;
    doAttr.DeviceClassGuid      = NULL;

    //Register Device
    status = NdisRegisterDeviceEx(g_NdisMiniportDriverHandle, &doAttr, &g_pDeviceObj, &g_NdisDeviceHandle);
    if(status != NDIS_STATUS_SUCCESS)
    {
        DEBUGP(MP_ERROR, ("[STARTUP] NdisMRegisterDeviceEx failed with 0x%08x\n", status));
    }

    return status;
}

/*
Set registration attributes
*/
NDIS_STATUS
MpSetRegistrationAttributes(
    IN PMP_ADAPTER Adapter)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_ADAPTER_ATTRIBUTES miniportAttributes;

    //
    // First we we set the registration attributes
    //
    NdisZeroMemory(&miniportAttributes, sizeof(miniportAttributes));
    miniportAttributes.RegistrationAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES;
    miniportAttributes.RegistrationAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_1;
    miniportAttributes.RegistrationAttributes.Header.Size = sizeof(NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES);

    miniportAttributes.RegistrationAttributes.MiniportAdapterContext = Adapter;

    miniportAttributes.RegistrationAttributes.AttributeFlags =  NDIS_MINIPORT_ATTRIBUTES_BUS_MASTER;
    miniportAttributes.RegistrationAttributes.CheckForHangTimeInSeconds = CHECK_FOR_HANG_TIME_IN_SECONDS;
    miniportAttributes.RegistrationAttributes.InterfaceType = (NDIS_INTERFACE_TYPE)0;

    ndisStatus = NdisMSetMiniportAttributes(
                    Adapter->AdapterHandle, 
                    &miniportAttributes);
    if(ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("Failed to set miniport registration attributes. Status = 0x%08x\n", ndisStatus);
    }

    return ndisStatus;
}

/*
Set miniport attributes
*/
NDIS_STATUS
MpSetMiniportAttributes(
    IN PMP_ADAPTER Adapter)
{
    NDIS_STATUS                         ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_ADAPTER_ATTRIBUTES    miniportAttributes;
    NDIS_PM_CAPABILITIES                pmCapabilities;

    NdisZeroMemory(&miniportAttributes, sizeof(miniportAttributes));

    miniportAttributes.GeneralAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES;
    miniportAttributes.GeneralAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    miniportAttributes.GeneralAttributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;

    miniportAttributes.GeneralAttributes.MediaType = NdisMedium802_3;
    miniportAttributes.GeneralAttributes.PhysicalMediumType = NdisPhysicalMediumUnspecified;    
    
    miniportAttributes.GeneralAttributes.MtuSize = NIC_MAXIMUM_TOTAL_SIZE;
    miniportAttributes.GeneralAttributes.MaxXmitLinkSpeed = XMIT_SPEED;
    miniportAttributes.GeneralAttributes.MaxRcvLinkSpeed = RECV_SPEED;
    miniportAttributes.GeneralAttributes.XmitLinkSpeed = RECV_SPEED;
    miniportAttributes.GeneralAttributes.RcvLinkSpeed = RECV_SPEED;
    miniportAttributes.GeneralAttributes.MediaConnectState = MediaConnectStateConnected;
    miniportAttributes.GeneralAttributes.MediaDuplexState = MediaDuplexStateFull;
    miniportAttributes.GeneralAttributes.LookaheadSize = NIC_MAX_LOOKAHEAD;
    
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

    miniportAttributes.GeneralAttributes.MaxMulticastListSize = NIC_MAX_MCAST_LIST;
    miniportAttributes.GeneralAttributes.MacAddressLength = sizeof(DOT11_MAC_ADDRESS);

    NdisMoveMemory(
        &miniportAttributes.GeneralAttributes.PermanentMacAddress,
        Adapter->CurrentAddress, 
        sizeof(DOT11_MAC_ADDRESS));

    NdisMoveMemory(
        &miniportAttributes.GeneralAttributes.CurrentMacAddress,
        Adapter->CurrentAddress, 
        sizeof(DOT11_MAC_ADDRESS));

    miniportAttributes.GeneralAttributes.RecvScaleCapabilities = NULL;
    miniportAttributes.GeneralAttributes.AccessType = NET_IF_ACCESS_BROADCAST;
    miniportAttributes.GeneralAttributes.DirectionType = NET_IF_DIRECTION_SENDRECEIVE;
    miniportAttributes.GeneralAttributes.IfType = IF_TYPE_ETHERNET_CSMACD;
    miniportAttributes.GeneralAttributes.IfConnectorPresent = TRUE;
    miniportAttributes.GeneralAttributes.DataBackFillSize = DATA_BACK_FILL_SIZE;

    MpQuerySupportedOidsList(
        &miniportAttributes.GeneralAttributes.SupportedOidList, 
        &miniportAttributes.GeneralAttributes.SupportedOidListLength);

    ndisStatus = NdisMSetMiniportAttributes(
            Adapter->AdapterHandle,
            &miniportAttributes);
    if(ndisStatus != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("Failed to set general attributes");
    }
    return ndisStatus;
}

/*
Allocate ndis port
*/
NDIS_STATUS 
VNicAllocateNdisPort(
    IN NDIS_HANDLE MiniportAdapterHandle, 
    OUT PNDIS_PORT_NUMBER AllocatedPortNumber)
{
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_PORT_CHARACTERISTICS   portChar;

    // Call NDIS to allocate the port
    NdisZeroMemory(&portChar, sizeof(NDIS_PORT_CHARACTERISTICS));

    MP_NDIS_OBJ_INIT(
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
    portChar.SendAuthorizationState = NdisPortAuthorizationUnknown; // Ignored
    portChar.RcvAuthorizationState = NdisPortAuthorizationUnknown; // Ignored

    ndisStatus = NdisMAllocatePort(MiniportAdapterHandle, &portChar);
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        *AllocatedPortNumber = portChar.PortNumber;
    }
    return ndisStatus;
}


/*
Allocate NET_BUFFER_LIST pool
*/
NDIS_STATUS
MPAllocatePacketPool(IN PMP_ADAPTER Adapter)
{
    NDIS_STATUS                         Status = NDIS_STATUS_SUCCESS;
    NET_BUFFER_LIST_POOL_PARAMETERS     NBLPP;

    //Fill PBLPP structure
    NBLPP.Header.Type           = NDIS_OBJECT_TYPE_DEFAULT;
    NBLPP.Header.Revision       = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
    NBLPP.Header.Size           = sizeof(NET_BUFFER_LIST_POOL_PARAMETERS);
    NBLPP.ProtocolId            = NDIS_PROTOCOL_ID_DEFAULT;
    NBLPP.fAllocateNetBuffer    = TRUE;
    NBLPP.ContextSize           = 0;
    NBLPP.PoolTag               = NIC_TAG;
    NBLPP.DataSize              = 0;

    //Allocate pool
    Adapter->RecvPacketOrNetBufferListPoolHandle = 
        NdisAllocateNetBufferListPool(Adapter->AdapterHandle, &NBLPP);
    if(Adapter->RecvPacketOrNetBufferListPoolHandle == NULL)
    {
        Status = NDIS_STATUS_FAILURE;
    }

    return Status;
}



