#include "_share_lock_uimp.h"
#include <stdio.h>

#define MAX_ERR	100000

ShareLock*
ShareLock::AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName) {

	return ShareLockUImp::AllocateShareMutex(lpMutexAttributes,
		bInitialOwner,
		lpName);
}

ShareLock*
ShareLock::AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName) {

	return ShareLockUImp::AllocateShareEvent(lpEventAttributes,
		bManualReset,
		bInitialState,
		lpName);
}

ShareLock*
ShareLock::AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName) {

	return ShareLockUImp::AllocateShareSemaphore(lpSemaphoreAttributes,
		lInitialCount,
		lMaximumCount,
		lpName);
}

ShareLock*
ShareLock::AllocateShareSpinlock() {

	return ShareLockUImp::AllocateShareSpinlock();
}

ShareLock*
ShareLock::AllocateShareNamedSpinlock(LPCWSTR lpName) {

	return ShareLockUImp::AllocateShareNamedSpinlock(lpName);
}

void
ShareLock::FreeShareLock(ShareLock* sl) {

	ShareLockUImp::FreeShareLock((ShareLockUImp*)sl);
}

ShareLockUImp*
ShareLockUImp::AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName) {

	ShareLockUImp* sl;
	sl = new ShareLockUImp();
	if (sl->InitializeShareMutex(lpMutexAttributes,
		bInitialOwner,
		lpName))
		return sl;
	ShareLockUImp::FreeShareLock(sl);
	return NULL;
}

ShareLockUImp*
ShareLockUImp::AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName) {

	ShareLockUImp* sl;
	sl = new ShareLockUImp();
	if (sl->InitializeShareEvent(lpEventAttributes,
		bManualReset,
		bInitialState,
		lpName))
		return sl;
	ShareLockUImp::FreeShareLock(sl);
	return NULL;
}

ShareLockUImp*
ShareLockUImp::AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName) {

	ShareLockUImp* sl;
	sl = new ShareLockUImp();
	if (sl->InitializeShareSemaphore(lpSemaphoreAttributes,
		lInitialCount,
		lMaximumCount,
		lpName))
		return sl;
	ShareLockUImp::FreeShareLock(sl);
	return NULL;
}

ShareLockUImp*
ShareLockUImp::AllocateShareSpinlock() {

	ShareLockUImp* sl;
	sl = new ShareLockUImp();
	if (sl->InitializeShareSpinlock())
		return sl;
	ShareLockUImp::FreeShareLock(sl);
	return NULL;
}

ShareLockUImp*
ShareLockUImp::AllocateShareNamedSpinlock(LPCWSTR lpName) {

	ShareLockUImp* sl;
	sl = new ShareLockUImp();
	if (sl->InitializeShareNamedSpinlock(lpName))
		return sl;
	ShareLockUImp::FreeShareLock(sl);
	return NULL;
}

void
ShareLockUImp::FreeShareLock(ShareLockUImp* sl) {

	if (sl) {
		delete sl;
		sl = NULL;
	}
}

DWORD 
ShareLockUImp::Lock(DWORD dwMilliseconds) {

	switch(m_LockType) {
	case LockTypeMutex:
		if (m_LockObject.m_Mutex.m_hMutex)
			return WaitForSingleObject(m_LockObject.m_Mutex.m_hMutex, 
				dwMilliseconds);
		break;
	case LockTypeEvent:
		if (m_LockObject.m_Event.m_hEvent)
			return WaitForSingleObject(m_LockObject.m_Event.m_hEvent, 
				dwMilliseconds);
		break;
	case LockTypeSemaphore:
		if (m_LockObject.m_Semaphore.m_hSemaphore)
			return WaitForSingleObject(m_LockObject.m_Semaphore.m_hSemaphore, 
				dwMilliseconds);
		break;
	case LockTypeSpinlock:
		break;
	case LockTypeNamedSpinlock:
		if (m_LockObject.m_NamedSpinlock.m_lpHeader) {
			LONG err;
			err = 0;
			while(1) {
				LONG pid;
				pid = m_LockObject.m_NamedSpinlock.m_lpHeader->m_LockProcId;
				if (pid) {
					err++;
					if (err > MAX_ERR) {
						err = 0;
						HANDLE proc;
						proc = OpenProcess(SYNCHRONIZE,
							FALSE,
							pid);
						if (proc)
							CloseHandle(proc);
						else 
						if (InterlockedCompareExchange(&m_LockObject.m_NamedSpinlock.m_lpHeader->m_LockProcId, 
							GetCurrentProcessId(),
							pid) == pid)
							break;
					}
					_mm_pause();
				}
				else
				if (InterlockedCompareExchange(&m_LockObject.m_NamedSpinlock.m_lpHeader->m_LockProcId,
					GetCurrentProcessId(),
					pid) == pid)
					break;
			}
			return WAIT_OBJECT_0;
		}
		break;
	default:
		break;
	}
	return WAIT_FAILED;
}

BOOL 
ShareLockUImp::Unlock() {

	switch(m_LockType) {
	case LockTypeMutex:
		if (m_LockObject.m_Mutex.m_hMutex)
			return ReleaseMutex(m_LockObject.m_Mutex.m_hMutex);
		break;
	case LockTypeEvent:
		if (m_LockObject.m_Event.m_hEvent)
			return SetEvent(m_LockObject.m_Event.m_hEvent);
		break;
	case LockTypeSemaphore:
		if (m_LockObject.m_Semaphore.m_hSemaphore)
			return ReleaseSemaphore(m_LockObject.m_Semaphore.m_hSemaphore,
				1,
				NULL);
		break;
	case LockTypeSpinlock:
		break;
	case LockTypeNamedSpinlock:
		if (m_LockObject.m_NamedSpinlock.m_lpHeader) {
			InterlockedCompareExchange(&m_LockObject.m_NamedSpinlock.m_lpHeader->m_LockProcId,
				0,
				GetCurrentProcessId());
			return TRUE;
		}
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL 
ShareLockUImp::SetShareEvent() {

	switch(m_LockType) {
	case LockTypeEvent:
		if (m_LockObject.m_Event.m_hEvent)
			return SetEvent(m_LockObject.m_Event.m_hEvent);
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL
ShareLockUImp::ResetShareEvent() {

	switch(m_LockType) {
	case LockTypeEvent:
		if (m_LockObject.m_Event.m_hEvent)
			return ResetEvent(m_LockObject.m_Event.m_hEvent);
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL
ShareLockUImp::ReleaseShareSemaphore(LONG lReleaseCount,
	LPLONG lpPreviousCount) {

	switch(m_LockType) {
	case LockTypeSemaphore:
		if (m_LockObject.m_Semaphore.m_hSemaphore)
			return ReleaseSemaphore(m_LockObject.m_Semaphore.m_hSemaphore,
				lReleaseCount,
				lpPreviousCount);
		break;
	default:
		break;
	}
	return FALSE;
}

ShareLockUImp::ShareLockUImp() :
	m_LockType(LockTypeNone) {

	memset(&m_LockObject, 
		0, 
		sizeof(m_LockObject));
}

ShareLockUImp::~ShareLockUImp() {

	UninitializeShareLock();
}

BOOL 
ShareLockUImp::InitializeShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName) {

	UninitializeShareLock();

	m_LockType = LockTypeMutex;
	m_LockObject.m_Mutex.m_hMutex = CreateMutexW(lpMutexAttributes,
		bInitialOwner,
		lpName);
	if (m_LockObject.m_Mutex.m_hMutex)
		return TRUE;
	UninitializeShareLock();
	return FALSE;
}

BOOL 
ShareLockUImp::InitializeShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName) {

	UninitializeShareLock();

	m_LockType = LockTypeEvent;
	m_LockObject.m_Event.m_hEvent = CreateEventW(lpEventAttributes,
		bManualReset,
		bInitialState,
		lpName);
	if (m_LockObject.m_Event.m_hEvent)
		return TRUE;
	UninitializeShareLock();
	return FALSE;
}

BOOL
ShareLockUImp::InitializeShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName) {

	UninitializeShareLock();

	m_LockType = LockTypeSemaphore;
	m_LockObject.m_Semaphore.m_hSemaphore = CreateSemaphoreW(lpSemaphoreAttributes,
		lInitialCount,
		lMaximumCount,
		lpName);
	if (m_LockObject.m_Semaphore.m_hSemaphore)
		return TRUE;
	UninitializeShareLock();
	return FALSE;
}

BOOL
ShareLockUImp::InitializeShareSpinlock() {

	UninitializeShareLock();
	return FALSE;
}


BOOL
ShareLockUImp::InitializeShareNamedSpinlock(LPCWSTR lpName) {

	UninitializeShareLock();

	m_LockType = LockTypeNamedSpinlock;
	m_LockObject.m_NamedSpinlock.m_hMapFile = CreateFileMappingW(NULL, 
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(NamedSpinlockHeader),
		lpName);
	if (m_LockObject.m_NamedSpinlock.m_hMapFile) {
		m_LockObject.m_NamedSpinlock.m_hMapView = MapViewOfFile(m_LockObject.m_NamedSpinlock.m_hMapFile,
			FILE_MAP_READ|FILE_MAP_WRITE,
			0,
			0,
			0);
		if (m_LockObject.m_NamedSpinlock.m_hMapView) {
			m_LockObject.m_NamedSpinlock.m_lpHeader = (NamedSpinlockHeader*)m_LockObject.m_NamedSpinlock.m_hMapView;
			return TRUE;
		}
	}
	UninitializeShareLock();
	return FALSE;
}

void 
ShareLockUImp::UninitializeShareLock() {

	switch(m_LockType) {
	case LockTypeMutex:
		if (m_LockObject.m_Mutex.m_hMutex) {
			CloseHandle(m_LockObject.m_Mutex.m_hMutex);
			m_LockObject.m_Mutex.m_hMutex = NULL;
		}
		break;
	case LockTypeEvent:
		if (m_LockObject.m_Event.m_hEvent) {
			CloseHandle(m_LockObject.m_Event.m_hEvent);
			m_LockObject.m_Event.m_hEvent = NULL;
		}
		break;
	case LockTypeSemaphore:
		if (m_LockObject.m_Semaphore.m_hSemaphore) {
			CloseHandle(m_LockObject.m_Semaphore.m_hSemaphore);
			m_LockObject.m_Semaphore.m_hSemaphore = NULL;
		}
		break;
	case LockTypeSpinlock:		
		break;
	case LockTypeNamedSpinlock:
		if (m_LockObject.m_NamedSpinlock.m_hMapView) {
			UnmapViewOfFile(m_LockObject.m_NamedSpinlock.m_hMapView);
			m_LockObject.m_NamedSpinlock.m_hMapView = NULL;
		}
		if (m_LockObject.m_NamedSpinlock.m_hMapFile) {
			CloseHandle(m_LockObject.m_NamedSpinlock.m_hMapFile);
			m_LockObject.m_NamedSpinlock.m_hMapFile = NULL;
		}
		m_LockObject.m_NamedSpinlock.m_lpHeader = NULL;
		break;
	default:
		break;
	}	
	m_LockType = LockTypeNone;
}

BOOL 
ShareLockUImp::AttachShareNamedSpinlock(NamedSpinlockHeader* lpHeader) {

	UninitializeShareLock();

	m_LockType = LockTypeNamedSpinlock;
	m_LockObject.m_NamedSpinlock.m_lpHeader = lpHeader;
	return TRUE;
}

HANDLE
AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName) {

	return (HANDLE)ShareLock::AllocateShareMutex(lpMutexAttributes,
		bInitialOwner,
		lpName);
}

HANDLE
AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName) {

	return (HANDLE)ShareLock::AllocateShareEvent(lpEventAttributes,
		bManualReset,
		bInitialState,
		lpName);
}

HANDLE
AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName) {

	return (HANDLE)ShareLock::AllocateShareSemaphore(lpSemaphoreAttributes,
		lInitialCount,
		lMaximumCount,
		lpName);
}

HANDLE
AllocateShareSpinlock() {

	return (HANDLE)ShareLock::AllocateShareSpinlock();
}

HANDLE
AllocateShareNamedSpinlock(LPCWSTR lpName) {

	return (HANDLE)ShareLock::AllocateShareNamedSpinlock(lpName);
}

void
FreeShareLock(HANDLE sl) {

	ShareLock::FreeShareLock((ShareLock*)sl);
}

DWORD 
Lock(HANDLE sl,
	DWORD dwMilliseconds) {

	return ((ShareLock*)sl)->Lock(dwMilliseconds);
}

BOOL 
Unlock(HANDLE sl) {

	return ((ShareLock*)sl)->Unlock();
}

BOOL 
SetShareEvent(HANDLE sl) {

	return ((ShareLock*)sl)->SetShareEvent();
}

BOOL
ResetShareEvent(HANDLE sl) {

	return ((ShareLock*)sl)->ResetShareEvent();
}

BOOL
ReleaseShareSemaphore(HANDLE sl,
	LONG lReleaseCount,
	LPLONG lpPreviousCount) {

	return ((ShareLock*)sl)->ReleaseShareSemaphore(lReleaseCount,
		lpPreviousCount);
}
