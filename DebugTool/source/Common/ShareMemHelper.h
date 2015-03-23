#pragma once

#include <Windows.h>
#include <assert.h>
#include "_share_mem_if.h"
#include "_share_lock_if.h"
#include "DACL_Control.h"

static inline ShareLock * AllocateSharedEvent(
		BOOL bManualReset,
		BOOL bInitialState,
		LPCWSTR lpName)
{
	ShareLock * lock = NULL;
	SECURITY_ATTRIBUTES sa;
	CreateDACLWithAllAccess(&sa);
	lock = ShareLock::AllocateShareEvent(&sa, bManualReset, bInitialState, lpName);
	LocalFree(sa.lpSecurityDescriptor);
	return lock;
}

static inline ShareMem * AllocateSharedMem(const wchar_t * name, int size, ShareMemClassInit * initFunc, void * context = 0)
{
	ShareMem * sm = NULL;
	SECURITY_ATTRIBUTES sa;
	BOOL succ = CreateDACLWithAllAccess(&sa);
	sm = ShareMem::AllocateShareMem(
		INVALID_HANDLE_VALUE,
		&sa,
		PAGE_READWRITE,
		size,
		name,
		FILE_MAP_ALL_ACCESS,
		initFunc,
		context
	);
	LocalFree(sa.lpSecurityDescriptor);
	return sm;
}

static inline BOOL DummyShareMemClassInit(ShareMem* sm, void* context)
{ return TRUE; }	// do nothing

#ifdef ASSERT
#undef ASSERT
#endif
