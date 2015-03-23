#pragma once

#include "_share_lock_if.h"

struct NamedSpinlockHeader {

	volatile LONG m_LockProcId;
};

class ShareLockKImp :
	public ShareLock {

	friend class ShareMemKImp;

	struct Mutex {

		KMUTEX m_Mutex;
	};

	struct Event {

		KEVENT m_Event;
	};

	struct Semaphore {

		KSEMAPHORE m_Semaphore;
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
	ShareLockKImp*
	AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL bInitialOwner,
		LPCWSTR lpName);

	__forceinline
	static 
	ShareLockKImp*
	AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL bManualReset,
		BOOL bInitialState,
		LPCWSTR lpName);

	__forceinline
	static
	ShareLockKImp*
	AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		LONG lInitialCount,
		LONG lMaximumCount,
		LPCWSTR lpName);

	__forceinline
	static 
	ShareLockKImp*
	AllocateShareSpinlock();

	__forceinline
	static 
	ShareLockKImp*
	AllocateShareNamedSpinlock(LPCWSTR lpName);

	__forceinline
	static 
	void
	FreeShareLock(ShareLockKImp* sl);

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
	ShareLockKImp();

	__inline
	virtual 
	~ShareLockKImp();

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
	AttachShareNamedSpinlock(NamedSpinlockHeader* Header);
	
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
