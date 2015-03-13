/*++
Copyright (c) Microsoft Corporation

Module Name: _sora_clock.h

Abstract: Clock for accurate timing. Only for x86 system.Code should be only run at one single CPU.

--*/

#ifndef _SORA_CLOCK_H
#define _SORA_CLOCK_H
#pragma once

typedef struct _SORA_CLOCK
{
    ULONGLONG       StartCycle;
    ULONGLONG       StopCycle;    
    ULONGLONG       DurationCycle;
    LARGE_INTEGER   Frequency;
} SORA_CLOCK, *PSORA_CLOCK;

__inline
VOID SoraBeginClock(IN PSORA_CLOCK pClock)
{
    pClock->StartCycle = __rdtsc();
}

__inline 
VOID SoraBeginClockFromCheckPoint(IN PSORA_CLOCK pClock)
{
    pClock->StartCycle = pClock->StopCycle;
}

__inline 
BOOLEAN SoraCheckClock(IN PSORA_CLOCK pClock)
{
    pClock->StopCycle = __rdtsc();
    return (BOOLEAN)((pClock->StopCycle - pClock->StartCycle) >= pClock->DurationCycle);
}

#ifndef USER_MODE
VOID SoraInitClock(IN PSORA_CLOCK pClock, ULONG Interval);
VOID SoraSpinSleep(ULONG MicroSeconds, PLARGE_INTEGER Start);
#endif

#endif