/*++
Copyright (c) Microsoft Corporation

Module Name: Driver.c

Abstract: Radio manager driver main file. 

History: 
          4/9/2009: Created by senxiang
--*/

#include "driver.h"
#include "device.h"
#include "trace.h"

#ifdef EVENT_TRACING
#include "driver.tmh"
#endif

#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, PCIEEvtDriverContextCleanup)

/*++

Routine Description:

Driver initialization entry point.
This entry point is called directly by the I/O system.

Arguments:

DriverObject - pointer to the driver object

RegistryPath - pointer to a unicode string representing the path,
to driver-specific key in the registry.

Return Value:

NTSTATUS    - if the status value is not STATUS_SUCCESS,
the driver will get unloaded immediately.

--*/
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG       Config;
    WDF_OBJECT_ATTRIBUTES   Attributes;

    WPP_INIT_TRACING(DriverObject,RegistryPath);
    
    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "--> DriverEntry \n");

    WDF_DRIVER_CONFIG_INIT( &Config, PCIEEvtDeviceAdd );
    
    WDF_OBJECT_ATTRIBUTES_INIT(&Attributes);
    Attributes.EvtCleanupCallback = PCIEEvtDriverContextCleanup;
    
    Status = WdfDriverCreate( 
                DriverObject,
                RegistryPath,
                &Attributes,
                &Config,
                WDF_NO_HANDLE);

    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "<-- DriverEntry \n");
    return Status;
}

/*++
Routine Description:

    Free all the resources allocated in DriverEntry.

Arguments:

    Driver - handle to a WDF Driver object.

Return Value:

    VOID.

--*/

VOID PCIEEvtDriverContextCleanup( IN WDFDRIVER Driver)
{
    UNREFERENCED_PARAMETER(Driver);
    
    PAGED_CODE ();

    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "--> PCIEEvtDriverContextCleanup \n");

    WPP_CLEANUP( WdfDriverWdmGetDriverObject( Driver ) );

    TraceEvents(
        TRACE_LEVEL_INFORMATION, 
        DBG_INIT, 
        "<-- PCIEEvtDriverContextCleanup \n");
}