#pragma once

#include <windef.h>

#ifndef USER_MODE

typedef PVOID					LPSECURITY_ATTRIBUTES;

#endif

#define PURE                    = 0

#ifdef __cplusplus

class ShareMem;

typedef BOOL ShareMemClassInit(ShareMem* sm,
	void* context);

class ShareMem {

public:
	static 
	ShareMem*
	AllocateShareMem(HANDLE hFile,
		LPSECURITY_ATTRIBUTES lpAttributes,
		DWORD flProtect,
		DWORD dwMaximumSize,
		LPCWSTR lpName, 
		DWORD dwDesiredAccess,
		ShareMemClassInit* lpInitialization,
		void* lpUserContext);

	static 
	void
	FreeShareMem(ShareMem* sm);

public:

#if 0	
	virtual 
	BOOL
	InitializeMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL bInitialOwner) 
	PURE;

	virtual 
	BOOL 
	InitializeEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL bManualReset,
		BOOL bInitialState)
	PURE;

	virtual 
	BOOL 
	InitializeSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		LONG lInitialCount,
		LONG lMaximumCount)
	PURE;

	virtual 
	BOOL 
	InitializeSpinlock()
	PURE;
#endif

	virtual 
	BOOL 
	InitializeNamedSpinlock()
	PURE;

	virtual 
	void*
	GetAddress()
	PURE;

	virtual 
	DWORD
	GetSize()
	PURE;

	virtual 
	DWORD 
	Lock(DWORD dwMilliseconds)
	PURE;

	virtual 
	BOOL 
	Unlock()
	PURE;

#if 0
	virtual
	BOOL 
	SetShareEvent()
	PURE;

	virtual
	BOOL
	ResetShareEvent()
	PURE;

	virtual
	BOOL
	ReleaseShareSemaphore(LONG lReleaseCount,
		LPLONG lpPreviousCount)
	PURE;
#endif

};

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL ShareMemHandleInit(HANDLE sm,
	void* context);

HANDLE 
AllocateShareMem(HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpAttributes,
	DWORD flProtect,
	DWORD dwMaximumSize,
	LPCWSTR lpName, 
	DWORD dwDesiredAccess,
	ShareMemHandleInit* lpInitialization,
	void* lpUserContext);

void
FreeShareMem(HANDLE sm);

#if 0
BOOL
InitializeShareMemMutex(HANDLE sm,
	LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner);

BOOL 
InitializeShareMemEvent(HANDLE sm,
	LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState);

BOOL 
InitializeShareMemSemaphore(HANDLE sm,
	LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount);

BOOL 
InitializeShareMemSpinlock(HANDLE sm);
#endif

BOOL 
InitializeShareMemNamedSpinlock(HANDLE sm);

void*
GetShareMemAddress(HANDLE sm);

DWORD
GetShareMemSize(HANDLE sm);

DWORD 
LockShareMem(HANDLE sm,
	DWORD dwMilliseconds);

BOOL 
UnlockShareMem(HANDLE sm);

#if 0
BOOL 
SetShareMemEvent(HANDLE sm);

BOOL
ResetShareMemEvent(HANDLE sm);

BOOL
ReleaseShareMemSemaphore(HANDLE sm,
	LONG lReleaseCount,
	LPLONG lpPreviousCount);
#endif

#ifdef __cplusplus
}
#endif
