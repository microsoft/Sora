#pragma once

#include "_share_mem_if.h"

class DpShareMemAllocator
{
public:
	static ShareMem * Create(const wchar_t * name, int size, ShareMemClassInit * initFunc, void * context = 0);

private:
	static BOOL CreateDACLWithAllAccess(SECURITY_ATTRIBUTES * pSA);
};
