/*++
Copyright (c) Microsoft Corporation

Module Name: Sora Library header file for developers and researchers.

Abstract: Sora is a software-defined radio implementation, including hardware PICE board, toolkit and 
          sample application for community, developer and researchers. Users can include this header 
          file for Sora development.

Note:     If you meet with symbol redefinition error while linking with ksora.lib, make sure sora.h is 
          included before other system header file, such as ndis.h, wdm.h, ntddk.h, etc.
--*/

#ifndef _SORA_
#define _SORA_
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#define WIN9X_COMPAT_SPINLOCK
#ifdef USER_MODE
#include "uwdm.h"

#define THREAD_ENTRY __cdecl

#else
#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>

#define THREAD_ENTRY
#endif
#pragma warning (disable: 4127)

#include <emmintrin.h>

#include "const.h"
#include "func.h"
#include "soratypes.h"
#include "_reg_conf.h"
#include "_sora_regman.h"
#include "_core_thread.h"
#include "_rx_manager.h"

#include "_tx_manager2.h"
#include "_rx_stream.h"
#include "_packet_base.h"
#include "_radio_manager.h"
#include "_hw_op.h"
#include "_WARP_Control.h"
#include "_signal_cache.h"
#include "_sora_clock.h"

#include "_fsm.h"

#include "_user_mode_ext.h"
#include "_UExtK.h"

#define INLINE_ASM

VOID 
SdrContextBind(
    IN PSDR_CONTEXT Context, 
    IN PVOID Nic,
    IN PVOID LinkLayer,
    IN PVOID Mac,
    IN PVOID Phy);

#pragma region "Dump support"

VOID __ClearNewRxBuffer(PSORA_RADIO pRadio);

NTSTATUS SoraDumpNewRxBuffer(
            IN PSORA_RADIO pRadio, 
            OUT PUCHAR pBuffer, 
            IN ULONG Size, 
            OUT PULONG pWritten);

NTSTATUS __CloseDumpFile(HANDLE hFile);

NTSTATUS __DumpBuffer2File(HANDLE hFile, PUCHAR pBuffer, ULONG Size);

HANDLE __CreateDumpFile(WCHAR *wszFileName);

NTSTATUS __CloseDumpFile(HANDLE hFile);
#pragma endregion

#ifdef __cplusplus
}
#endif

#endif
