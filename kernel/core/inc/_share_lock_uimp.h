#pragma once

#include <windows.h>
#include <tchar.h>
#include "_share_lock_if.h"

struct NamedSpinlockHeader {

	volatile LONG m_LockProcId;
};

class ShareLockUImp :
	public ShareLock {

	friend class ShareMemUImp;

	struct Mutex {

		HANDLE m_hMutex;
	};

	struct Event {

		HANDLE m_hEvent;
	};

	struct Semaphore {

		HANDLE m_hSemaphore;
	};

	struct Spinlock {

	};

	struct NamedSpinlock {

		HANDLE m_hMapFile;
		void* m_hMapView;
		NamedSpinlockHeader* m_lpHeader;
	};

	enum LockType {

		LockTypeNone = 0,
		LockTypeMutex,
		LockTypeEvent,
		LockTypeSemaphore,
		LockTypeSpinlock,
		LockTypeNamedSpinlock
	};

public:
	__forceinline
	static
	ShareLockUImp*
	AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL bInitialOwner,
		LPCWSTR lpName);

	__forceinline
	static 
	ShareLockUImp*
	AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL bManualReset,
		BOOL bInitialState,
		LPCWSTR lpName);

	__forceinline
	static
	ShareLockUImp*
	AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		LONG lInitialCount,
		LONG lMaximumCount,
		LPCWSTR lpName);

	__forceinline
	static 
	ShareLockUImp*
	AllocateShareSpinlock();

	__forceinline
	static 
	ShareLockUImp*
	AllocateShareNamedSpinlock(LPCWSTR lpName);

	__forceinline
	static 
	void
	FreeShareLock(ShareLockUImp* sl);

public:
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
	ShareLockUImp();

	__inline
	virtual 
	~ShareLockUImp();

	__inline
	virtual
	BOOL 
	InitializeShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL bInitialOwner,
		LPCWSTR lpName);

	__inline
	virtual
	BOOL 
	InitializeShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL bManualReset,
		BOOL bInitialState,
		LPCWSTR lpName);

	__inline
	virtual
	BOOL
	InitializeShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		LONG lInitialCount,
		LONG lMaximumCount,
		LPCWSTR lpName);

	__inline
	virtual
	BOOL
	InitializeShareSpinlock();

	__inline
	virtual
	BOOL
	InitializeShareNamedSpinlock(LPCWSTR lpName);

	__inline
	virtual
	void
	UninitializeShareLock();
	
	__inline
	virtual 
	BOOL 	
	AttachShareNamedSpinlock(NamedSpinlockHeader* lpHeader);

protected:
	LockType m_LockType;
	union {
		Mutex m_Mutex;
		Event m_Event;
		Semaphore m_Semaphore;
		Spinlock m_Spinlock;
		NamedSpinlock m_NamedSpinlock;
	} m_LockObject;	
};
