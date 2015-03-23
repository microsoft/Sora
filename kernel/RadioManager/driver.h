/*++
Copyright (c) Microsoft Corporation

Module Name: Driver.h

Abstract: Radio manager driver main file header. It includes macros and function declaration.

History: 
          4/9/2009: Created by senxiang
--*/

#ifndef _DRIVER_H
#define _DRIVER_H
#include "sora.h"
//#include <ntddk.h>
#include <wdf.h>

//#include <wdm.h>

NTSTATUS DriverEntry(
            IN PDRIVER_OBJECT  DriverObject,
            IN PUNICODE_STRING RegistryPath);

VOID PCIEEvtDriverContextCleanup(IN WDFDRIVER Driver);

#endif 