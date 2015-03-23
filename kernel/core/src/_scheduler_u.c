#include "_scheduler.h"
#include "_scheduler_define.h"
#include "_scheduling_define.h"
#include "thread_if.h"
#include "_share_mem_if.h"
#include "_auto_mutex_u.h"
#include "fiber_tracker.h"

#define WAIT_OBJECT_1	(WAIT_OBJECT_0 + 1)

#pragma warning(disable:4018)

#define allocate_scheduler_user_routine(c)								\
	(struct scheduler_user_routine*)new char[sizeof(struct scheduler_user_routine) + c * sizeof(struct scheduler_user_routine::user_routine)]

#define free_scheduler_user_routine(u) {								\
	if (u) {															\
		unsigned long i;												\
		for(i=0; i<u->m_count; i++)										\
			((fiber_tracker*)u->m_routine[i].m_user_args)->release();	\
		delete u;														\
		u = NULL;														\
	}																	\
}

void validate_register(struct scheduler_dispatcher* sd, long entry, char lock) {

	if (entry != -1) {
		acquire_share_mem_lock(sd->m_sharemem,
			lock);

		if (entry < sd->m_ssa_manager->m_max_concurrent_running)
			if (sd->m_ssa_content->m_register[entry].m_process_id.m_dword) {
				HANDLE proc;
				proc = OpenProcess(SYNCHRONIZE,
					FALSE,
					sd->m_ssa_content->m_register[entry].m_process_id.m_dword);
				if (proc)
					CloseHandle(proc);
				else
					unregister_scheduler_no_latency(sd,
						entry,
						0);
			}

		release_share_mem_lock(sd->m_sharemem,
			lock);	
	}
}

char initialize_scheduler_enlist(struct scheduler_enlist* slist) {

	uninitialize_scheduler_enlist(slist);

	InitializeListHead(&slist->m_head.m_entry);
	slist->m_sync = CreateMutex(NULL, FALSE, NULL);
	slist->m_head.m_value = NULL;
	return 1;
}

void uninitialize_scheduler_enlist(struct scheduler_enlist* slist) {

	if (slist->m_sync) {
		CloseHandle(slist->m_sync);
		slist->m_sync = NULL;
	}
}

struct LIST_ENTRY_EX* SORAAPI insert_tail(struct scheduler_enlist* slist, void*  value) {

	WaitForSingleObject(slist->m_sync, INFINITE);
	struct LIST_ENTRY_EX* entry;
	entry = (struct LIST_ENTRY_EX*)malloc(sizeof(struct LIST_ENTRY_EX));
	entry->m_value = value;
	InsertTailList(&slist->m_head.m_entry, &entry->m_entry);
	ReleaseMutex(slist->m_sync);
	return entry;
}

char SORAAPI remove_head(struct scheduler_enlist* slist, void** data) {

	WaitForSingleObject(slist->m_sync, INFINITE);
	if (IsListEmpty(&slist->m_head.m_entry)) {
		ReleaseMutex(slist->m_sync);
		return false;
	}
	struct LIST_ENTRY_EX* entry;
	entry = (LIST_ENTRY_EX*)RemoveHeadList(&slist->m_head.m_entry);
	*data = entry->m_value;
	free(entry);
	ReleaseMutex(slist->m_sync);
	return true;
}

struct scheduler_switch* allocate_scheduler_switch() {

	struct scheduler_switch* ss;
	ss = (struct scheduler_switch*)malloc(sizeof(struct scheduler_switch));
	ss->m_ref = 1;
	ss->m_affinity = 0;
	ss->m_caller_sleep = 0;
	ss->m_callee_ready = 0;
	return ss;
}

long addref_scheduler_switch(struct scheduler_switch* ss) {

	return InterlockedIncrement(&ss->m_ref); 
}

long delref_scheduler_switch(struct scheduler_switch* ss) {

	long ref;
	ref = InterlockedDecrement(&ss->m_ref);
	switch(ref) {
	case 0:
		free(ss);
		break;
	default:
		break;
	}
	return ref;	
}

void set_caller_sleep(struct scheduler_switch* ss) {

	ss->m_caller_sleep = 1;
}

void set_callee_ready(struct scheduler_switch* ss) {

	ss->m_callee_ready = 1;
}

char get_caller_sleep(struct scheduler_switch* ss) {

	return ss->m_caller_sleep;
}

char get_callee_ready(struct scheduler_switch* ss) {

	return ss->m_callee_ready;
}

char SORAAPI on_wait_object_0(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	safe_free_scheduler_switch(st->m_switch);

	if (remove_head(st->m_enlist, (void**)&st->m_switch)) {
		st->m_affinity = st->m_switch->m_affinity;
		if (st->m_affinity)
			SetThreadAffinityMask(GetCurrentThread(), st->m_affinity);
		set_callee_ready(st->m_switch);
		while(1) {
			if (get_caller_sleep(st->m_switch))
				break;
			if (st->m_scheduler->m_stop)
				break;
		}		
		st->m_last_switch_timestamp.QuadPart = GetTickCount();
		
		delref_scheduler_switch(st->m_switch);
		st->m_switch = NULL;
	}
	else {
		while(1) {
			st->m_affinity = acquire_affinity_u(st);
			if (st->m_affinity)
				break;
		}
		if (st->m_affinity)
			SetThreadAffinityMask(GetCurrentThread(), st->m_affinity);
	}
	st->m_doproc = do_scheduler_wakeup_callee;
	return 1;
}

char SORAAPI on_wait_object_1(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	return 0;
}

char SORAAPI on_default(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	return 0;
}

unsigned do_scheduler_process(void* args) {

    // Convert current thread to fiber
    ConvertThreadToFiber(NULL);
	
	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

    while(1) {
		if (st->m_doproc(args)) {
			if (st->m_scheduler->m_stop) 
				break;
			struct scheduler_user_routine* sur;
			sur = (struct scheduler_user_routine*)InterlockedPopEntrySList(&st->m_scheduler->m_garbage_user_routine);
			if (sur)
				free_scheduler_user_routine(sur);
			if (st->m_scheduler->m_scheduing) {
				if (st->m_scheduler->m_scheduing(st->m_scheduler))
					continue;
			}
			st->m_scheduler->m_stop = 1;
			if (st->m_scheduler->m_event_control)
				SetEvent(st->m_scheduler->m_event_control); 				
			break;
		}
		break;
	}
	struct scheduler_switch* ss;
	while(remove_head(st->m_enlist, (void**)&ss)) {
		release_affinity(st->m_scheduler, ss->m_affinity);
		safe_free_scheduler_switch(ss);
	}
	release_affinity(st->m_scheduler, st->m_affinity);
	st->m_affinity = 0;
	safe_free_scheduler_switch(st->m_switch);

    // Convert current fiber to thread
    ConvertFiberToThread();
	return 0;
}

char do_scheduler_wakeup_callee(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	safe_free_scheduler_switch(st->m_switch);

	KAFFINITY affinity;
	affinity = acquire_affinity_u(st);
	if (affinity) {
		st->m_switch = allocate_scheduler_switch();
		st->m_switch->m_affinity = affinity;
		addref_scheduler_switch(st->m_switch);
		insert_tail(st->m_enlist, st->m_switch);

		ReleaseSemaphore(st->m_scheduler->m_semaphore_control, 1, NULL);

		st->m_doproc = do_scheduler_check_callee;
	}
	return 1;
}

char do_scheduler_check_callee(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	if (st->m_switch)
		if (get_callee_ready(st->m_switch)) {			
			set_caller_sleep(st->m_switch);			
			delref_scheduler_switch(st->m_switch);
			st->m_switch = NULL;			
			return do_scheduler_wait_caller(args);
		}
	return 1;
}

char do_scheduler_wait_caller(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	release_affinity(st->m_scheduler, st->m_affinity);
	st->m_affinity = 0;

	DWORD_PTR process_affinity;
	DWORD_PTR system_affinity;
	GetProcessAffinityMask(GetCurrentProcess(), &process_affinity, &system_affinity);
	SetThreadAffinityMask(GetCurrentThread(), system_affinity & ~1);

	HANDLE object[2];
	object[0] = st->m_scheduler->m_semaphore_control;
	object[1] = st->m_scheduler->m_event_control;
	switch(WaitForMultipleObjects(2,
		object,
		FALSE, 
		INFINITE)) {
	case WAIT_OBJECT_0:
		return on_wait_object_0(args);
		break;
	case WAIT_OBJECT_1:
		return on_wait_object_1(args);
		break;
	default:
		return on_default(args);
		break;
	}
	return 0;
}

long SORAAPI register_scheduler_no_latency(struct scheduler_dispatcher* sd, struct scheduler_no_latency* snl, char lock) {

	acquire_share_mem_lock(sd->m_sharemem,
		lock);

	restore_rollback(sd->m_ssa_manager);

	if (sd->m_ssa_content->m_concurrent_running + snl->m_concurrent_running > sd->m_ssa_manager->m_max_concurrent_running) {
		unsigned long i;
		for(i=0; i<sd->m_ssa_manager->m_max_concurrent_running; i++)
			validate_register(sd,
				i,
				0);
		if (sd->m_ssa_content->m_concurrent_running + snl->m_concurrent_running > sd->m_ssa_manager->m_max_concurrent_running) {
			release_share_mem_lock(sd->m_sharemem,
				lock);
			return -1;
		}
	}

	unsigned long i;
	for(i=0; i<sd->m_ssa_manager->m_max_concurrent_running; i++) {
		if (sd->m_ssa_content->m_register[i].m_process_id.m_dword)
			continue;

		prepare_rollback(sd->m_ssa_manager);

		RtlZeroMemory(&sd->m_ssa_content->m_register[i], 
			sizeof(struct scheduler_share_affinity_register));
		
		sd->m_ssa_content->m_register[i].m_concurrent_running = snl->m_concurrent_running;
		sd->m_ssa_content->m_register[i].m_process_id.m_dword = GetCurrentProcessId();

		sd->m_ssa_content->m_concurrent_running += sd->m_ssa_content->m_register[i].m_concurrent_running;

		release_rollback(sd->m_ssa_manager);

		release_share_mem_lock(sd->m_sharemem,
			lock);
		return i;
	}	
	release_share_mem_lock(sd->m_sharemem,
		lock);
	return -1;
}

KAFFINITY SORAAPI acquire_affinity_u(struct scheduler_thread* st) {

	KAFFINITY affinity;
	affinity = 0;
	LARGE_INTEGER timestamp;
	timestamp.QuadPart = GetTickCount();
	if ((timestamp.QuadPart-st->m_last_switch_timestamp.QuadPart) >= (LONGLONG)schedule_switch_time ||
		(st->m_last_switch_timestamp.QuadPart-timestamp.QuadPart) >= (LONGLONG)schedule_switch_time) {
		affinity = acquire_affinity(st->m_scheduler);
		if (affinity)
			st->m_fail_count = 0;
		else {
			st->m_fail_count ++;

			if (st->m_fail_count < validate_count)
				return affinity;			

			st->m_validate_entry ++;
			st->m_validate_entry %= st->m_scheduler->m_dispatcher->m_ssa_manager->m_max_concurrent_running;
			
			validate_register(st->m_scheduler->m_dispatcher,
				st->m_validate_entry,
				1);
			
			st->m_fail_count = 0;
		}
	}
	return affinity;
}

HANDLE SoraThreadAlloc() {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)malloc(sizeof(struct scheduler_no_latency));
	memset(snl, 0, sizeof(struct scheduler_no_latency));
	snl->m_dispatcher = NULL;
	snl->m_concurrent_running = 1;
	snl->m_total_runnable = 2;
	snl->m_register_entry = -1;
	snl->m_user_routine = NULL;
	InitializeSListHead(&snl->m_garbage_user_routine);
	snl->m_semaphore_control = NULL;	
	snl->m_event_control = CreateEvent(NULL,
		TRUE,
		TRUE,
		NULL);
	initialize_scheduler_enlist(&snl->m_enlist);
	snl->m_thread_pool = (struct scheduler_thread*)malloc(snl->m_total_runnable*sizeof(struct scheduler_thread));
	{
		unsigned long i;
		for(i=0; i<snl->m_total_runnable; i++) {
			struct scheduler_thread* st;
			st = snl->m_thread_pool + i;
			st->m_scheduler = snl;
			st->m_enlist = &snl->m_enlist;
			st->m_switch = NULL;
			st->m_doproc = NULL;
			st->m_thread_object = NULL;			
			st->m_priority = REALTIME_PRIORITY_CLASS;
			st->m_affinity = 0;
			st->m_last_switch_timestamp.u.HighPart = 0;
			st->m_last_switch_timestamp.u.LowPart = 0;
			st->m_fail_count = 0;
			st->m_validate_entry = 0;
		}
	}
	snl->m_scheduing = NULL;
	snl->m_scheduing_info.index = -1;
	snl->m_lock = CreateMutex(
		NULL, 
		FALSE,
		NULL);		
	return (HANDLE)snl;
}

VOID SoraThreadFree(HANDLE Thread) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread;
	AutoMutex am(&snl->m_lock);
	
	SoraThreadStop(Thread);	
	if (snl->m_event_control) {
		CloseHandle(snl->m_event_control);
		snl->m_event_control = NULL;
	}
	uninitialize_scheduler_enlist(&snl->m_enlist);
	if (snl->m_thread_pool) {
		free(snl->m_thread_pool);
		snl->m_thread_pool = NULL;
	}
	am.Unlock();
	am.Detach();
	CloseHandle(snl->m_lock);
	free(Thread);
}

BOOL initialize_sora_affinity(HANDLE sm,
	void* context) {

	struct scheduler_share_affinity_manager* ssam;
	ssam = (struct scheduler_share_affinity_manager*)GetShareMemAddress(sm);

	struct scheduler_share_initialization* ssi;
	ssi = (struct scheduler_share_initialization*)context;

	RtlZeroMemory(ssam, 
		sizeof(struct scheduler_share_affinity_manager) + 2 * ssi->m_content_size);

	ssam->m_max_concurrent_running = ssi->m_max_concurrent_running;
	ssam->m_content_size = ssi->m_content_size;
	ssam->m_content_offset = sizeof(struct scheduler_share_affinity_manager) + 
		0 * ssam->m_content_size;

	struct scheduler_share_affinity_content* content;
	content = get_content(ssam);

	content->m_affinity = ssi->m_affinity;	
	
	ssam->m_rollback_content_offset = sizeof(struct scheduler_share_affinity_manager) + 
		1 * ssam->m_content_size;

	return TRUE;
}

BOOLEAN SoraThreadStart(HANDLE Thread, PSORA_UTHREAD_PROC User_Routine, PVOID User_Context) {

	return SoraThreadStartEx(Thread, &User_Routine, &User_Context, 1, ROUND_ROBIN);
}

BOOLEAN SoraThreadStartEx(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, PVOID* User_Context, ULONG Count, SCHEDULING_TYPE Type) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread;
	AutoMutex am(&snl->m_lock);	

	SoraThreadStop(Thread);

	snl->m_dispatcher = (struct scheduler_dispatcher*)malloc(sizeof(struct scheduler_dispatcher));

	RtlZeroMemory(snl->m_dispatcher, 
		sizeof(struct scheduler_dispatcher));

	scheduler_share_initialization ssi = { 0 };

	DWORD_PTR process;
	DWORD_PTR system;
	GetProcessAffinityMask(GetCurrentProcess(),
		&process,
		&system);

	ssi.m_affinity = system & ~1;
	unsigned long i;
	for(i=0; i < sizeof(KAFFINITY)<<3; i++)
		if (ssi.m_affinity & ((KAFFINITY)1)<<i)
			ssi.m_max_concurrent_running++;
	if (ssi.m_max_concurrent_running)
		ssi.m_max_concurrent_running--;

	if (ssi.m_max_concurrent_running) {
		ssi.m_content_size = sizeof(struct scheduler_share_affinity_content) + ssi.m_max_concurrent_running * sizeof(scheduler_share_affinity_register);
		
		snl->m_dispatcher->m_sharemem = AllocateShareMem(INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE, 
			sizeof(struct scheduler_share_affinity_manager) + 2 * ssi.m_content_size,
			L"SoraAffinity",
			FILE_MAP_READ|FILE_MAP_WRITE,
			initialize_sora_affinity,
			&ssi);
		if (snl->m_dispatcher->m_sharemem) {
			snl->m_dispatcher->m_ssa_manager = (struct scheduler_share_affinity_manager*)GetShareMemAddress(snl->m_dispatcher->m_sharemem);
			snl->m_dispatcher->m_ssa_content = get_content(snl->m_dispatcher->m_ssa_manager);
			if (InitializeShareMemNamedSpinlock(snl->m_dispatcher->m_sharemem)) {
				snl->m_register_entry = register_scheduler_no_latency(snl->m_dispatcher,
					snl,
					1);
				if (snl->m_register_entry != -1) {
					snl->m_semaphore_control = CreateSemaphore(NULL,
						snl->m_concurrent_running,
						snl->m_total_runnable,
						NULL);
					snl->m_user_routine = allocate_scheduler_user_routine(Count);
					snl->m_user_routine->m_count = Count;
					unsigned long i;
					for(i=0; i<snl->m_user_routine->m_count; i++) {
						snl->m_user_routine->m_routine[i].m_user_proc = &fiber_tracker::fiber_call;
						snl->m_user_routine->m_routine[i].m_user_args = new fiber_tracker(User_Routine[i], User_Context[i]);
						snl->m_user_routine->m_routine[i].m_user_return = TRUE;
					}
					switch(Type) {
					case ROUND_ROBIN:
						snl->m_scheduing = Scheduling_RoundRobin;
						snl->m_scheduing_info.index = -1;
						break;
					default:
						snl->m_scheduing = NULL;
						snl->m_scheduing_info.index = -1;
						break;
					}
					if (snl->m_user_routine &&
						snl->m_scheduing) {
						if (snl->m_event_control)
							ResetEvent(snl->m_event_control);
						snl->m_stop = 0;
						if (snl->m_thread_pool) {
							unsigned long i;
							for(i=0; i<snl->m_total_runnable; i++) {
								struct scheduler_thread* st;
								st = snl->m_thread_pool + i;
								st->m_doproc = do_scheduler_wait_caller;
								st->m_thread_object = (HANDLE)_beginthreadex(NULL,
									0,
									do_scheduler_process,
									st, 
									0,
									NULL);
								SetThreadPriority(st->m_thread_object,
									st->m_priority);
							}
						}
						SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
						return 1;
					}
				}				
			}
		}
	}
	SoraThreadStop(Thread);
	return 0;
}

VOID SoraThreadStop(HANDLE Thread) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread;
	AutoMutex am(snl->m_lock);
	
	snl->m_stop = 1;
	if (snl->m_event_control)
		SetEvent(snl->m_event_control); 
	if (snl->m_thread_pool) {
		unsigned long i;
		for(i=0; i<snl->m_total_runnable; i++) {
			struct scheduler_thread* st;
			st = snl->m_thread_pool + i;
			if (st->m_thread_object) {
				WaitForSingleObject(st->m_thread_object, INFINITE);
				CloseHandle(st->m_thread_object);
				st->m_thread_object = NULL;
			}			
		}
	}
	if (snl->m_dispatcher) {
		if (snl->m_register_entry != -1) {
			unregister_scheduler_no_latency(snl->m_dispatcher,
				snl->m_register_entry,
				1);
			snl->m_register_entry = -1;
		}
		if (snl->m_dispatcher->m_sharemem) {
			FreeShareMem(snl->m_dispatcher->m_sharemem);
			snl->m_dispatcher->m_sharemem = NULL;
		}
		free(snl->m_dispatcher);
		snl->m_dispatcher = NULL;
	}
	if (snl->m_semaphore_control) {
		CloseHandle(snl->m_semaphore_control);
		snl->m_semaphore_control = NULL;
	}
	free_scheduler_user_routine(snl->m_user_routine);

	snl->m_scheduing = NULL;
	snl->m_scheduing_info.index = -1;
	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
}

BOOLEAN SoraThreadJoin(HANDLE Thread, ULONG Timeout) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread; 
	HANDLE* object;
	object = NULL;
	unsigned long count;
	count = 0;
	{
		AutoMutex am(&snl->m_lock);
		if (snl->m_thread_pool) {
			if (snl->m_total_runnable) {
				object = (HANDLE*)new char[snl->m_total_runnable * sizeof(HANDLE)];
				unsigned long i;
				for(i=0; i<snl->m_total_runnable; i++) {
					struct scheduler_thread* st;
					st = snl->m_thread_pool + i;
					if (st->m_thread_object) {
						object[count] = st->m_thread_object;
						count++;
					}
				}
			}
		}
	}
	if (object &&
		count) {
		DWORD status;
		status = WaitForMultipleObjects(
			count, 
			object, 
			TRUE,
			Timeout);
		delete object;
		if (status > WAIT_OBJECT_0 + count - 1)
			return FALSE;
	}
	return TRUE;
}

/*
We always force the concurrent running thread to 1 now, so we just simply implement the SoraThreadAddUserRoutine and SoraThreadDelUserRoutine. 
If more than 1 concurrent running thread is required after, more complicated design is required. 
Such as addref & release is required for scheduler_user_routine.
*/
BOOLEAN SoraThreadAddUserRoutine(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, PVOID* User_Context, ULONG Count) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread;
	AutoMutex am(&snl->m_lock);
	
	struct scheduler_user_routine* sur;
	sur = snl->m_user_routine;
	if (sur) {
		unsigned long count;
		count = sur->m_count + Count;
		struct scheduler_user_routine* new_sur;
		new_sur = allocate_scheduler_user_routine(count);
		new_sur->m_count = 0;
		unsigned long i;
		for(i=0; i<sur->m_count; i++)
			if (sur->m_routine[i].m_user_return) {
				new_sur->m_routine[new_sur->m_count] = sur->m_routine[i];
				fiber_tracker* ft;
				ft = (fiber_tracker*)sur->m_routine[new_sur->m_count].m_user_args;
				ft->addref();
				new_sur->m_count++;
			}
		for(i=0; i<Count; i++) {
			new_sur->m_routine[new_sur->m_count].m_user_proc = &fiber_tracker::fiber_call;
			new_sur->m_routine[new_sur->m_count].m_user_args = new fiber_tracker(User_Routine[i], User_Context[i]);
			new_sur->m_count++;
		}
		snl->m_user_routine = new_sur;
		InterlockedPushEntrySList(&snl->m_garbage_user_routine, 
			&sur->m_entry);
		return TRUE;
	}
	return FALSE;
}

BOOLEAN SoraThreadDelUserRoutine(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, ULONG Count) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread; 	
	AutoMutex am(&snl->m_lock);

	struct scheduler_user_routine* sur;
	sur = snl->m_user_routine;
	if (sur) {
		unsigned long count;
		count = sur->m_count;
		struct scheduler_user_routine* new_sur;
		new_sur = allocate_scheduler_user_routine(count);
		new_sur->m_count = 0;
		unsigned long i;
		unsigned long j;
		char match;
		for(i=0; i<sur->m_count; i++) {
			match = false;
			for(j=0; j<Count; j++) {
				fiber_tracker* ft;
				ft = (fiber_tracker*)sur->m_routine[i].m_user_args; 
				if (ft->get_user_routine() != User_Routine[j])
					continue;
				match = true;
				break;
			}
			if (match)
				continue;			
			if (sur->m_routine[i].m_user_return) {
				new_sur->m_routine[new_sur->m_count] = sur->m_routine[i];
				fiber_tracker* ft;
				ft = (fiber_tracker*)new_sur->m_routine[new_sur->m_count].m_user_args; 
				ft->addref();
				new_sur->m_count++;
			}
		}
		snl->m_user_routine = new_sur;
		InterlockedPushEntrySList(&snl->m_garbage_user_routine,
				&sur->m_entry);
		return TRUE;
	}
	return FALSE;
}

#ifdef USER_MODE
void 
SoraThreadCompareWaitGt(long* var, long val) {

	while(!(*(volatile long*)var > val))
		SoraThreadYield(TRUE);
}

void 
SoraThreadCompareWaitLt(long* var, long val) {

	while(!(*(volatile long*)var < val)) 
		SoraThreadYield(TRUE);
}

void 
SoraThreadCompareWait(long* var, long val) {

	while(!(*(volatile long*)var == val)) 
		SoraThreadYield(TRUE);
}
#endif

