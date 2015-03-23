#pragma once

#include <wdm.h>
#include "_private_ext_u.h"

struct MAP_MEM_K {

	struct MAP_MEM_U m_map_mem_u;
	PMDL m_mdl;
};

struct ACQ_RCB_MEM_K {

	struct ACQ_RCB_MEM_U m_acq_rcb_mem_u;
	PMDL m_mdl;
};

#ifdef __cplusplus
extern "C" {
#endif

char
map_mem_kernel_to_user(
	void* k_addr,
	unsigned long k_size,
	PMDL* k_mdl,
	void** u_addr);

struct MAP_MEM_K*
map_mem_kernel_to_user_ex(
	void* k_addr,
	unsigned long k_size);

char
map_mem_kernel_to_user_with_map_mem_k(
	void* k_addr,
	unsigned long k_size,
	struct MAP_MEM_K* map_mem_k);

void 
unmap_mem_kernel_to_user(
	PMDL k_mdl,
	void* u_addr);

#ifdef __cplusplus
	}
#endif

