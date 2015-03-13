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
#pragma NDIS_PAGEABLE_FUNCTION(MPHalt)
#pragma NDIS_PAGEABLE_FUNCTION(MPUnload)
#pragma NDIS_PAGEABLE_FUNCTION(MPDeviceIoControl)
#pragma NDIS_PAGEABLE_FUNCTION(MPAllocatePacketPool)

#ifdef NDIS51_MINIPORT   
#pragma NDIS_PAGEABLE_FUNCTION(MPPnPEventNotify)
#endif

INT             MPDebugLevel = MP_INFO;
NDIS_HANDLE     NdisWrapperHandle;


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
    NDIS_STATUS                     status;
    NDIS_MINIPORT_CHARACTERISTICS   mpChar;
    UNICODE_STRING                  devName;
    UNICODE_STRING                  symLinkName;
    PDRIVER_DISPATCH                DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    WPP_INIT_TRACING(DriverObject, (PCUNICODE_STRING)RegistryPath);
  
    DEBUGP(MP_INFO, 
        ("[STARTUP]---> DriverEntry built on "__DATE__" at "__TIME__ 
        ", MP_NDIS_MAJOR_VERSION=%d, MP_NDIS_MINOR_VERSION=%d\n", 
        MP_NDIS_MAJOR_VERSION, 
        MP_NDIS_MINOR_VERSION));

    //Associate miniport driver with NDIS
    NdisMInitializeWrapper(
            &NdisWrapperHandle,
            DriverObject,
            RegistryPath,
            NULL
            );

    if(!NdisWrapperHandle){
        DEBUGP(MP_ERROR, ("[STARTUP] NdisMInitializeWrapper failed \n"));
        return NDIS_STATUS_FAILURE;
    }

    NdisZeroMemory(&mpChar, sizeof(mpChar));
    mpChar.Ndis50Chars.MajorNdisVersion          = MP_NDIS_MAJOR_VERSION;
    mpChar.Ndis50Chars.MinorNdisVersion          = MP_NDIS_MINOR_VERSION;
    mpChar.Ndis50Chars.InitializeHandler         = MPInitialize;
    mpChar.Ndis50Chars.HaltHandler               = MPHalt;
  
    mpChar.Ndis50Chars.SetInformationHandler     = MPSetInformation;
    mpChar.Ndis50Chars.QueryInformationHandler   = MPQueryInformation;
    
    mpChar.Ndis50Chars.SendPacketsHandler        = MPSendPackets;
    mpChar.Ndis50Chars.ReturnPacketHandler       = MPReturnPacket;
    mpChar.CancelSendPacketsHandler              = MPCancelSendPackets;
    mpChar.AdapterShutdownHandler                = MPShutdown;

#ifdef NDIS51_MINIPORT
    mpChar.PnPEventNotifyHandler     =  MPPnPEventNotify;
#endif

    status = NdisMRegisterMiniport(
                    NdisWrapperHandle,
                    &mpChar,
                    sizeof(NDIS_MINIPORT_CHARACTERISTICS));
    if (status != NDIS_STATUS_SUCCESS) {
        
        DEBUGP(MP_ERROR,
            ("[STARTUP]NdisMRegisterMiniport failed with status 0x%08x\n", 
            status));
        
        NdisTerminateWrapper(NdisWrapperHandle, NULL);

        return status;
    }

    NIC_REGISTER_DEVICE_INIT_PARAMETERS(DispatchTable, devName, symLinkName);

    status = 
        NdisMRegisterDevice(
            NdisWrapperHandle, 
            &devName, 
            &symLinkName,
            DispatchTable, 
            &g_pDeviceObj, 
            &g_NdisDeviceHandle);
    if (status != NDIS_STATUS_SUCCESS) 
    {
        DEBUGP(MP_ERROR,
            ("[STARTUP] NdisMRegisterDevice failed with 0x%08x\n", status));
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
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext)
{
    NDIS_STATUS             Status  = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER             Adapter = NULL;
    UINT                    index;

    DEBUGP(MP_INFO, ("[STARTUP] -->MPInitialize \n"));
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
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }
        *SelectedMediumIndex = index;

        Status = NICAllocAdapter(&Adapter);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        Adapter->AdapterHandle = MiniportAdapterHandle;
        Status = NICInitializeAdapter(Adapter, WrapperConfigurationContext);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            FreeAdapter(Adapter);
            break;
        }
      
       
        NdisMSetAttributesEx(
            MiniportAdapterHandle,
            (NDIS_HANDLE) Adapter,
            0,
#ifdef NDIS50_MINIPORT            
            NDIS_ATTRIBUTE_DESERIALIZE|
            NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS, 
#else 
            NDIS_ATTRIBUTE_DESERIALIZE,
#endif               
            NIC_INTERFACE_TYPE);

    }while(FALSE);
    
    DEBUGP(MP_INFO, ("[STARTUP] <--MPInitialize with 0x%08x\n", Status));
    return Status;
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
    IN  NDIS_HANDLE MiniportAdapterContext
    )
{
    PMP_ADAPTER Adapter         = (PMP_ADAPTER) MiniportAdapterContext;
   
    PAGED_CODE();

	DEBUGP(MP_INFO, ("[EXIT]--> MPHalt \n"));

	Adapter->Phy.BBContextFor11A.fCanWork = FALSE;
	Adapter->Phy.BBContextFor11B.fCanWork = FALSE;
      
    STOP_SORA_THREAD(Adapter->PHYInitThread);
    
    SoraKUExtKeDtor(
        &Adapter->KeAppExtObj, Adapter->TransferObj);

    SdrMacCleanUp(&(Adapter->Mac));
    SdrPhyCleanUp(&(Adapter->Phy));

    FreeAdapter(Adapter);
    
    NdisMDeregisterDevice(g_NdisDeviceHandle);

    DEBUGP(MP_INFO, ("[EXIT]<-- MPHalt \n"));
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
    IN  PDRIVER_OBJECT  DriverObject
    )
{   
    DEBUGP(MP_INFO, ("[EXIT]--> MPUnload \n"));
    PAGED_CODE();

    WPP_CLEANUP(DriverObject);
    DEBUGP(MP_INFO, ("[EXIT]<-- MPUnload \n"));
}


/*
 MPPnPEventNotify  is to handle PnP notification messages.

Parameters:
MiniportAdapterContext:      pointer to our adapter
PnPEvent:                    Self-explanatory 
InformationBuffer:           Self-explanatory 
InformationBufferLength:     Self-explanatory 

Return:

None

IRQL:   PASSIVE_LEVEL
*/

VOID MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    )
{
    if(PnPEvent == IRP_MN_START_DEVICE)
        KdPrint(("IRP_MN_START_DEVICE\n"));

#pragma warning(push)
#pragma warning(disable: 4390)
    TraceEvents( TRACE_LEVEL_INFORMATION, DBG_PNP, 
        "---> MPPnPEventNotify, PnPEvent=%d, InformationBufferLength=%d \n", 
        PnPEvent, InformationBufferLength);
    PAGED_CODE();
    TraceEvents( TRACE_LEVEL_INFORMATION, DBG_PNP, "<--- MPPnPEventNotify\n");
#pragma warning(pop)
}

/*
    MPCancelSendPackets ackets cancels the transmission of all packets that
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
MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId
    )
{
    KdPrint(("--> MPCancelSendPackets\n"));
    TraceEvents(TRACE_LEVEL_WARNING, DBG_DPC, "--> MPCancelSendPackets\n");
    TraceEvents(TRACE_LEVEL_WARNING, DBG_DPC, "<-- MPCancelSendPackets\n");
}


/*
    The MPShutdown handler restores a NIC to its initial state when
    the system is shut down.

    Argument: 
    MiniportAdapterContext: pointer to our context

    Return:
    

    IRQL: PASSIVE_LEVEL
*/
VOID 
MPShutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    TraceEvents(TRACE_LEVEL_WARNING, DBG_INTERRUPT, "--> MPShutdown\n");
    if (Adapter)
    {
        SdrPhyStopHardware(&Adapter->Phy);
    }
    TraceEvents(TRACE_LEVEL_WARNING, DBG_INTERRUPT, "<-- MPShutdown\n");

    
}


/*
Allocate packet poll
*/
NDIS_STATUS
MPAllocatePacketPool(IN PMP_ADAPTER Adapter)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    NdisAllocatePacketPool(
        &Status, 
        &Adapter->RecvPacketOrNetBufferListPoolHandle,
        NIC_RECV_POOL_SIZE,
        PROTOCOL_RESERVED_SIZE_IN_PACKET);

    return Status;
}



