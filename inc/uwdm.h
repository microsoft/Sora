/*++
Copyright (c) Microsoft Corporation

Module Name: Sora WDM header for user mode compilation, and linking.

Abstract: This header file redefines WDM, DDK macros, functions for user mode compilation, linking
          and test. 
--*/

#ifndef _UWDM_

#define _UWDM_
#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include "soratypes.h"

#define MmNonCached                                         0
#define MmCached                                            1
#define MmWriteCombined                                     2

#define PAGED_CODE()

#define PsCreateSystemThread(...)                           0
#define PsTerminateSystemThread(...)
#define MmMapIoSpace(...)                                   0
#define KeSetSystemAffinityThread(pmask)                    UNREFERENCED_PARAMETER(pmask)
    
#define KeDelayExecutionThread(...)
#define IoGetDeviceObjectPointer(...)                       0
#define KeInitializeEvent(...)
#define IoBuildDeviceIoControlRequest(...)                  0
#define KeWaitForSingleObject(...)

#ifdef _VERBOSE_
	#define KdPrint(x)                                          printf x
	#define DbgPrint                                            printf
#else
	#define KdPrint(x)                                          
	#define DbgPrint                                            
#endif

#define IoGetNextIrpStackLocation(...)                      0
#define RtlInitUnicodeString(...)       
#define KeReleaseSpinLock(...)

#define IoCallDriver(...)                                   0
#define KeAcquireSpinLock(a, b)                             UNREFERENCED_PARAMETER(b)
#define KeInitializeSpinLock(...)
#define ObDereferenceObject(...)
#define ObReferenceObject(...)
#define MmUnmapIoSpace(...)                                 ((void*)0)  /* prevent warning C4390 */

#define IoCreateNotificationEvent(...)                      0
#define KeSetEvent(...)                                     
#define ZwClose(...)                                        0

#define InitializeListHead(pHead)                 \
    do { (pHead)->Flink = (pHead); (pHead)->Blink = (pHead); } while(0);

#define IsListEmpty(pHead) \
    ((pHead)->Flink == (pHead))

__inline VOID
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
    
}
  
__inline PLIST_ENTRY RemoveHeadList(IN PLIST_ENTRY ListHead)
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}
__inline VOID InsertHeadList(
            IN PLIST_ENTRY ListHead,
            IN PLIST_ENTRY Entry
           )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}

__inline BOOLEAN
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

#define ASSERT(x)                       (assert(x))

#define IN
#define OUT
#define KernelMode

#define FALSE                           0
#define TRUE                            1

typedef VOID (*PKSTART_ROUTINE) (IN PVOID Context);

#define KIRQL                           long
#define VOID                            void

#define KSPIN_LOCK                      long
#define PHYSICAL_ADDRESS                LARGE_INTEGER

#define NTSTATUS                        long

#define STATUS_SUCCESS                  0
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define READ_REGISTER_ULONG(Register)          (*(volatile ULONG *)(Register))
#define WRITE_REGISTER_ULONG(Register, Value)  (*(volatile ULONG *)(Register) = (Value))


#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))
#define PRKTHREAD                       ULONG
#define KeGetCurrentThread(...)         (0)
#define KeSetPriorityThread(...)        (0)
#define KeBugCheck(...)   
#define ExInitializeNPagedLookasideList(...) 

#define ExDeleteNPagedLookasideList(...)
#define ExInitializeFastMutex(...)
#define KeGetCurrentIrql()            0

typedef struct _FAST_MUTEX
{
    unsigned __int32    unknown[8];
} FAST_MUTEX, *PFAST_MUTEX;

typedef struct _MDL 
{
    struct _MDL *Next;
    short MdlSize;
    short MdlFlags;
    void * Process;
    ULONG *MappedSystemVa;
    ULONG *StartVa;
    ULONG ByteCount;
    ULONG ByteOffset;
} MDL;
typedef MDL *PMDL;

#ifdef _M_X64 
typedef unsigned __int64 ULONG_PTR;
#else
typedef unsigned long   ULONG_PTR;
#endif 

typedef ULONG NPAGED_LOOKASIDE_LIST, *PNPAGED_LOOKASIDE_LIST;

typedef char            *PCHAR;
typedef unsigned char   *PUCHAR;
typedef unsigned long   *PULONG;
typedef void            *PVOID;
typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;

typedef unsigned long   ULONG;
typedef unsigned long   DWORD;
typedef long            LONG;
typedef long            *PFILE_OBJECT;
typedef long            *PDEVICE_OBJECT;
typedef unsigned short  *UNICODE_STRING;
typedef HANDLE          KEVENT;
typedef long            IRP, *PIRP;

typedef PVOID           PKEVENT;
typedef struct __IO_STACK_LOCATION
{
    void        *FileObject;
}IO_STACK_LOCATION, *PIO_STACK_LOCATION;

typedef struct __IO_STATUS_BLOCK
{
    long Status;
}IO_STATUS_BLOCK;

 typedef HANDLE 	 KMUTEX;
 typedef HANDLE 	 KSEMAPHORE;
 typedef int		 KPRIORITY;
 typedef PVOID		 POBJECT_ATTRIBUTES;
 
 __inline PHYSICAL_ADDRESS MmGetPhysicalAddress(PVOID va)
 {
     PHYSICAL_ADDRESS    PhysicalAddress      = {0, 0};
     UNREFERENCED_PARAMETER(va);
     PhysicalAddress.QuadPart = (UPOINTER)va;
     return PhysicalAddress;
 }

__inline PVOID ExAllocateFromNPagedLookasideList(PVOID p)
{
    UNREFERENCED_PARAMETER(p);
    return malloc(1024);
}

__inline void ExFreeToNPagedLookasideList(PVOID p, PVOID q)
{
    UNREFERENCED_PARAMETER(p);
    free(q);
}

__inline PVOID MmAllocateContiguousMemorySpecifyCache(ULONG size, ...)       
{
    return malloc(size);
}
__inline void MmFreeContiguousMemorySpecifyCache(PVOID base, ...)
{
    free(base);
}


__inline void DbgBreakPoint(){}
#endif