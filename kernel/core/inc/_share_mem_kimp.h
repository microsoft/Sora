#pragma once

#include "_share_mem_if.h"
#include "_share_lock_kimp.h"

struct ShareMemHeader {

	WCHAR m_LockName[260];
	NamedSpinlockHeader m_Header;
	LONG m_bInitialized;
	LONG m_bResult;
};

#ifdef __cplusplus

class ShareMemKImp :
	public ShareMem {

public:
	__forceinline
	static 
	ShareMemKImp*
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
	FreeShareMem(ShareMemKImp* sm);

public:

#if 0
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
#endif

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

#if 0
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
#endif

protected:
	__inline
	ShareMemKImp();

	__inline
	virtual
	~ShareMemKImp();

	__inline
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
	void 
	UninitializeShareMem();

protected:
	ShareLockKImp m_LockObject;	
	HANDLE m_hMapSect;
	void* m_hMapView;
	DWORD m_hMapSize;
};

#endif
