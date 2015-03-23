#pragma once

#include <windef.h>

#ifndef USER_MODE

typedef PVOID					LPSECURITY_ATTRIBUTES;

#endif

#define PURE                    = 0

#ifdef __cplusplus

class ShareLock;

class ShareLock {

public:
	static
	ShareLock*
	AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL bInitialOwner,
		LPCWSTR lpName);

	static 
	ShareLock*
	AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL bManualReset,
		BOOL bInitialState,
		LPCWSTR lpName);

	static
	ShareLock*
	AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		LONG lInitialCount,
		LONG lMaximumCount,
		LPCWSTR lpName);

	static 
	ShareLock*
	AllocateShareSpinlock();

	static 
	ShareLock*
	AllocateShareNamedSpinlock(LPCWSTR lpName);

	static 
	void
	FreeShareLock(ShareLock* sl);

public:
	virtual
	DWORD 
	Lock(DWORD dwMilliseconds)
	PURE;

	virtual 
	BOOL 
	Unlock()
	PURE;

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
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

HANDLE
AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName);

HANDLE
AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName);

HANDLE
AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName);

HANDLE
AllocateShareSpinlock();

HANDLE
AllocateShareNamedSpinlock(LPCWSTR lpName);

void
FreeShareLock(HANDLE sl);

DWORD 
Lock(HANDLE sl,
	DWORD dwMilliseconds);

BOOL 
Unlock(HANDLE sl);

BOOL 
SetShareEvent(HANDLE sl);

BOOL
ResetShareEvent(HANDLE sl);

BOOL
ReleaseShareSemaphore(HANDLE sl,
	LONG lReleaseCount,
	LPLONG lpPreviousCount);

#ifdef __cplusplus
}
#endif
