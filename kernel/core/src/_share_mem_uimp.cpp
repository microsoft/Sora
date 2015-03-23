#include "_share_mem_uimp.h"
#include <stdio.h>
#include <aclapi.h>

ShareMem*
ShareMem::AllocateShareMem(HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpAttributes,
	DWORD flProtect,
	DWORD dwMaximumSize,
	LPCWSTR lpName, 
	DWORD dwDesiredAccess,
	ShareMemClassInit* lpInitialization, 
	void* lpUserContext) {
	
	return ShareMemUImp::AllocateShareMem(hFile,
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

	ShareMemUImp::FreeShareMem((ShareMemUImp*)sm);
}

ShareMemUImp*
ShareMemUImp::AllocateShareMem(HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpAttributes,
	DWORD flProtect,
	DWORD dwMaximumSize,
	LPCWSTR lpName, 
	DWORD dwDesiredAccess,
	ShareMemClassInit* lpInitialization,
	void* lpUserContext) {

	ShareMemUImp* sm;
	sm = new ShareMemUImp();
	if (sm->InitializeShareMem(hFile,
		lpAttributes,
		flProtect,
		dwMaximumSize,
		lpName,
		dwDesiredAccess,
		lpInitialization,
		lpUserContext))
		return sm;
	ShareMemUImp::FreeShareMem(sm);
	return NULL;
}

void
ShareMemUImp::FreeShareMem(ShareMemUImp* sm) {

	if (sm) {
		delete sm;
		sm = NULL;
	}
}

BOOL
ShareMemUImp::InitializeMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner) {

	m_LockObject.UninitializeShareLock();

	if (m_hMapView)
		return m_LockObject.InitializeShareMutex(lpMutexAttributes,
			bInitialOwner,
			((ShareMemHeader*)m_hMapView)->m_LockName);

	return FALSE;
}

BOOL 
ShareMemUImp::InitializeEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState) {

	m_LockObject.UninitializeShareLock();

	if (m_hMapView)
		return m_LockObject.InitializeShareEvent(lpEventAttributes,
			bManualReset,
			bInitialState,
			((ShareMemHeader*)m_hMapView)->m_LockName);

	return FALSE;
}

BOOL 
ShareMemUImp::InitializeSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount) {

	m_LockObject.UninitializeShareLock();

	if (m_hMapView)
		return m_LockObject.InitializeShareSemaphore(lpSemaphoreAttributes,
			lInitialCount,
			lMaximumCount,
			((ShareMemHeader*)m_hMapView)->m_LockName);

	return FALSE;
}

BOOL 
ShareMemUImp::InitializeSpinlock() {

	m_LockObject.UninitializeShareLock();

	if (m_hMapView)
		return m_LockObject.InitializeShareSpinlock();

	return FALSE;
}

BOOL 
ShareMemUImp::InitializeNamedSpinlock() {

	m_LockObject.UninitializeShareLock();

	if (m_hMapView)
		return m_LockObject.AttachShareNamedSpinlock(&((ShareMemHeader*)m_hMapView)->m_Header);

	return FALSE;
}

void*
ShareMemUImp::GetAddress() {

	if (m_hMapView)
		return (PBYTE)m_hMapView + sizeof(ShareMemHeader);

	return NULL;
}

DWORD
ShareMemUImp::GetSize() {

	return m_hMapSize;
}

DWORD 
ShareMemUImp::Lock(DWORD dwMilliseconds) {

	return m_LockObject.Lock(dwMilliseconds);
}

BOOL 
ShareMemUImp::Unlock() {

	return m_LockObject.Unlock();
}

BOOL 
ShareMemUImp::SetShareEvent() {

	return m_LockObject.SetShareEvent();
}

BOOL
ShareMemUImp::ResetShareEvent() {

	return m_LockObject.ResetShareEvent();
}

BOOL
ShareMemUImp::ReleaseShareSemaphore(LONG lReleaseCount,
	LPLONG lpPreviousCount) {

	return m_LockObject.ReleaseShareSemaphore(lReleaseCount,
		lpPreviousCount);
}

ShareMemUImp::ShareMemUImp() :
	m_hMapFile(NULL), 
	m_hMapView(NULL),
	m_hMapSize(0) {

}

ShareMemUImp::~ShareMemUImp() {

	UninitializeShareMem();
}

BOOL 
ShareMemUImp::InitializeShareMem(HANDLE hFile,
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

	m_hMapFile = CreateFileMappingW(hFile,
		lpAttributes, 
		flProtect,
		0,
		size,
		lpName);

	BOOLEAN init;
	init = FALSE;

	if (m_hMapFile) 
		init = GetLastError() != ERROR_ALREADY_EXISTS ? 1 : 0;
	else {
		AlterNamedSecurityInfo(lpName, 
			dwDesiredAccess);

		m_hMapFile = OpenFileMappingW(dwDesiredAccess,
			FALSE,
			lpName);
	}

	if (m_hMapFile) {
		m_hMapView = MapViewOfFile(m_hMapFile,
			dwDesiredAccess,
			0,
			0,
			0);
		if (m_hMapView) {
			m_hMapSize = dwMaximumSize;

			ShareMemHeader* smh;
			smh = (ShareMemHeader*)m_hMapView;
			if (init) {	
				swprintf_s(smh->m_LockName, 
					L"U-%08x-%08x-%08x", 
					GetCurrentProcessId(),
					GetCurrentThreadId(),
					GetTickCount());

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
ShareMemUImp::UninitializeShareMem() {

	m_LockObject.UninitializeShareLock();

	if (m_hMapView) {
		UnmapViewOfFile(m_hMapView);
		m_hMapView = NULL;
	}
	if (m_hMapFile) {
		CloseHandle(m_hMapFile);
		m_hMapFile = NULL;
	}
	m_hMapSize = 0;
}

DWORD 
ShareMemUImp::AlterNamedSecurityInfo(LPCWSTR lpName,
	DWORD dwDesiredAccess) {

	DWORD err;

	PACL dacl;
	PSECURITY_DESCRIPTOR psd;
	err = GetNamedSecurityInfoW(lpName, 
		SE_KERNEL_OBJECT,
		DACL_SECURITY_INFORMATION,
		NULL, 
		NULL, 
		&dacl, 
		NULL, 
		&psd);
	if (err != ERROR_SUCCESS)
		goto error_exit;

	int i;
	for(i=0; i<dacl->AceCount; i++) {
		PACE_HEADER header;
		if (!GetAce(dacl, 
			i, 
			(LPVOID*)&header))
			continue;

		if (header->AceType != ACCESS_ALLOWED_ACE_TYPE)
			continue;

		ACCESS_ALLOWED_ACE* allow;
		allow = (ACCESS_ALLOWED_ACE*)header;

		allow->Mask |= dwDesiredAccess;
	}

	err = SetNamedSecurityInfoW((LPWSTR)lpName,
		SE_KERNEL_OBJECT,
		DACL_SECURITY_INFORMATION,
		NULL,
		NULL, 
		dacl,
		NULL);

error_exit:
	
	if (psd)
		LocalFree(psd);

	return err;
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

	return ((ShareMem*)sm)->InitializeMutex(lpMutexAttributes,
		bInitialOwner);
}

BOOL 
InitializeShareMemEvent(HANDLE sm,
	LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState) {

	return ((ShareMem*)sm)->InitializeEvent(lpEventAttributes,
		bManualReset,
		bInitialState);
}

BOOL 
InitializeShareMemSemaphore(HANDLE sm,
	LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount) {

	return ((ShareMem*)sm)->InitializeSemaphore(lpSemaphoreAttributes,
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
