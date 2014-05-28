#pragma once

#ifdef USER_MODE
#include <Windows.h>

typedef struct _TIMINGINFO
{
    LARGE_INTEGER ti_start, ti_end;
} TIMINGINFO, *PTIMINGINFO;

__forceinline void TimerStart(PTIMINGINFO pti)
{
    QueryPerformanceCounter(&(pti->ti_start));   
}

__forceinline void TimerStop(PTIMINGINFO pti)
{
    QueryPerformanceCounter(&(pti->ti_end));
}

__forceinline double TimerRead(PTIMINGINFO pti)
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return ((double)(pti->ti_end.QuadPart - pti->ti_start.QuadPart)) /
        freq.QuadPart * 1000.0;
}

#endif
