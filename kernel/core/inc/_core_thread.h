/*++
Copyright (c) Microsoft Corporation

Module Name: Sora core thread management utility

Abstract: 
This header file defines structs, macros, interfaces to 
help Sora user manage their system thread and real-time 
core system thread.

--*/

#ifndef _CORE_THREAD_
#define _CORE_THREAD_
#pragma once

#if (WIN_VER >= 0x601)
#define ESCAPE_FREQ       (100*1000) //u second
#else
#define ESCAPE_FREQ       (1000*1000) //u second
#endif

#define INIT_SORA_THREAD(Thread, Entry, pContext, DA, core) \
    do \
    { \
        (Thread).pStartRoutine = (Entry); \
        (Thread).pStartContext = (pContext);\
        (Thread).DesiredAccess = (DA); \
        (Thread).fTerminate    = __CORE_THREAD_TERMINATED; \
        (Thread).Affinity      = core; \
    } while(FALSE);

/*++
START_CORE_THREAD starts the initialized thread object.

Note: 
        Thread function must call PsTerminateSystemThread before it returns.
--*/
#define START_SORA_THREAD(Thread)   \
    do  \
    { \
        NTSTATUS Status = PsCreateSystemThread( \
                                &(Thread.hThread), \
                                (Thread).DesiredAccess,\
                                NULL, NULL, NULL, \
                                ((Thread).pStartRoutine), \
                                (&(Thread).pStartContext)); \
        if (NT_SUCCESS(Status)) \
        { \
            (Thread).fTerminate = FALSE; \
             ZwClose((Thread).hThread); \
        } \
        else \
        { \
            (Thread).fTerminate = __CORE_THREAD_TERMINATED; \
        } \
    } while(FALSE); 


#define STOP_SORA_THREAD(Thread) \
    do \
    { \
        if((Thread).fTerminate == __CORE_THREAD_TERMINATED)\
            break;\
        if ((Thread).fTerminate == FALSE) \
        { \
            (Thread).fTerminate = TRUE;  \
            while ((Thread).fTerminate != __CORE_THREAD_TERMINATED) \
            { \
                _mm_pause(); \
            } \
        }\
    } while(0);

#define SORA_THREAD_STOPPED(ppContext) \
    do { \
        (CONTAINING_RECORD(ppContext, SORA_THREAD, pStartContext))->fTerminate \
            =  __CORE_THREAD_TERMINATED; \
    } while(0);

#define IS_SORA_THREAD_NEED_TERMINATE(ppContext) \
    ((CONTAINING_RECORD(ppContext, SORA_THREAD, pStartContext))->fTerminate == TRUE)

#define SORA_THREAD_AFFINITY(ppContext) \
    ((CONTAINING_RECORD(ppContext, SORA_THREAD, pStartContext))->Affinity)

#define SORA_THREAD_ROUTINE(ppContext) \
    ((CONTAINING_RECORD(ppContext, SORA_THREAD, pStartContext))->pStartRoutine)

#define SORA_THREAD_CONTEXT_PTR(T, ppContext)  \
    (*(T**)ppContext)

typedef struct __THREAD{
    HANDLE           hThread;
    ULONG            DesiredAccess;
    PKSTART_ROUTINE  pStartRoutine;
    PVOID            pStartContext;
    volatile ULONG   fTerminate;

    ULONG            fMacInDpc;
    PULONG           pDpcInterruptCnt;
    KIRQL            OldIrql;
    KAFFINITY        Affinity;
}__THREAD, *__PTHREAD, **__PPTHREAD;

typedef __THREAD SORA_THREAD, *PSORA_THREAD;

/* Sora Real Time Thread SORA_ETHREAD */
typedef __THREAD SORA_ETHREAD, *PSORA_ETHREAD;

/*
SoraEthreadInit initializes a ETHREAD object.

Parameters:
    EThread: caller provided object pointer.
    CallBackRoutine: workload callback of the real-time thread.
    Context: workload callback's running context.
    Affinity: CPU affinity mask for the thread.

Return:

Note: 
    CallBackRoutine can be called at <= DPC_LEVEL.
*/
VOID 
SoraEthreadInit(
    IN PSORA_ETHREAD    EThread, 
    IN PKSTART_ROUTINE  CallBackRoutine,
    IN PVOID            Context,
    IN KAFFINITY        Affinity);

/*
SoraEthreadStart starts the real-time thread.

Parameters:
    EThread: sora thread object pointer.

Return:
    STATUS_SUCCESS if successfully started.

Note:
*/
NTSTATUS SoraEthreadStart(IN PSORA_ETHREAD EThread);

/*
SoraEthreadStop stops the real-time thread

Parameters:
    EThread: sora thread object pointer.

Return:

Note:
    This function exits when the thread is actually terminated.
*/
VOID SoraEthreadStop(IN PSORA_ETHREAD EThread);

#endif