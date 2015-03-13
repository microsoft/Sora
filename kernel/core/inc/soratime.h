#pragma once

#include <assert.h>
#include <sora.h>

#ifdef USER_MODE
#define TIME_ONE_MILLION 1000000
#define TIME_ONE_GIGA    1000000000

// Note: below declaration are orginal found in <intrin.h> in VS2010 include directory.
// However the header file is missing in WinDDK\7600.16385.1, so copy here to remove the dependency
extern "C" extern void __cpuid(int CPUInfo[4], int InfoType);

typedef struct tagTimeStampInfo {
	int   use_rdtsc;
	LARGE_INTEGER    CPUFreq; 
	unsigned __int64 TickPerSecond; 
} TIMESTAMPINFO, *PTIMESTAMPINFO;

typedef struct tagSoraTimeofDayInfo {
	unsigned int hh;
	unsigned int mm;
	unsigned int ss;
	unsigned int ms;
	unsigned int us;
} SORATIMEOFDAYINFO, *PSORATIMEOFDAYINFO;

// Type of return value of __cpuid intrinsic
// The meaning of the feature information value, the value of EDX which is written to CPUInfo[3], when the InfoType argument is 1.
// ref: http://msdn.microsoft.com/en-us/library/hskdteyh%28v=vs.100%29.aspx
typedef union {
    struct { // low order
        unsigned int FPU:1;                  // 0
        unsigned int VME:1;                  // 1
        unsigned int DE:1;                   // 2
        unsigned int PSE:1;                  // 3
        unsigned int TSC:1;                  // 4
        unsigned int MSR:1;                  // 5
        unsigned int PAE:1;                  // 6
        unsigned int MCE:1;                  // 7
        unsigned int CX8:1;                  // 8
        unsigned int APIC:1;                 // 9
        unsigned int reserved1:1;            // 10
        unsigned int SEP:1;                  // 11
        unsigned int MTRR:1;                 // 12
        unsigned int PGE:1;                  // 13
        unsigned int MCA:1;                  // 14
        unsigned int CMOV:1;                 // 15
        unsigned int PAT:1;                  // 16
        unsigned int PSE36:1;                  // 17     
        unsigned int PSN:1;                  // 18
        unsigned int CFLSH:1;                // 19     
        unsigned int Reserved2:1;            // 20
        unsigned int DS:1;                   // 21
        unsigned int ACPI:1;                 // 22
        unsigned int MMX:1;                  // 23
        unsigned int FXSR:1;                 // 24
        unsigned int SSE:1;                  // 25
        unsigned int SSE2:1;                 // 26     
        unsigned int SS:1;                   // 27
        unsigned int HTT:1;                  // 28     
        unsigned int TM:1;                   // 29
        unsigned int Reserved3:1;            // 30
        unsigned int PBE:1;                  // 31
    };
    unsigned int w;
} EDX1b;

// Determine whether current CPU support Time Stamp Counter (TSC) instructions
// Parameters
//     None
// Return value
//     true if suporting, false otherwise
FINL 
bool ExistTSC()
{
    int CPUInfo[4];
    EDX1b edx;
    __cpuid(CPUInfo, 1);
    edx.w = CPUInfo[3];
    return (bool)edx.TSC;
}

// Set the method of TIMESTAMPINFO object to specified method
// Parameters
//     pInfo    - pointer to the TIMESTAMPINFO object
//     use_tsc  - true means use TSC, false means not
// Return value
//     original method of TIMESTAMPINFO object
FINL 
int	SoraSetTimestampMethod (PTIMESTAMPINFO pInfo, int use_tsc )
{
	if ( use_tsc == pInfo->use_rdtsc ) 
		return pInfo->use_rdtsc;

	int old = pInfo->use_rdtsc;
	if ( use_tsc ) {
		if (pInfo->TickPerSecond == (ULONGLONG)-1) {
			// no tsc available or not initialized
			return 0;
		} 		
	} 

	pInfo->use_rdtsc = use_tsc;
	return old;
}

// Intialize TIMESTAMPINFO object
// Parameters
//     pInfo    - pointer to the TIMESTAMPINFO object
//     use_tsc  - true means use TSC, false means not
// Return value
//     none
FINL
void InitializeTimestampInfo ( PTIMESTAMPINFO pInfo, bool use_tsc = true )
{
	if ( use_tsc ) {
		if ( ExistTSC () ) {
			pInfo->use_rdtsc = 1;
		}
	}

	QueryPerformanceFrequency ( &pInfo->CPUFreq );

	// calibrate rdtsc
    unsigned __int64 sts, ets;
	LARGE_INTEGER    spc, epc;
	if ( pInfo->use_rdtsc ) {
		QueryPerformanceCounter ( &spc );
		sts = __rdtsc();

		int i;
		while (1) {
			for (i=0; i<10000; i++);
			QueryPerformanceCounter ( &epc );	
			ets = __rdtsc();
			if ( epc.QuadPart - spc.QuadPart >= pInfo->CPUFreq.QuadPart ) break;
		}

		pInfo->TickPerSecond = (ets-sts) * pInfo->CPUFreq.QuadPart / 
			(epc.QuadPart - spc.QuadPart); 
	} else {
		pInfo->TickPerSecond = ULONGLONG(-1);
	}
}

// Get current timestamp by the method specified by TIMESTAMPINFO object
// Parameters
//     pTSInfo  - pointer to the TIMESTAMPINFO object
// Return value
//     current timestamp
FINL
ULONGLONG SoraGetCPUTimestamp (PTIMESTAMPINFO pTSInfo) { 
	if (pTSInfo->use_rdtsc) 
		return __rdtsc(); 
	else {
		LARGE_INTEGER perfcnt;
		QueryPerformanceCounter ( &perfcnt );
		return perfcnt.QuadPart;
	}
};

// Get the number of us has passed since last reboot
// Parameters
//     pTSInfo  - pointer to the TIMESTAMPINFO object
// Return value
//     the number of us has passed since last reboot
FINL
ULONGLONG SoraGetTimeofDay (PTIMESTAMPINFO pTSInfo) { 
	ULONGLONG ticks, us = 0;
	if (pTSInfo->use_rdtsc) {
		ticks = __rdtsc();
		us = ticks * TIME_ONE_MILLION / pTSInfo->TickPerSecond;
	}	
	else {
		LARGE_INTEGER perfcnt;
		QueryPerformanceCounter ( &perfcnt );
		us = perfcnt.QuadPart * TIME_ONE_MILLION / pTSInfo->CPUFreq.QuadPart;
	}

	return us;
}

// Parse the microseond value into hh:mm:ss.us format stored in SORATIMEOFDAYINFO object
// Parameters
//     info     - pointer to the SORATIMEOFDAYINFO object
//     us       - the microseond value
// Return value
//     true
FINL
bool SoraParseTime ( PSORATIMEOFDAYINFO info, ULONGLONG us )
{
	info->ss = (ULONG) ( us / TIME_ONE_MILLION );
	us =  (us % TIME_ONE_MILLION);
	
	info->hh  = info->ss / 3600; 
	info->ss -= info->hh * 3600;
	
	info->mm = info->ss / 60;
	info->ss -= info->mm * 60;
	info->ms = (ULONG) (us / 1000 );
	info->us = (ULONG) (us % 1000 );

	return true;
}

// Convert ticks value to nanoseconds. Ticks value is difference between two timestamp returned 
// by SoraGetCPUTimestamp functions
// Parameters
//     info     - pointer to the SORATIMEOFDAYINFO object
//     ts       - time elapse measured by timestamp value
// Return value
//     nanoseconds value
FINL
ULONGLONG SoraTimeElapsed ( ULONGLONG ts, PTIMESTAMPINFO tsinfo )
{
	if ( tsinfo->use_rdtsc ) {
		return ( ts * TIME_ONE_GIGA / tsinfo->TickPerSecond );
	} else {
		return ( ts * TIME_ONE_GIGA / tsinfo->CPUFreq.QuadPart );
	}
}

// Spinlock specified nanoseconds
// Parameters
//     pTSInfo  - pointer to the TIMESTAMPINFO object
//     waitns   - the nanoseconds value
// Return value
//     none
FINL
void SoraStallWait ( PTIMESTAMPINFO pTSInfo, ULONGLONG waitns ) // nano-second
{
	ULONGLONG ticks;
	ULONGLONG sts = SoraGetCPUTimestamp (pTSInfo);
	if ( pTSInfo->use_rdtsc ) {
		ticks = waitns * pTSInfo->TickPerSecond / 1000000000;
	} else {
		ticks = waitns * pTSInfo->CPUFreq.QuadPart / 1000000000;
	}
	ULONGLONG ets;
	while (1) {
		// mini wait
		ets = SoraGetCPUTimestamp (pTSInfo);
		if ( ets - sts >= ticks ) break;
		_mm_pause();
	}	
}

class SoraStopwatch
{
    TIMESTAMPINFO info;
    ULONGLONG acc;
    ULONGLONG start;
public:
    SoraStopwatch(bool use_tsc = true)
    {
        InitializeTimestampInfo(&info, use_tsc);
        Reset();
    }

    FINL void Reset()
    {
        acc = 0;
    }

    FINL void Restart()
    {
        Reset();
        Start();
    }

    FINL void Start()
    {
        start = SoraGetCPUTimestamp(&info);
    }

    FINL void Stop()
    {
        acc += SoraGetCPUTimestamp(&info) - start;
    }

    FINL ULONGLONG GetElapsedNanoseconds()
    {
        return SoraTimeElapsed(acc, &info);
    }
};

#endif
