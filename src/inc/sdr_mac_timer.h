#ifndef _DOT11B_MAC_TIMER_H_
#define _DOT11B_MAC_TIMER_H_

typedef struct _MAC_TIMER{
    ULONGLONG     StartCycle;
    ULONGLONG     StopCycle;
    ULONGLONG     FrequencyCycle;
    ULONGLONG     DurationCycle;

    LARGE_INTEGER StartCounter;
    LARGE_INTEGER StopCounter;
    LARGE_INTEGER Frequency;

    ULONGLONG     OneMicroSecondCycles;
    ULONGLONG     TenMicroSecondCycles;
    ULONGLONG     FifteenMicroSecondCycles;
}MAC_TIMER, *PMAC_TIMER, **PPMAC_TIMER;


#define  GetMacTimerStartCycle(pMacTimer)                          \
_asm {                                                             \
    _asm  mov ebx, (pMacTimer)                                     \
    _asm  rdtsc                                                    \
    _asm  mov DWORD PTR [ebx], eax                                 \
    _asm  mov DWORD PTR [ebx + 4], edx                             \
}


#define GetMacTimerStopCycle(pMacTimer)                            \
_asm {                                                             \
    _asm    mov ebx, (pMacTimer)                                   \
    _asm    rdtsc                                                  \
    _asm    mov DWORD PTR [ebx + 8], eax                           \
    _asm    mov DWORD PTR [ebx + 12], edx                          \
}

#define SDR_MAC_TIMER_SET_TIME(_pMacTimer, Time)				   \
{                                                                  \
	GetMacTimerStopCycle(_pMacTimer);                              \
	(_pMacTimer)->StopCycle += (Time);                             \
}

#define SDR_MAC_TIMER_CHECK_CURRENT_TIME(_pMacTimer)               \
{                                                                  \
	GetMacTimerStartCycle(_pMacTimer);                             \
}

#define SDR_MAC_TIMER_IS_TIMEOUT(_pMacTimer)     				   \
	(((_pMacTimer)->StartCycle >= (_pMacTimer)->StopCycle))

#define SDR_MAC_TIEMR_WAIT_UNTIL_TIMEOUT(_pMacTimer, _MicroSeconds)\
{                                                                  \
    SDR_MAC_TIMER_SET_TIME(_pMacTimer, _pMacTimer);                \
                                                                   \
    do                                                             \
    {                                                              \
        _asm                                                       \
        {                                                          \
            _asm pause                                             \
        }                                                          \
    } while (!SDR_MAC_TIMER_IS_TIMEOUT(_pMacTimer));               \
}


VOID
InitializeMacTimer(PMAC_TIMER pMacTimer);


#endif