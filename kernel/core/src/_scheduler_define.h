#pragma once

#include "_scheduler.h"
#include "_share_mem_if.h"

#define INFINITE 0xFFFFFFFF

#define safe_free_scheduler_switch(ss) {		\
												\
	if (ss) {									\
		set_caller_sleep(ss);					\
		set_callee_ready(ss);					\
		delref_scheduler_switch(ss);			\
		ss = NULL;								\
	}											\
}

#define acquire_share_mem_lock(sm, lock) {		\
												\
	if (lock)									\
		LockShareMem(sm,						\
			INFINITE);							\
}

#define release_share_mem_lock(sm, lock) {		\
												\
	if (lock)									\
		UnlockShareMem(sm);						\
}

#define get_content(ssam)				((struct scheduler_share_affinity_content*)((LPBYTE)ssam + ssam->m_content_offset))

#define get_rollback_content(ssam)		((struct scheduler_share_affinity_content*)((LPBYTE)ssam + ssam->m_rollback_content_offset))

inline void prepare_rollback(struct scheduler_share_affinity_manager* ssam) {

	RtlCopyMemory(get_rollback_content(ssam),
		get_content(ssam), 
		ssam->m_content_size);
	ssam->m_rollback = 1;
}

inline void restore_rollback(struct scheduler_share_affinity_manager* ssam) {
							
	if (ssam->m_rollback) {
		RtlCopyMemory(get_content(ssam),
			get_rollback_content(ssam),
			ssam->m_content_size);
		ssam->m_rollback = 0;
	}
}

inline void release_rollback(struct scheduler_share_affinity_manager* ssam) {

	ssam->m_rollback = 0;
}

inline char SORAAPI unregister_scheduler_no_latency(struct scheduler_dispatcher* sd, long entry, char lock) {

	if (entry != -1) {
		acquire_share_mem_lock(sd->m_sharemem,
			lock);

		restore_rollback(sd->m_ssa_manager);
		
		prepare_rollback(sd->m_ssa_manager);

		sd->m_ssa_content->m_concurrent_running -= sd->m_ssa_content->m_register[entry].m_concurrent_running;
		sd->m_ssa_content->m_affinity |= sd->m_ssa_content->m_register[entry].m_affinity;

		RtlZeroMemory(&sd->m_ssa_content->m_register[entry], 
			sizeof(struct scheduler_share_affinity_register));

		release_rollback(sd->m_ssa_manager);

		release_share_mem_lock(sd->m_sharemem,
			lock)
		return true;		
	}
	return false;
}

inline KAFFINITY SORAAPI acquire_affinity(struct scheduler_no_latency* snl) {

	KAFFINITY affinity;
	affinity = 0;
	if (snl->m_register_entry != -1) {
		acquire_share_mem_lock(snl->m_dispatcher->m_sharemem,
			1);

		restore_rollback(snl->m_dispatcher->m_ssa_manager);	

		unsigned long index;
#ifdef _M_X64
		if (BitScanReverse64(&index, snl->m_dispatcher->m_ssa_content->m_affinity)) {
#else
		if (BitScanReverse(&index, snl->m_dispatcher->m_ssa_content->m_affinity)) {
#endif			
			
			affinity = (KAFFINITY)(0x1 << index);
			
			prepare_rollback(snl->m_dispatcher->m_ssa_manager);
			
			snl->m_dispatcher->m_ssa_content->m_affinity &= ~affinity;
			
			snl->m_dispatcher->m_ssa_content->m_register[snl->m_register_entry].m_affinity |= affinity;
				
			release_rollback(snl->m_dispatcher->m_ssa_manager);			
		}
		release_share_mem_lock(snl->m_dispatcher->m_sharemem,
			1);
	}
	return affinity;
}

inline void SORAAPI release_affinity(struct scheduler_no_latency* snl, KAFFINITY affinity) {

	if (snl->m_register_entry != -1) {
		acquire_share_mem_lock(snl->m_dispatcher->m_sharemem,
			1);

		restore_rollback(snl->m_dispatcher->m_ssa_manager);

		prepare_rollback(snl->m_dispatcher->m_ssa_manager);

		snl->m_dispatcher->m_ssa_content->m_affinity |= affinity;
		
		snl->m_dispatcher->m_ssa_content->m_register[snl->m_register_entry].m_affinity &= ~affinity;
			
		release_rollback(snl->m_dispatcher->m_ssa_manager);

		release_share_mem_lock(snl->m_dispatcher->m_sharemem,
			1);
	}
}
