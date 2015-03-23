/*++
Copyright (c) Microsoft Corporation

Module Name: Device.c

Abstract: Radio manager device main file. 

History: 
          4/9/2009: Created by senxiang
--*/
#include <initguid.h>
#include "device.h"
#include "IsrDpc.h"
#include "trace.h"
#ifdef EVENT_TRACING
#include "device.tmh"
#endif

#pragma alloc_text (PAGE, PCIEEvtDeviceAdd)
#pragma alloc_text (PAGE, PCIEEvtDevicePrepareHardware)
#pragma alloc_text (PAGE, PCIEEvtDeviceReleaseHardware)
#pragma alloc_text (PAGE, PCIEEvtDeviceD0Entry)
#pragma alloc_text (PAGE, PCIEEvtDeviceD0Exit)
#pragma alloc_text (PAGE, PCIESetIdleAndWakeSettings)
#pragma alloc_text (PAGE, PCIEInitializeDeviceExtension)

/*++

Routine Description:

EvtDeviceAdd is called by the framework in response to AddDevice
call from the PnP manager. Here the driver should register all the
PNP, power and Io callbacks, register interfaces and allocate other
software resources required by the Device. The driver can query
any interfaces or get the config space information from the bus driver
but cannot access hardware registers or initialize the Device.

Arguments:

Return Value:

--*/
NTSTATUS PCIEEvtDeviceAdd(
            IN WDFDRIVER        Driver,
            IN PWDFDEVICE_INIT  DeviceInit
            )
{
    NTSTATUS                     Status = STATUS_SUCCESS;
    WDF_PNPPOWER_EVENT_CALLBACKS PnpPowerCallbacks;
    WDF_OBJECT_ATTRIBUTES        Attributes;
    WDFDEVICE                    Device;
    PDEVICE_EXTENSION            pDevExt = NULL;
    UNICODE_STRING               DeviceName;
    UNICODE_STRING               DeviceSymbolLink;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();
    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "--> PCIEEvtDeviceAdd \n");
    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);

    //
    // Zero out the PnpPowerCallbacks structure.
    //
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&PnpPowerCallbacks);

    // Set Callbacks for any of the functions we are interested in.
    // If no callback is set, Framework will take the default action
    // by itself.
    //
    PnpPowerCallbacks.EvtDevicePrepareHardware = PCIEEvtDevicePrepareHardware;
    PnpPowerCallbacks.EvtDeviceReleaseHardware = PCIEEvtDeviceReleaseHardware;

    //
    // These two callbacks set up and tear down hardware state that must be
    // done every time the Device moves in and out of the D0-working state.
    //
    PnpPowerCallbacks.EvtDeviceQueryRemove     = PCIEEvtDeviceQueryStopRemove;
	PnpPowerCallbacks.EvtDeviceQueryStop       = PCIEEvtDeviceQueryStopRemove;
    PnpPowerCallbacks.EvtDeviceD0Entry         = PCIEEvtDeviceD0Entry;
    PnpPowerCallbacks.EvtDeviceD0Exit          = PCIEEvtDeviceD0Exit;

    //
    // Register the PnP Callbacks..
    //
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &PnpPowerCallbacks);

    //
    // Initialize Fdo Attributes.
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attributes, DEVICE_EXTENSION);
    //
    // By opting for SynchronizationScopeDevice, we tell the framework to
    // synchronize callbacks events of all the objects directly associated
    // with the Device. In this driver, we will associate queues and
    // and DpcForIsr. By doing that we don't have to worrry about synchronizing
    // access to Device-context by Io Events and DpcForIsr because they would
    // not concurrently ever. Framework will serialize them by using an
    // internal Device-lock.
    //
    Attributes.SynchronizationScope = WdfSynchronizationScopeNone;
    RtlInitUnicodeString(&DeviceName, SORA_DEVICE_NAME);
    RtlInitUnicodeString(&DeviceSymbolLink, SORA_DEVICE_SYMBOL_NAME);

    do{
        Status = WdfDeviceInitAssignName(
                    DeviceInit,
                    &DeviceName);
        if (!NT_SUCCESS(Status))
        {
            break;
        }

        Status = WdfDeviceCreate( &DeviceInit, &Attributes, &Device );
        if (!NT_SUCCESS(Status))
        {
            break;
        }

        Status = WdfDeviceCreateSymbolicLink(Device, &DeviceSymbolLink);
        if (!NT_SUCCESS(Status)) 
        {
            break;
        }

        Status = WdfDeviceCreateDeviceInterface( Device, (LPGUID) &GUID_PCIE_INTERFACE, NULL);
        if (!NT_SUCCESS(Status)) 
        {
            break;
        }
        
        pDevExt = PCIEGetDeviceContext(Device);
        pDevExt->Device = Device;

        //
        // set idle may cause the PCI-E bus stop responsing after the idle time, so mark this setting here
        //
        //PCIESetIdleAndWakeSettings(pDevExt);
        
        Status = PCIEInitializeDeviceExtension(pDevExt);
        if (!NT_SUCCESS(Status)) 
        {   
            TraceEvents(
                TRACE_LEVEL_FATAL, 
                DBG_INIT, 
                "PCIEInitializeDeviceExtension Failed with %08X\n", Status);
            break;
        }

    } while(FALSE);

    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "<-- PCIEEvtDeviceAdd \n");
    return Status;
}

/*++
Routine Description:

This routine is called by EvtDeviceAdd. Here the device context is
initialized and all the software resources required by the device is
allocated.

Arguments:

pDevExt     Pointer to the Device Extension

Return Value:

NTSTATUS

--*/

NTSTATUS PCIEInitializeDeviceExtension(IN PDEVICE_EXTENSION pDevExt)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    WDF_IO_QUEUE_CONFIG  QueueConfig;
    WDF_IO_QUEUE_CONFIG  InternalQueueConfig;

    PAGED_CODE();

    //RtlZeroMemory(pDevExt, sizeof(DEVICE_EXTENSION));

    WDF_IO_QUEUE_CONFIG_INIT ( &QueueConfig, WdfIoQueueDispatchSequential);
    QueueConfig.EvtIoDeviceControl = PCIEDeviceIoCtrl;
    do 
    {
        Status = WdfIoQueueCreate( 
                    pDevExt->Device, 
                    &QueueConfig, 
                    WDF_NO_OBJECT_ATTRIBUTES, 
                    &pDevExt->IOCtrlQueue );
        if(!NT_SUCCESS(Status)) 
        {
            break;
        }

        Status = WdfDeviceConfigureRequestDispatching( 
                    pDevExt->Device, 
                    pDevExt->IOCtrlQueue, 
                    WdfRequestTypeDeviceControl);
        if(!NT_SUCCESS(Status)) 
        {
            break;
        }

        Status = PCIEInterruptCreate(pDevExt);
        if(!NT_SUCCESS(Status)) 
        {
            break;
        }
    } while(FALSE);

	pDevExt->RadioManagerRef = 0;
	pDevExt->RadioMask = 0;
	
    return Status;
}

/*++
Routine Description:

Called by EvtDeviceAdd to set the idle and wait-wake policy. Registering this policy
causes Power Management Tab to show up in the Device manager. By default these
options are enabled and the user is provided control to change the settings.

Return Value:

NTSTATUS - Failure Status is returned if the Device is not capable of suspending
or wait-waking the machine by an external event. Framework checks the
capability information reported by the bus driver to decide whether the Device is
capable of waking the machine.

--*/
NTSTATUS
PCIESetIdleAndWakeSettings(IN PDEVICE_EXTENSION FdoData)
{
    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS IdleSettings;
    WDF_DEVICE_POWER_POLICY_WAKE_SETTINGS WakeSettings;
    NTSTATUS    Status = STATUS_SUCCESS;

    PAGED_CODE();
    
    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "--> PCIESetIdleAndWakeSettings \n");
    do
    {
        WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&IdleSettings, IdleCanWakeFromS0);
        IdleSettings.IdleTimeout = DEVICE_IDLE_TIMEOUT; 

        Status = WdfDeviceAssignS0IdleSettings(FdoData->Device, &IdleSettings);
        if ( !NT_SUCCESS(Status)) 
        {
            break;
        }
    
        WDF_DEVICE_POWER_POLICY_WAKE_SETTINGS_INIT(&WakeSettings);
        Status = WdfDeviceAssignSxWakeSettings(FdoData->Device, &WakeSettings);
        if (!NT_SUCCESS(Status)) 
        {
            break;
        }
    } while (FALSE);
    
    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "<-- PCIESetIdleAndWakeSettings \n");
    return Status;
}

/*++

Routine Description:

Performs whatever initialization is needed to setup the Device, setting up
a DMA channel or mapping any I/O port resources.  This will only be called
as a Device starts or restarts, not every time the Device moves into the D0
state.  Consequently, most hardware initialization belongs elsewhere.

Arguments:

Device - A handle to the WDFDEVICE

Resources - The raw PnP resources associated with the Device.  Most of the
time, these aren't useful for a PCI Device.

ResourcesTranslated - The translated PnP resources associated with the
Device.  This is what is important to a PCI Device.

Return Value:

NT Status code - failure will result in the Device stack being torn down

--*/

NTSTATUS
PCIEEvtDevicePrepareHardware (
    WDFDEVICE      Device,
    WDFCMRESLIST   Resources,
    WDFCMRESLIST   ResourcesTranslated
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    HRESULT             hRes;
    PDEVICE_EXTENSION   pDevExt;
    ULONG               Index, Count = 0;
    ULONG               RegPhyAddressLength  = 0;
    PHYSICAL_ADDRESS    RegPhyAddressBase    = {0, 0};

    PCM_PARTIAL_RESOURCE_DESCRIPTOR  pResDesc;

    UNREFERENCED_PARAMETER(Resources);
    
    PAGED_CODE();

    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "--> PCIEEvtDevicePrepareHardware \n");

    do 
    {
        pDevExt = PCIEGetDeviceContext(Device);
        
        INIT_SORA_THREAD(
            pDevExt->RadioUpdateThread, 
            RadioUpdator, 
            pDevExt, 
            SYNCHRONIZE | GENERIC_ALL | STANDARD_RIGHTS_ALL, 
            CORE_ANY);

        for (Index=0; Index < WdfCmResourceListGetCount(ResourcesTranslated); Index++) 
        {
            pResDesc = WdfCmResourceListGetDescriptor(ResourcesTranslated, Index);
            if (!pResDesc)
            {
                Status = STATUS_DEVICE_CONFIGURATION_ERROR;
                break;
            }
            switch(pResDesc->Type) 
            {
            case CmResourceTypeMemory:
                RegPhyAddressLength = pResDesc->u.Memory.Length;
                RegPhyAddressBase = pResDesc->u.Memory.Start;
                TraceEvents(
                        TRACE_LEVEL_INFORMATION, 
                        DBG_INIT, 
                        "Hardware Memory Resource %08x%08x, Length = %08x\n", 
                        RegPhyAddressBase.HighPart, 
                        RegPhyAddressBase.LowPart, 
                        RegPhyAddressLength
                        );
                Count++;
                break;
            default: //Currently ignore other resources
                break;
            }
        }

        if (1 != Count)
        {
            Status =  STATUS_DEVICE_CONFIGURATION_ERROR;
            break;
        }

        hRes = SoraInitRadioManager2(
            RegPhyAddressBase, 
            RegPhyAddressLength, 
            &pDevExt->RadioManager);
        if (FAILED(hRes))
        {
            TraceEvents(
                TRACE_LEVEL_ERROR, 
                DBG_INIT, 
                "SoraInitRadioManager failed \n");
            Status =  STATUS_DEVICE_CONFIGURATION_ERROR;
            break;
        }        

		pDevExt->ResourceMonitor = AllocateProcessObjectMonitor();
		if (!pDevExt) {
			Status = STATUS_UNSUCCESSFUL;
			break;
		}		
    } while(FALSE);

    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "<-- PCIEEvtDevicePrepareHardware \n");

    return Status;
}

VOID RadioUpdator(PVOID pVoid)
{
    PDEVICE_EXTENSION pDevExt = SORA_THREAD_CONTEXT_PTR(DEVICE_EXTENSION, pVoid);
    LARGE_INTEGER       delay;
    PSORA_RADIO_MANAGER pRadioMgr = &pDevExt->RadioManager;
    
    delay.QuadPart = WDF_REL_TIMEOUT_IN_MS(RADIO_UPDATE_INTERVAL);
    
    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "--> RadioUpdator Started \n");
    do
    {
        ULONG i;
        for ( i = 0; i < pRadioMgr->__radio_count; i++)
        {
            SORA_HW_READ_RADIO_POWER_STATUS(&pRadioMgr->__radio_pool[i]);
        }
        KdPrint(("Radio status Updator running ....\n"));
        KeDelayExecutionThread(KernelMode, TRUE, &delay);
    } while(!IS_SORA_THREAD_NEED_TERMINATE(pVoid));
    
    SORA_THREAD_STOPPED(pVoid);
    
    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "--> RadioUpdator Stopped \n");
    KdPrint(("Radio status Updator stopped \n"));
    PsTerminateSystemThread(STATUS_SUCCESS);
}

/*++

Routine Description:

Unmap the resources that were mapped in PCIEEvtDevicePrepareHardware.
This will only be called when the Device stopped for resource rebalance,
surprise-removed or query-removed.

Arguments:

Device - A handle to the WDFDEVICE

ResourcesTranslated - The translated PnP resources associated with the
Device.  This is what is important to a PCI Device.

Return Value:

NT Status code - failure will result in the Device stack being torn down

--*/

NTSTATUS PCIEEvtDeviceReleaseHardware(
            IN  WDFDEVICE Device,
            IN  WDFCMRESLIST ResourcesTranslated
            )
{
    PDEVICE_EXTENSION   pDevExt         = NULL;
    NTSTATUS            Status          = STATUS_SUCCESS;
    PSORA_RADIO_MANAGER pRadioManager   = NULL;
    HRESULT             hRes;
    INT                 Retry = 0;
    LARGE_INTEGER       delay;
    
    
    delay.QuadPart = WDF_REL_TIMEOUT_IN_MS(CLEANUP_RETRY_INTERVAL);

    UNREFERENCED_PARAMETER(ResourcesTranslated);

    PAGED_CODE();

    pDevExt         = PCIEGetDeviceContext(Device);
    pRadioManager   = &pDevExt->RadioManager;

	if (pDevExt->ResourceMonitor) {
		ReleaseProcessObjectMonitor(pDevExt->ResourceMonitor);
		pDevExt->ResourceMonitor = NULL;
	}
    
    STOP_SORA_THREAD(pDevExt->RadioUpdateThread);
    do 
    {
        hRes = SoraCleanupRadioManager2(&pDevExt->RadioManager);
        Retry++;
        if (Retry > 1)
        {
            KeDelayExecutionThread(KernelMode, TRUE, &delay);
        }
    } while (FAILED(hRes) && Retry < CLEANUP_RETRY_TIMES);

    if (FAILED(hRes))
    {
        Status = STATUS_DEVICE_BUSY;
    }
    return Status;
}

/*++

Routine Description:

This routine prepares the Device for use.  It is called whenever the Device
enters the D0 state, which happens when the Device is started, when it is
restarted, and when it has been powered off.

Note that interrupts will not be enabled at the time that this is called.
They will be enabled after this callback completes.

This function is not marked pageable because this function is in the
Device power up path. When a function is marked pagable and the code
section is paged out, it will generate a page fault which could impact
the fast resume behavior because the client driver will have to wait
until the system drivers can service this page fault.

Arguments:

Device  - The handle to the WDF Device object

PreviousState - The state the Device was in before this callback was invoked.

Return Value:

NTSTATUS

Success implies that the Device can be used.

Failure will result in the    Device stack being torn down.

--*/
NTSTATUS PCIEEvtDeviceD0Entry(
    IN  WDFDEVICE Device,
    IN  WDF_POWER_DEVICE_STATE PreviousState
    )
{
    PDEVICE_EXTENSION   pDevExt;
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER(PreviousState);

    return STATUS_SUCCESS;
}

/*++

Routine Description:

This routine undoes anything done in PCIEEvtDeviceD0Entry.  It is called
whenever the Device leaves the D0 state, which happens when the Device
is stopped, when it is removed, and when it is powered off.

The Device is still in D0 when this callback is invoked, which means that
the driver can still touch hardware in this routine.

Note that interrupts have already been disabled by the time that this
callback is invoked.

Arguments:

Device  - The handle to the WDF Device object

TargetState - The state the Device will go to when this callback completes.

Return Value:

Success implies that the Device can be used.  Failure will result in the
Device stack being torn down.

--*/
NTSTATUS PCIEEvtDeviceD0Exit(
            IN  WDFDEVICE Device,
            IN  WDF_POWER_DEVICE_STATE TargetState
            )
{
    PDEVICE_EXTENSION   pDevExt;

    PAGED_CODE();

    pDevExt = PCIEGetDeviceContext(Device);

    switch (TargetState) {
    case WdfPowerDeviceD1:
    case WdfPowerDeviceD2:
    case WdfPowerDeviceD3:

        //
        // Fill in any code to save hardware state here.
        //

        //
        // Fill in any code to put the Device in a low-power state here.
        //
        break;

    case WdfPowerDevicePrepareForHibernation:

        //
        // Fill in any code to save hardware state here.  Do not put in any
        // code to shut the Device off.  If this Device cannot support being
        // in the paging path (or being a parent or grandparent of a paging
        // path Device) then this whole case can be deleted.
        //

        break;

    case WdfPowerDeviceD3Final:
    default:

        //
        // Reset the hardware, as we're shutting down for the last time.
        //
        
        break;
    }

    return STATUS_SUCCESS;

}

NTSTATUS PCIEEvtDeviceQueryStopRemove(IN WDFDEVICE Device) {
	
	PDEVICE_EXTENSION pDevExt;
	pDevExt = PCIEGetDeviceContext(Device);
	if (pDevExt->RadioManagerRef ||
		pDevExt->RadioMask)
		return STATUS_DEVICE_BUSY;
	return STATUS_SUCCESS;
}
