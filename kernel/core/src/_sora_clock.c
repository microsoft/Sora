#include "sora.h"
#include "_sora_clock.h"

#ifndef USER_MODE

/*It is slow, and should be limited to run at one CPU */
VOID SoraInitClock(IN PSORA_CLOCK pClock, ULONG Interval)
{
    KeQueryPerformanceCounter(&pClock->Frequency);
#if (WIN_VER >= 0x601)
    pClock->DurationCycle = (ULONGLONG) (pClock->Frequency.QuadPart * Interval / (1000)); 
#else
	pClock->DurationCycle = (ULONGLONG) (pClock->Frequency.QuadPart * Interval / (1000 * 1000)); 
#endif
}

VOID SoraSpinSleep(ULONG MicroSeconds, PLARGE_INTEGER Start)
{
    LARGE_INTEGER Frequency, End, Ticks;
    
    KeQueryPerformanceCounter(&Frequency);
	Ticks.QuadPart = MicroSeconds * Frequency.QuadPart / (1000 * 1000);

    if (MicroSeconds > 1000) return;

    LARGE_INTEGER Cur;
    while (Cur = KeQueryPerformanceCounter(NULL)
        , Cur.QuadPart - Start->QuadPart < Ticks.QuadPart)
    {
        _mm_pause();
    }
    DbgPrint("SoraSpinSleep finished at %I64d\n", Cur.QuadPart);

    return;
}

#endif 
