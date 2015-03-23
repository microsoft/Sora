#pragma once

#include <Windows.h>
#include "_share_lock_if.h"
#include "_share_lock_uimp.h"
#include "_share_mem_if.h"

struct ShareMemHeader {

	WCHAR m_LockName[260];
	NamedSpinlockHeader m_Header;
	LONG m_bInitialized;
	LONG m_bResult;
};

class ShareMemUImp :
	public ShareMem {

public:
	__forceinline
	static 
	ShareMemUImp*
	AllocateShareMem(HANDLE hFile,
		LPSECURITY_ATTRIBUTES lpAttributes,
		DWORD flProtect,
		DWORD dwMaximumSize,
		LPCWSTR lpName, 
		DWORD dwDesiredAccess,
		ShareMemClassInit* lpInitialization,
		void* lpUserContext);

	__forceinline
	static 
	void
	FreeShareMem(ShareMemUImp* sm);

public:
	__inline
	virtual
	BOOL
	InitializeMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL bInitialOwner);

	__inline
	virtual
	BOOL 
	InitializeEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL bManualReset,
		BOOL bInitialState);

	__inline
	virtual
	BOOL 
	InitializeSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		LONG lInitialCount,
		LONG lMaximumCount);

	__inline
	virtual
	BOOL 
	InitializeSpinlock();

	__inline
	virtual 
	BOOL 
	InitializeNamedSpinlock();

	__inline
	virtual
	void*
	GetAddress();

	__inline
	virtual
	DWORD
	GetSize();

	__inline
	virtual
	DWORD 
	Lock(DWORD dwMilliseconds);

	__inline
	virtual
	BOOL 
	Unlock();

	__inline
	virtual
	BOOL 
	SetShareEvent();

	__inline
	virtual
	BOOL
	ResetShareEvent();

	__inline
	virtual
	BOOL
	ReleaseShareSemaphore(LONG lReleaseCount,
		LPLONG lpPreviousCount);

protected:
	__inline
	ShareMemUImp();

	__inline
	virtual
	~ShareMemUImp();

	__inline
	virtual 
	BOOL 
	InitializeShareMem(HANDLE hFile,
		LPSECURITY_ATTRIBUTES lpAttributes,
		DWORD flProtect,
		DWORD dwMaximumSize,
		LPCWSTR lpName, 
		DWORD dwDesiredAccess,
		ShareMemClassInit* lpInitialization,
		void* lpUserContext);

	__inline
	virtual
	void 
	UninitializeShareMem();

	__inline
	virtual 
	DWORD
	AlterNamedSecurityInfo(LPCWSTR lpName,
		DWORD dwDesiredAccess);

protected:
	ShareLockUImp m_LockObject;	
	HANDLE m_hMapFile;
	void* m_hMapView;
	DWORD m_hMapSize;
};