#pragma once

#include "_private_ext_k.h"

#define TAG	'txep'

char
map_mem_kernel_to_user(
	void* k_addr,
	unsigned long k_size,
	PMDL* k_mdl,
	void** u_addr) {

	PMDL mdl;
	void* addr;
	mdl = NULL;
	addr = NULL;

	if (!k_addr ||
		!k_size)
		goto error_exit;

	mdl = IoAllocateMdl(
		k_addr,
		k_size,
		FALSE,
		FALSE,
		NULL);		
	if (!mdl)
		goto error_exit;

	MmBuildMdlForNonPagedPool(mdl);

	__try {
		addr = MmMapLockedPagesSpecifyCache(
			mdl, 
			UserMode,
			MmNonCached,
			NULL, 
			FALSE,
			NormalPagePriority);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		goto error_exit;
	}

	if (!addr)
		goto error_exit;

	if ( k_mdl)
		*k_mdl = mdl;
	
	if ( u_addr)
		*u_addr = addr;
	
	return true;

error_exit:

	unmap_mem_kernel_to_user(mdl, addr);
	
	return false;
}


struct MAP_MEM_K*
map_mem_kernel_to_user_ex(
	void* k_addr,
	unsigned long k_size) {

	struct MAP_MEM_K* map_mem_k;
	map_mem_k = NULL;

	if (!k_addr ||
		!k_size)
		goto error_exit;
	
	map_mem_k = (struct MAP_MEM_K*)ExAllocatePoolWithTag(
		PagedPool,
		sizeof(struct MAP_MEM_K),
		TAG);
	if (!map_mem_k)
		goto error_exit;
	
	RtlZeroMemory(
		map_mem_k,
		sizeof(struct MAP_MEM_K));

	if (!map_mem_kernel_to_user(k_addr,
		k_size,
		&map_mem_k->m_mdl,
		&map_mem_k->m_map_mem_u.m_addr))
		goto error_exit;

	map_mem_k->m_map_mem_u.m_size = k_size;
	
	return map_mem_k;

error_exit:

	if (map_mem_k) {
		unmap_mem_kernel_to_user(
			map_mem_k->m_mdl, 
			map_mem_k->m_map_mem_u.m_addr);
		ExFreePool(map_mem_k);
		map_mem_k = NULL;
	}
	return map_mem_k;
}

char
map_mem_kernel_to_user_with_map_mem_k(
	void* k_addr,
	unsigned long k_size,
	struct MAP_MEM_K* map_mem_k) {

	if (!map_mem_k)
		goto error_exit;
	
	RtlZeroMemory(
		map_mem_k, 
		sizeof(struct MAP_MEM_K));
	
	if (!k_addr ||
		!k_size)
		goto error_exit;
		
	if (!map_mem_kernel_to_user(k_addr,
		k_size,
		&map_mem_k->m_mdl,
		&map_mem_k->m_map_mem_u.m_addr))
		goto error_exit;

	map_mem_k->m_map_mem_u.m_size = k_size;
	
	return true;

error_exit:

	if (map_mem_k)
		unmap_mem_kernel_to_user(
			map_mem_k->m_mdl, 
			map_mem_k->m_map_mem_u.m_addr);

	return false;

}

void 
unmap_mem_kernel_to_user(
	PMDL k_mdl,
	void* u_addr) {

	if (k_mdl) {
		if (u_addr) {
			MmUnmapLockedPages(u_addr, k_mdl);
			u_addr = NULL;
		}
		IoFreeMdl(k_mdl);
		k_mdl = NULL;
	}
}

