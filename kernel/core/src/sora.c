/*++
Copyright (c) Microsoft Corporation

Module Name: Sora Library interface functions implementation.

Abstract: The file includes the function implementation of Sora Library.

History: 
          3/12/2009: Created by senxiang
--*/

#include "sora.h"
#pragma warning (disable :4305)
#pragma warning (disable :4740)

VOID 
SdrContextBind(
    IN PSDR_CONTEXT Context, 
    IN PVOID Nic,
    IN PVOID LinkLayer,
    IN PVOID Mac,
    IN PVOID Phy
    )
{
    Context->Nic        = Nic;
    Context->LinkLayer  = LinkLayer;
    Context->Mac        = Mac;
    Context->Phy        = Phy;
}

const CCHAR KiFindFirstSetLeft[256] = {
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

#if 0
#ifndef USER_MODE

VOID __TestWorkLoad(PULONG interrupts, PULONG counter, SORA_CLOCK *Clock, ULONG PMask);

VOID __SORA_FSM_ENGINE(PVOID pVoid)
{
#ifndef _M_X64
    ULONG interrupts = 0;
    ULONG counter = 0;
    KAFFINITY       PMask;
    LARGE_INTEGER   Delay = {0};
    PRKTHREAD       pKThread;
    
    __PFSM_BASE     pSMBasic = (__PFSM_BASE)pVoid;
    PSORA_FSM_STATE  pCurrentState = &(pSMBasic->__CurrentState);
    SORA_CLOCK      Clock;
    
    UNREFERENCED_PARAMETER(interrupts);
    UNREFERENCED_PARAMETER(counter);
    UNREFERENCED_PARAMETER(pCurrentState);

    Delay.QuadPart = -2 *1000 * 10; // 10ms
    PMask = pSMBasic->__Thread.Affinity;
    KeSetSystemAffinityThread (PMask);
    KeDelayExecutionThread(KernelMode, FALSE, &Delay); //dominate one cpu core
    pKThread = KeGetCurrentThread();
    KeSetPriorityThread(pKThread, HIGH_PRIORITY);
    
    SoraInitClock(&Clock, ESCAPE_FREQ);
    SoraBeginClock(&Clock);
    do
    {
        (pSMBasic->__pFSMHandlers[*pCurrentState])((PFSM_BASE)pVoid);
        if (SoraCheckClock(&Clock))
        {
            KiTryEscape(PMask);
            SoraBeginClockFromCheckPoint(&Clock);
        }  
        //__TestWorkLoad(&interrupts, &counter, &Clock, PMask);
    }while(!__FSM_NEED_TERMINATE(pSMBasic));
    
    pSMBasic->__fTerminate = __FSM_TERMINATED;
    PsTerminateSystemThread(STATUS_SUCCESS);
#endif
}
#endif
#endif

#if 0
#ifndef USER_MODE
VOID __TestWorkLoad(PULONG interrupts, PULONG counter, SORA_CLOCK *Clock, ULONG PMask)
{
#ifndef _M_X64
        ULONGLONG begin, end, interval;
        //KIRQL OldIrq;
        //short count = 1;
        
        //OldIrq = KeRaiseIrqlToDpcLevel();
        __asm
        {
            cli;
        }
        begin = __rdtsc();
        __asm 
        {
            
            mov edx, 0
loopbody:   
            add edx, 1 ;
            cmp edx, 400000;
            jnz loopbody;
           

        }
        
        if (SoraCheckClock(Clock))
        {
            KiTryEscape(PMask);
            SoraBeginClockFromCheckPoint(Clock);
        }
        end = __rdtsc();
        __asm
        {
            sti;
        }
        interval = ((end - begin) * 1000 * 1000 /(Clock->Frequency.QuadPart));
        if (interval > 1000) //150
        {
            
            (*interrupts)++;
            DbgPrint("[Perf] time=%llu us", interval);
            
        }
        //KeLowerIrql(OldIrq);
        
        (*counter)++;
        if ((*counter) > 100000)
        {
            DbgPrint("[Stat] High priority with escape: %u /100,000 \n", *interrupts );
            (*counter) = 0;
            (*interrupts) = 0;
        }
#endif		
}
#endif
#endif