#pragma once

#include "const.h"

typedef BOOLEAN (__stdcall *PSORA_UTHREAD_PROC)(PVOID Context);

typedef enum {

	ROUND_ROBIN,
} SCHEDULING_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

HANDLE SoraThreadAlloc();
VOID SoraThreadFree(HANDLE Thread);
BOOLEAN SoraThreadStart(HANDLE Thread, PSORA_UTHREAD_PROC User_Routine, PVOID User_Context);
BOOLEAN SoraThreadStartEx(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, PVOID* User_Context, ULONG Count, SCHEDULING_TYPE Type);
VOID SoraThreadStop(HANDLE Thread);
BOOLEAN SoraThreadJoin(HANDLE Thread, ULONG Timeout);
void SoraThreadYield(BOOLEAN rc);
BOOLEAN SoraThreadAddUserRoutine(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, PVOID* User_Context, ULONG Count);
BOOLEAN SoraThreadDelUserRoutine(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, ULONG Count);

#ifdef USER_MODE
void SoraThreadCompareWaitGt(long* var, long val);	// wait for var > val
void SoraThreadCompareWaitLt(long* var, long val);	// wait for var < val
void SoraThreadCompareWait(long* var, long val);	// wait for var = val
#endif

#ifdef __cplusplus
}
#endif

#if !defined(USER_MODE)
// Note: yielding sora thread is not supported in kernel mode
// force inline to prevent function call overhead
FINL void SoraThreadYield(BOOLEAN rc) { return; }
#endif
