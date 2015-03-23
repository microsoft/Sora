/*++
Copyright (c) Microsoft Corporation

Module Name: Device.c

Abstract: Radio manager device operation header file. 

History: 
          4/9/2009: Created by senxiang
--*/


#ifndef _PCIE_DEVICE_H_
#define _PCIE_DEVICE_H_
#include "sora.h"
//#include <ntddk.h>
#include <wdf.h>

//#include <wdm.h>



#include "__reg_file.h"
#include "__radio_man_internal.h"
#include "resource_collection.h"

#define DEVICE_IDLE_TIMEOUT             10000   // 10-sec
#define RADIO_UPDATE_INTERVAL           5000    // 5-sec
#define CLEANUP_RETRY_TIMES             8
#define CLEANUP_RETRY_INTERVAL          1000    // 1-sec

#define POOL_TAG                        ((ULONG)'radi')
//
// The device extension for the device object
//
typedef struct _DEVICE_EXTENSION {
    WDFDEVICE               Device;
    WDFINTERRUPT            Interrupt;
    WDFQUEUE                IOCtrlQueue;

    SORA_THREAD             RadioUpdateThread;
    
    SORA_RADIO_MANAGER      RadioManager;
	ULONG 					RadioManagerRef;
	ULONG					RadioMask;


	struct PROCESS_OBJECT_MONITOR* ResourceMonitor;

}  DEVICE_EXTENSION, *PDEVICE_EXTENSION;

//
// This will generate the function named PCIEGetDeviceContext to be use for
// retreiving the DEVICE_EXTENSION pointer.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, PCIEGetDeviceContext)

#pragma warning(disable:4127) // avoid conditional expression is constant error with W4

#ifdef __cplusplus
extern "C"
#endif
NTSTATUS PCIEEvtDeviceAdd(
            IN WDFDRIVER        Driver,
            IN PWDFDEVICE_INIT  DeviceInit
            );

#ifdef __cplusplus
extern "C"
#endif
NTSTATUS PCIEInitializeDeviceExtension(IN PDEVICE_EXTENSION pDevExt);

#ifdef __cplusplus
extern "C"
#endif
NTSTATUS PCIESetIdleAndWakeSettings(IN PDEVICE_EXTENSION FdoData);

#ifdef __cplusplus
extern "C"
#endif
NTSTATUS PCIEEvtDevicePrepareHardware (
            WDFDEVICE      Device,
            WDFCMRESLIST   Resources,
            WDFCMRESLIST   ResourcesTranslated
            );

#ifdef __cplusplus
extern "C"
#endif
NTSTATUS PCIEEvtDeviceReleaseHardware(
            IN  WDFDEVICE Device,
            IN  WDFCMRESLIST ResourcesTranslated
            );

#ifdef __cplusplus
extern "C"
#endif
NTSTATUS PCIEEvtDeviceD0Entry(
            IN  WDFDEVICE Device,
            IN  WDF_POWER_DEVICE_STATE PreviousState);

#ifdef __cplusplus
extern "C"
#endif
NTSTATUS PCIEEvtDeviceD0Exit(
            IN  WDFDEVICE Device,
            IN  WDF_POWER_DEVICE_STATE TargetState
            );

#ifdef __cplusplus
extern "C"
#endif
NTSTATUS PCIEEvtDeviceQueryStopRemove(
            IN  WDFDEVICE Device);


VOID PCIEDeviceShutdown(IN PDEVICE_EXTENSION pDevExt);

VOID PCIEDeviceReset(IN PDEVICE_EXTENSION pDevExt);

VOID PCIEDeviceWait(INT ms);

VOID PCIEDeviceIoCtrl(
    IN WDFQUEUE      Queue,
    IN WDFREQUEST    Request,
    IN size_t        OutputBufferLength,
    IN size_t        InputBufferLength,
    IN ULONG         IoControlCode
    );

VOID RadioUpdator(PVOID pVoid);

#endif