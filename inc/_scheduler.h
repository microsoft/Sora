#pragma once

#ifdef USER_MODE
#include "sora.h"
#else
#include <windef.h>
#include "const.h"
#include "thread_if.h"
#include "_share_mem_if.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef char SORAAPI scheduler_doproc(void* args);

typedef char SORAAPI scheduling(void* args);

#define	schedule_switch_time	  8	// ms

#define validate_count			128 

struct LIST_ENTRY_EX;
struct scheduler_enlist;
struct scheduler_switch;
struct scheduler_thread;
struct scheduler_dispatcher;
struct scheduler_no_latency;

#ifndef _LIST_ENTRY_EX_
#define _LIST_ENTRY_EX_

struct LIST_ENTRY_EX {

	LIST_ENTRY m_entry;
	void* m_value;
};

#endif

struct scheduler_enlist {

	struct LIST_ENTRY_EX m_head;
	KMUTEX m_sync;	
};

char SORAAPI initialize_scheduler_enlist(struct scheduler_enlist* slist);
void SORAAPI uninitialize_scheduler_enlist(struct scheduler_enlist* slist);
struct LIST_ENTRY_EX* SORAAPI insert_tail(struct scheduler_enlist* slist, void*  data);
char SORAAPI remove_entry(struct scheduler_enlist* slist, struct LIST_ENTRY_EX* entry);
char SORAAPI remove_head(struct scheduler_enlist* slist, void** data);

struct scheduler_switch {

	long m_ref;
	volatile char m_caller_sleep;
	volatile char m_callee_ready;
	KAFFINITY m_affinity;
};

struct scheduler_switch* SORAAPI allocate_scheduler_switch();
long SORAAPI addref_scheduler_switch(struct scheduler_switch* ss);
long SORAAPI delref_scheduler_switch(struct scheduler_switch* ss);
void SORAAPI set_caller_sleep(struct scheduler_switch* ss);
void SORAAPI set_callee_ready(struct scheduler_switch* ss);
char SORAAPI get_caller_sleep(struct scheduler_switch* ss);
char SORAAPI get_callee_ready(struct scheduler_switch* ss);

struct scheduler_share_initialization {
	
	unsigned long m_max_concurrent_running;
	KAFFINITY m_affinity;
	unsigned long m_content_size;	
};

struct scheduler_share_affinity_register {

	unsigned long m_concurrent_running;
	KAFFINITY m_affinity;
	union {
		HANDLE m_handle;
		DWORD m_dword;
	} m_process_id;
};

struct scheduler_share_affinity_content {

	unsigned long m_concurrent_running;
	KAFFINITY m_affinity;
	struct scheduler_share_affinity_register m_register[1];
};

struct scheduler_share_affinity_manager {

	unsigned long m_max_concurrent_running;
	unsigned long m_content_size;
	unsigned long m_rollback;
	unsigned long m_content_offset;	
	unsigned long m_rollback_content_offset;
};

struct scheduler_thread {

	struct scheduler_no_latency* m_scheduler;
	struct scheduler_enlist* m_enlist;
	struct scheduler_switch* m_switch;
	scheduler_doproc* m_doproc;
	void* m_thread_object;	
	KPRIORITY m_priority;
	KAFFINITY m_affinity;
	LARGE_INTEGER m_last_switch_timestamp;
	long m_fail_count;
	long m_validate_entry;
};

struct scheduler_dispatcher {

	HANDLE m_sharemem;
	struct scheduler_share_affinity_manager* m_ssa_manager;
	struct scheduler_share_affinity_content* m_ssa_content;
};

struct scheduler_dispatcher* SORAAPI allocate_scheduler_dispatcher();
void free_scheduler_dispatcher(struct scheduler_dispatcher* sd);
long SORAAPI register_scheduler_no_latency(struct scheduler_dispatcher* sd, struct scheduler_no_latency* snl, char lock);
char SORAAPI unregister_scheduler_no_latency(struct scheduler_dispatcher* sd, long entry, char lock);

struct scheduling_info {

	unsigned long index;
};

#define state_initialized	1 << 0

struct scheduler_user_routine {

	SLIST_ENTRY m_entry;
	unsigned long m_count;
	struct user_routine {
		PSORA_UTHREAD_PROC m_user_proc;
		void* m_user_args;
		BOOLEAN m_user_return;
	} m_routine[1];
};

struct scheduler_no_latency {

	KMUTEX m_lock;	
	struct scheduler_dispatcher* m_dispatcher;
	unsigned long m_concurrent_running;
	unsigned long m_total_runnable;	
	long m_register_entry;
	struct scheduler_user_routine* m_user_routine;
	SLIST_HEADER m_garbage_user_routine;
	KSEMAPHORE m_semaphore_control;
	KEVENT m_event_control;
	volatile char m_stop;	
	struct scheduler_enlist  m_enlist;
	struct scheduler_thread* m_thread_pool;	
	PVOID m_thread_init_dispatcher;
	long m_state;
	scheduling* m_scheduing;
	struct scheduling_info m_scheduing_info;
};

#ifdef USER_MODE
unsigned SORAAPI do_scheduler_process(void* args);
#else
void SORAAPI do_scheduler_process(void* args);
#endif
char SORAAPI do_scheduler_wakeup_callee(void* args);
char SORAAPI do_scheduler_check_callee(void* args);
char SORAAPI do_scheduler_wait_caller(void* args);
char SORAAPI initialize_scheduler_no_latency(struct scheduler_no_latency* snl, unsigned long concurrent_running, unsigned long total_runnable, PSORA_UTHREAD_PROC* proc, void** args, unsigned long count, SCHEDULING_TYPE type);
void SORAAPI uninitialize_scheduler_no_latency(struct scheduler_no_latency* snl);
char SORAAPI start(struct scheduler_no_latency* snl, unsigned long access, POBJECT_ATTRIBUTES attr, char wait_init);
void SORAAPI stop(struct scheduler_no_latency* snl, char wait_init);
void SORAAPI set_priority(struct scheduler_no_latency* snl, unsigned long i, KPRIORITY priority);
void SORAAPI set_affinity(struct scheduler_no_latency* snl, unsigned long i, KAFFINITY affinity);
KAFFINITY SORAAPI acquire_affinity(struct scheduler_no_latency* snl);
KAFFINITY SORAAPI acquire_affinity_k(struct scheduler_thread* st);
KAFFINITY SORAAPI acquire_affinity_u(struct scheduler_thread* st);
void SORAAPI release_affinity(struct scheduler_no_latency* snl, KAFFINITY affinity);

#ifdef __cplusplus
}
#endif
