#include <Windows.h>
#include <Sddl.h>
#include "_share_mem_if.h"
#include "DpSharedMemAllocator.h"

ShareMem * DpShareMemAllocator::Create(const wchar_t * name, int size, ShareMemClassInit * initFunc, void * context)
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

//DpShareMemAllocator::DpShareMemAllocator()
//{
//
//}

//void * DpShareMemAllocator::ShareToRaw(int sptr)
//{
//
//}
//
//int DpShareMemAllocator::RawToShare(void * rptr)
//{
//
//}

BOOL DpShareMemAllocator::CreateDACLWithAllAccess(SECURITY_ATTRIBUTES * pSA)
{
	char * szSD = "D:"       // Discretionary ACL
		"(A;OICI;GA;;;BG)"     // full control to built-in guests
		"(A;OICI;GA;;;AN)"     // full control to anonymous logon
		"(A;OICI;GA;;;AU)"     // Allow full control to authenticated users
		"(A;OICI;GA;;;BA)";    // Allow full control to administrators

	if (NULL == pSA)
		return FALSE;

	return ConvertStringSecurityDescriptorToSecurityDescriptorA(
		szSD,
		SDDL_REVISION_1,
		&(pSA->lpSecurityDescriptor),
		NULL);
}
