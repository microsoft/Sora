#include <ntddk.h>
#include <ntstrsafe.h>
#include "_share_mem_kimp.h"
#include "_new.h"

#pragma warning(disable:4100)
#pragma warning(disable:4127)

ShareMem*
ShareMem::AllocateShareMem(HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpAttributes,
	DWORD flProtect,
	DWORD dwMaximumSize,
	LPCWSTR lpName, 
	DWORD dwDesiredAccess,
	ShareMemClassInit* lpInitialization,
	void* lpUserContext) {
	
	return ShareMemKImp::AllocateShareMem(hFile,
		lpAttributes,
		flProtect,
		dwMaximumSize,
		lpName,
		dwDesiredAccess,
		lpInitialization, 
		lpUserContext);
}

void
ShareMem::FreeShareMem(ShareMem* sm) {

	ShareMemKImp::FreeShareMem((ShareMemKImp*)sm);
}

ShareMemKImp*
ShareMemKImp::AllocateShareMem(HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpAttributes,
	DWORD flProtect,
	DWORD dwMaximumSize,
	LPCWSTR lpName, 
	DWORD dwDesiredAccess,
	ShareMemClassInit* lpInitialization, 
	void* lpUserContext) {

	ShareMemKImp* sm;
	sm = new (NonPagedPool) ShareMemKImp;
	if (sm->InitializeShareMem(hFile,
		lpAttributes,
		flProtect,
		dwMaximumSize,
		lpName,
		dwDesiredAccess,
		lpInitialization,
		lpUserContext))
		return sm;
	ShareMemKImp::FreeShareMem(sm);
	return NULL;
}

void
ShareMemKImp::FreeShareMem(ShareMemKImp* sm) {

	if (sm) {
		delete sm;
		sm = NULL;
	}
}

#if 0
BOOL
ShareMemKImp::InitializeMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner) {

	return FALSE;
}

BOOL 
ShareMemKImp::InitializeEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState) {

	return FALSE;
}

BOOL 
ShareMemKImp::InitializeSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount) {

	return FALSE;
}

BOOL 
ShareMemKImp::InitializeSpinlock() {

	return FALSE;
}
#endif

BOOL 
ShareMemKImp::InitializeNamedSpinlock() {

	if (m_hMapView)
		return m_LockObject.AttachShareNamedSpinlock(&((ShareMemHeader*)m_hMapView)->m_Header);
	
	return FALSE;
}

void*
ShareMemKImp::GetAddress() {

	if (m_hMapView)
		return (PBYTE)m_hMapView + sizeof(ShareMemHeader);

	return NULL;
}

DWORD
ShareMemKImp::GetSize() {

	return m_hMapSize;
}

DWORD
ShareMemKImp::Lock(DWORD dwMilliseconds) {

	return m_LockObject.Lock(dwMilliseconds);
}

BOOL 
ShareMemKImp::Unlock() {

	return m_LockObject.Unlock();
}

#if 0
BOOL 
ShareMemKImp::SetShareEvent() {

	return FALSE;
}

BOOL 
ShareMemKImp::ResetShareEvent() {

	return FALSE;
}

BOOL 
ShareMemKImp::ReleaseShareSemaphore(LONG lReleaseCount,
	LPLONG lpPreviousCount) {

	return FALSE;
}
#endif

ShareMemKImp::ShareMemKImp() :
	m_hMapSect(NULL), 
	m_hMapView(NULL),
	m_hMapSize(0) {

}

ShareMemKImp::~ShareMemKImp() {

	UninitializeShareMem();
}

BOOL 
ShareMemKImp::InitializeShareMem(HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpAttributes,
	DWORD flProtect,
	DWORD dwMaximumSize,
	LPCWSTR lpName, 
	DWORD dwDesiredAccess,
	ShareMemClassInit* lpInitialization,
	void* lpUserContext) {

	UninitializeShareMem();

	DWORD size;
	size = dwMaximumSize + sizeof(ShareMemHeader);
	if (size < dwMaximumSize)
		return FALSE;

	UNICODE_STRING us;
	RtlInitUnicodeString(&us,
		lpName);

	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes(&attr,
		&us,
		OBJ_OPENIF,
		NULL,
		NULL);

	LARGE_INTEGER li;
	li.QuadPart = size;

	NTSTATUS status;
	status = ZwCreateSection(&m_hMapSect,
		READ_CONTROL|WRITE_DAC|dwDesiredAccess,
		&attr,
		&li,
		flProtect,
		SEC_COMMIT,
		hFile);

	BOOLEAN init;
	init = status != STATUS_SUCCESS ? FALSE : TRUE;
	
	if (m_hMapSect) {
		SIZE_T v_size;
		v_size = 0;
		status = ZwMapViewOfSection(m_hMapSect,
			ZwCurrentProcess(),
			&m_hMapView,
			0,
			size,
			NULL,
			&v_size,			
			ViewUnmap,
			0,
			PAGE_READWRITE);
		if (m_hMapView) {
			m_hMapSize = dwMaximumSize;

			ShareMemHeader* smh;
			smh = (ShareMemHeader*)m_hMapView;			
			if (init) {
				LARGE_INTEGER tick;
				KeQueryTickCount(&tick);

				RtlStringCchPrintfW(smh->m_LockName, 
					sizeof(smh->m_LockName), 
					L"K-%08x-%08x-%08x",
					PsGetCurrentProcessId(),
					PsGetCurrentThreadId(),
					tick.LowPart);
								
				if (lpInitialization)
					smh->m_bResult = lpInitialization(this,
						lpUserContext);
				else
					smh->m_bResult = TRUE;

				smh->m_bInitialized = TRUE;
			}
			else
				while(1)
					if (smh->m_bInitialized)
						break;

			return smh->m_bResult;
		}
	}
	return FALSE;
}

void 
ShareMemKImp::UninitializeShareMem() {

	m_LockObject.UninitializeShareLock();

	if (m_hMapView) {
		ZwUnmapViewOfSection(PsGetCurrentProcessId(),
			m_hMapView);
		m_hMapView = NULL;
	}
	if (m_hMapSect) {
		ZwClose(m_hMapSect);
		m_hMapSect = NULL;
	}
	m_hMapSize = 0;
}

HANDLE 
AllocateShareMem(HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpAttributes,
	DWORD flProtect,
	DWORD dwMaximumSize,
	LPCWSTR lpName, 
	DWORD dwDesiredAccess,
	ShareMemHandleInit* lpInitialization,
	void* lpUserContext) {

	return (HANDLE)ShareMem::AllocateShareMem(hFile,
		lpAttributes,
		flProtect,
		dwMaximumSize,
		lpName,
		dwDesiredAccess,
		(ShareMemClassInit*)lpInitialization,
		lpUserContext);
}

void
FreeShareMem(HANDLE sm) {

	ShareMem::FreeShareMem((ShareMem*)sm);
}

#if 0
BOOL
InitializeShareMemMutex(HANDLE sm,
	LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner) {

	return ((ShareMem*)sm)->InitialMutex(lpMutexAttributes,
		bInitialOwner);
}

BOOL 
InitializeShareMemEvent(HANDLE sm,
	LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState) {

	return ((ShareMem*)sm)->InitialEvent(lpEventAttributes,
		bManualReset,
		bInitialState);
}

BOOL 
InitializeShareMemSemaphore(HANDLE sm,
	LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount) {

	return ((ShareMem*)sm)->InitialSemaphore(lpSemaphoreAttributes,
		lInitialCount,
		lMaximumCount);
}

BOOL 
InitializeShareMemSpinlock(HANDLE sm) {

	return ((ShareMem*)sm)->InitializeSpinlock();
}
#endif

BOOL 
InitializeShareMemNamedSpinlock(HANDLE sm) {

	return ((ShareMem*)sm)->InitializeNamedSpinlock();
}

void*
GetShareMemAddress(HANDLE sm) {

	return ((ShareMem*)sm)->GetAddress();
}

DWORD
GetShareMemSize(HANDLE sm) {

	return ((ShareMem*)sm)->GetSize();
}

DWORD 
LockShareMem(HANDLE sm,
	DWORD dwMilliseconds) {

	return ((ShareMem*)sm)->Lock(dwMilliseconds);
}

BOOL 
UnlockShareMem(HANDLE sm) {

	return ((ShareMem*)sm)->Unlock();
}

#if 0
BOOL 
SetShareMemEvent(HANDLE sm) {

	return ((ShareMem*)sm)->SetShareEvent();
}

BOOL
ResetShareMemEvent(HANDLE sm) {

	return ((ShareMem*)sm)->ResetShareEvent();
}

BOOL
ReleaseShareMemSemaphore(HANDLE sm,
	LONG lReleaseCount,
	LPLONG lpPreviousCount) {

	return ((ShareMem*)sm)->ReleaseShareSemaphore(lReleaseCount,
		lpPreviousCount);
}
#endif

