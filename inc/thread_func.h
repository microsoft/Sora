#pragma once
#include "sora.h"

inline void StopFreeThread(const HANDLE& hThread)
{
	if (hThread)
    {
		SoraThreadStop(hThread);
		SoraThreadFree(hThread);
	}
}

inline void StopFreeThread(HANDLE& hThread)
{
    StopFreeThread((const HANDLE&)hThread);
    hThread = NULL;
}

inline HANDLE AllocStartThread(PSORA_UTHREAD_PROC User_Routine, PVOID User_Context = NULL)
{
	HANDLE ret = SoraThreadAlloc();
	if (!ret) return ret;
		
	if (SoraThreadStart(ret, User_Routine, User_Context)) return ret;
	// Failed to start sora thread, clean up
    StopFreeThread(ret);
    return NULL;
}

inline HANDLE AllocStartThreadEx(PSORA_UTHREAD_PROC* User_Routine, PVOID* User_Context, ULONG Count)
{
	HANDLE ret = SoraThreadAlloc();
	if (!ret) return ret;
		
	if (SoraThreadStartEx(ret, User_Routine, User_Context, Count, ROUND_ROBIN)) return ret;
	// Failed to start sora thread, clean up
    StopFreeThread(ret);
    return NULL;
}
