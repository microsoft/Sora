#include <ntddk.h>
#include <wdm.h>
#include "_share_lock_kimp.h"
#include "_new.h"

#pragma warning(disable:4100)

#define MAX_ERR	100000

ShareLock*
ShareLock::AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName) {

	return ShareLockKImp::AllocateShareMutex(lpMutexAttributes,
		bInitialOwner,
		lpName);
}

ShareLock*
ShareLock::AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName) {

	return ShareLockKImp::AllocateShareEvent(lpEventAttributes,
		bManualReset,
		bInitialState,
		lpName);
}

ShareLock*
ShareLock::AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName) {

	return ShareLockKImp::AllocateShareSemaphore(lpSemaphoreAttributes,
		lInitialCount,
		lMaximumCount,
		lpName);
}

ShareLock*
ShareLock::AllocateShareSpinlock() {

	return ShareLockKImp::AllocateShareSpinlock();
}

ShareLock*
ShareLock::AllocateShareNamedSpinlock(LPCWSTR lpName) {

	return ShareLockKImp::AllocateShareNamedSpinlock(lpName);
}

void
ShareLock::FreeShareLock(ShareLock* sl) {

	ShareLockKImp::FreeShareLock((ShareLockKImp*)sl);
}

ShareLockKImp*
ShareLockKImp::AllocateShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName) {

	ShareLockKImp* sl;
	sl = new (NonPagedPool) ShareLockKImp;
	if (sl->InitializeShareMutex(lpMutexAttributes,
		bInitialOwner,
		lpName))
		return sl;
	ShareLockKImp::FreeShareLock(sl);
	return NULL;
}

ShareLockKImp*
ShareLockKImp::AllocateShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName) {

	ShareLockKImp* sl;
	sl = new (NonPagedPool) ShareLockKImp;
	if (sl->InitializeShareEvent(lpEventAttributes,
		bManualReset,
		bInitialState,
		lpName))
		return sl;
	ShareLockKImp::FreeShareLock(sl);
	return NULL;
}

ShareLockKImp*
ShareLockKImp::AllocateShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName) {

	ShareLockKImp* sl;
	sl = new (NonPagedPool) ShareLockKImp;
	if (sl->InitializeShareSemaphore(lpSemaphoreAttributes,
		lInitialCount,
		lMaximumCount,
		lpName))
		return sl;
	ShareLockKImp::FreeShareLock(sl);
	return NULL;
}

ShareLockKImp*
ShareLockKImp::AllocateShareSpinlock() {

	ShareLockKImp* sl;
	sl = new (NonPagedPool) ShareLockKImp;
	if (sl->InitializeShareSpinlock())
		return sl;
	ShareLockKImp::FreeShareLock(sl);
	return NULL;
}

ShareLockKImp*
ShareLockKImp::AllocateShareNamedSpinlock(LPCWSTR lpName) {

	ShareLockKImp* sl;
	sl = new (NonPagedPool) ShareLockKImp;
	if (sl->InitializeShareNamedSpinlock(lpName))
		return sl;
	ShareLockKImp::FreeShareLock(sl);
	return NULL;
}
	
void
ShareLockKImp::FreeShareLock(ShareLockKImp* sl) {

	if (sl) {
		delete sl;
		sl = NULL;
	}
}

DWORD 
ShareLockKImp::Lock(DWORD dwMilliseconds) {

	switch(m_LockType) {
	case LockTypeMutex: {
			LARGE_INTEGER li;
			li.QuadPart = dwMilliseconds;
			return KeWaitForSingleObject(&m_LockObject.m_Mutex.m_Mutex,
				Executive,
				KernelMode,
				FALSE,
				&li);
		}
		break;
	case LockTypeEvent: {
			LARGE_INTEGER li;
			li.QuadPart = dwMilliseconds;
			return KeWaitForSingleObject(&m_LockObject.m_Event.m_Event,
				Executive,
				KernelMode,
				FALSE,
				&li);
		}
		break;
	case LockTypeSemaphore: {
			LARGE_INTEGER li;
			li.QuadPart = dwMilliseconds;
			return KeWaitForSingleObject(&m_LockObject.m_Semaphore.m_Semaphore,
				Executive,
				KernelMode,
				FALSE,
				&li);
		}
		break;
	case LockTypeSpinlock:
		break;
	case LockTypeNamedSpinlock: {
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
							NTSTATUS status;
							HANDLE proc;
							OBJECT_ATTRIBUTES objattr;
							InitializeObjectAttributes(&objattr,
								NULL, 
								0,
								NULL,
								NULL);
							CLIENT_ID cid;
							cid.UniqueProcess = (HANDLE)pid;
							cid.UniqueThread = NULL;					
							status = ZwOpenProcess(&proc,
								SYNCHRONIZE,
								&objattr, 
								&cid);
							if (status != STATUS_SUCCESS) {
								if (InterlockedCompareExchange(&m_LockObject.m_NamedSpinlock.m_lpHeader->m_LockProcId, 
									(LONG)PsGetCurrentProcessId(),
									pid) == pid)
									break;
							}
							else
								ZwClose(proc);
						}
						_mm_pause();
					}
					else 
					if (InterlockedCompareExchange(&m_LockObject.m_NamedSpinlock.m_lpHeader->m_LockProcId,
						(LONG)PsGetCurrentProcessId(),
						pid) == pid)
						break;
				}
				return STATUS_SUCCESS;
			}
		}		
		break;
	default:
		break;
	}
	return (DWORD)STATUS_UNSUCCESSFUL;
}

BOOL 
ShareLockKImp::Unlock() {

	switch(m_LockType) {
	case LockTypeMutex:
		return KeReleaseMutex(&m_LockObject.m_Mutex.m_Mutex,
			FALSE);
		break;
	case LockTypeEvent:
		return KeSetEvent(&m_LockObject.m_Event.m_Event,
			IO_NO_INCREMENT,
			FALSE);
		break;
	case LockTypeSemaphore:
		return KeReleaseSemaphore(&m_LockObject.m_Semaphore.m_Semaphore,
			IO_NO_INCREMENT,
			1,
			FALSE);
		break;
	case LockTypeSpinlock:
		break;
	case LockTypeNamedSpinlock: {
			if (m_LockObject.m_NamedSpinlock.m_lpHeader) {
				InterlockedCompareExchange(&m_LockObject.m_NamedSpinlock.m_lpHeader->m_LockProcId,
					0,
					(LONG)PsGetCurrentProcessId());
				return STATUS_SUCCESS;
			}		
		}
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL 
ShareLockKImp::SetShareEvent() {

	switch(m_LockType) {
	case LockTypeEvent:
		return KeSetEvent(&m_LockObject.m_Event.m_Event,
			IO_NO_INCREMENT,
			FALSE);
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL
ShareLockKImp::ResetShareEvent() {

	switch(m_LockType) {
	case LockTypeEvent:
		return KeResetEvent(&m_LockObject.m_Event.m_Event);
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL
ShareLockKImp::ReleaseShareSemaphore(LONG lReleaseCount,
	LPLONG lpPreviousCount) {

	switch(m_LockType) {
	case LockTypeSemaphore:
		return KeReleaseSemaphore(&m_LockObject.m_Semaphore.m_Semaphore,
			IO_NO_INCREMENT,
			lReleaseCount,
			FALSE);
		break;
	default:
		break;
	}
	return FALSE;
}

ShareLockKImp::ShareLockKImp() :
	m_LockType(LockTypeNone) {

	RtlZeroMemory(&m_LockObject, 
		sizeof(m_LockObject));
}

ShareLockKImp::~ShareLockKImp() {

	UninitializeShareLock();
}

BOOL 
ShareLockKImp::InitializeShareMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCWSTR lpName) {

	UninitializeShareLock();

	m_LockType = LockTypeMutex;
	KeInitializeMutex(&m_LockObject.m_Mutex.m_Mutex,
		0);
	return TRUE;
}

BOOL 
ShareLockKImp::InitializeShareEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCWSTR lpName) {

	UninitializeShareLock();

	m_LockType = LockTypeEvent;
	KeInitializeEvent(&m_LockObject.m_Event.m_Event,
		bManualReset ? NotificationEvent : SynchronizationEvent,
		(BOOLEAN)bInitialState);
	return TRUE;
}

BOOL
ShareLockKImp::InitializeShareSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCWSTR lpName) {

	UninitializeShareLock();

	m_LockType = LockTypeSemaphore;
	KeInitializeSemaphore(&m_LockObject.m_Semaphore.m_Semaphore,
		lInitialCount,
		lMaximumCount);
	return TRUE;
}

BOOL
ShareLockKImp::InitializeShareSpinlock() {

	UninitializeShareLock();
	return FALSE;
}

BOOL
ShareLockKImp::InitializeShareNamedSpinlock(LPCWSTR lpName) {

	return FALSE;
}

void 
ShareLockKImp::UninitializeShareLock() {	

	switch(m_LockType) {
	case LockTypeMutex:
		RtlZeroMemory(&m_LockObject.m_Mutex.m_Mutex,
			sizeof(KMUTEX));
		break;
	case LockTypeEvent:
		RtlZeroMemory(&m_LockObject.m_Event.m_Event, 
			sizeof(KEVENT));
		break;
	case LockTypeSemaphore:
		RtlZeroMemory(&m_LockObject.m_Semaphore.m_Semaphore, 
			sizeof(KSEMAPHORE));
		break;
	case LockTypeSpinlock:
		break;
	case LockTypeNamedSpinlock: {
			if (m_LockObject.m_NamedSpinlock.m_hMapView) {
				ZwUnmapViewOfSection(ZwCurrentProcess(),
					m_LockObject.m_NamedSpinlock.m_hMapView);
				m_LockObject.m_NamedSpinlock.m_hMapView = NULL;
			}
			if (m_LockObject.m_NamedSpinlock.m_hMapFile) {
				ZwClose(m_LockObject.m_NamedSpinlock.m_hMapFile);
				m_LockObject.m_NamedSpinlock.m_hMapFile = NULL;
			}
			m_LockObject.m_NamedSpinlock.m_lpHeader = NULL;
		}
		break;
	default:
		break;
	}
	m_LockType = LockTypeNone;
	
}

BOOL 
ShareLockKImp::AttachShareNamedSpinlock(NamedSpinlockHeader* lpHeader) {

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

	ShareLockKImp::FreeShareLock((ShareLockKImp*)sl);
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

