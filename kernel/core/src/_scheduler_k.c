#include <ntddk.h>
#include <wdm.h>
#include "_scheduler.h"
#include "_scheduler_define.h"
#include "_scheduling_define.h"
#include "thread_if.h"
#include "_share_mem_if.h"
#include "_auto_mutex_k.h"

#define tag	' rds'

#pragma warning(disable:4018)

#define allocate_scheduler_user_routine(c)								\
	(struct scheduler_user_routine*)ExAllocatePoolWithTag(PagedPool, sizeof(struct scheduler_user_routine) + c * sizeof(struct scheduler_user_routine::user_routine), tag);

void validate_register(struct scheduler_dispatcher* sd, long entry, char lock) {

	if (entry != -1) {
		acquire_share_mem_lock(sd->m_sharemem,
			lock);

		if (entry < sd->m_ssa_manager->m_max_concurrent_running)			
			if (sd->m_ssa_content->m_register[entry].m_process_id.m_handle) {
				NTSTATUS status;
				HANDLE proc;
				OBJECT_ATTRIBUTES objattr;
				InitializeObjectAttributes(&objattr,
					NULL,
					0,
					NULL,
					NULL);
				CLIENT_ID cid;
				cid.UniqueProcess = sd->m_ssa_content->m_register[entry].m_process_id.m_handle;
				cid.UniqueThread = NULL;
				status = ZwOpenProcess(&proc,
					SYNCHRONIZE,
					&objattr,
					&cid);
				if (status != STATUS_SUCCESS)
					unregister_scheduler_no_latency(sd,
						entry,
						0);
				else
					ZwClose(proc);	
			}
	
		release_share_mem_lock(sd->m_sharemem,
			lock);	
	}
}

char SORAAPI initialize_scheduler_enlist(struct scheduler_enlist* slist) {

	InitializeListHead(&slist->m_head.m_entry);
	KeInitializeMutex(&slist->m_sync, 0);
	slist->m_head.m_value = NULL;
	return true;
}

void SORAAPI uninitialize_scheduler_enlist(struct scheduler_enlist* slist) {

}

struct LIST_ENTRY_EX* SORAAPI insert_tail(struct scheduler_enlist* slist, void*  value) {

	KeWaitForMutexObject(&slist->m_sync, Executive, KernelMode, FALSE, NULL);
	struct LIST_ENTRY_EX* entry;
	entry = (struct LIST_ENTRY_EX*)ExAllocatePoolWithTag(PagedPool, sizeof(struct LIST_ENTRY_EX), tag);
	entry->m_value = value;
	InsertTailList(&slist->m_head.m_entry, &entry->m_entry);
	KeReleaseMutex(&slist->m_sync, FALSE);
	return entry;
}

char SORAAPI remove_entry(struct scheduler_enlist* slist, struct LIST_ENTRY_EX* entry) {

	KeWaitForMutexObject(&slist->m_sync, Executive, KernelMode, FALSE, NULL);
	if (IsListEmpty(&slist->m_head.m_entry)) {
		KeReleaseMutex(&slist->m_sync, FALSE);
		return false;
	}
	RemoveEntryList(&entry->m_entry);
	KeReleaseMutex(&slist->m_sync, FALSE);
	return true;
}

char SORAAPI remove_head(struct scheduler_enlist* slist, void** data) {

	KeWaitForMutexObject(&slist->m_sync, Executive, KernelMode, FALSE, NULL);
	if (IsListEmpty(&slist->m_head.m_entry)) {
		KeReleaseMutex(&slist->m_sync, FALSE);
		return false;
	}
	struct LIST_ENTRY_EX* entry;
	entry = (LIST_ENTRY_EX*)RemoveHeadList(&slist->m_head.m_entry);
	*data = entry->m_value;
	ExFreePoolWithTag(entry, tag);
	KeReleaseMutex(&slist->m_sync, FALSE);
	return true;
}

struct scheduler_switch* SORAAPI allocate_scheduler_switch() {

	struct scheduler_switch* ss;
	ss = (struct scheduler_switch*)ExAllocatePoolWithTag(PagedPool, sizeof(struct scheduler_switch), tag);
	ss->m_ref = 1;
	ss->m_affinity = 0;
	ss->m_caller_sleep = 0;
	ss->m_callee_ready = 0;
	return ss;
}

long SORAAPI addref_scheduler_switch(struct scheduler_switch* ss) {

	return InterlockedIncrement(&ss->m_ref); 
}

long SORAAPI delref_scheduler_switch(struct scheduler_switch* ss) {

	long ref;
	ref = InterlockedDecrement(&ss->m_ref);
	switch(ref) {
	case 0:
		ExFreePoolWithTag(ss, tag);
		break;
	default:
		break;
	}
	return ref;	
}

void SORAAPI set_caller_sleep(struct scheduler_switch* ss) {

	ss->m_caller_sleep = 1;
}

void SORAAPI set_callee_ready(struct scheduler_switch* ss) {

	ss->m_callee_ready = 1;
}

char SORAAPI get_caller_sleep(struct scheduler_switch* ss) {

	return ss->m_caller_sleep;
}

char SORAAPI get_callee_ready(struct scheduler_switch* ss) {

	return ss->m_callee_ready;
}

BOOL SORAAPI initialize_sora_affinity(HANDLE sm,
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

struct scheduler_dispatcher* SORAAPI allocate_scheduler_dispatcher() {

	struct scheduler_dispatcher* sd;
	sd = (struct scheduler_dispatcher*)ExAllocatePoolWithTag(PagedPool, 
		sizeof(struct scheduler_dispatcher), 
		tag);
	
	RtlZeroMemory(sd, 
		sizeof(struct scheduler_dispatcher));

	scheduler_share_initialization ssi = { 0 };

	ssi.m_affinity = KeQueryActiveProcessors() & ~1;
	unsigned long i;
	for(i=1; i < sizeof(KAFFINITY)<<3; i++)
		if (ssi.m_affinity & ((KAFFINITY)1)<<i)
			ssi.m_max_concurrent_running++;
	if (ssi.m_max_concurrent_running)
		ssi.m_max_concurrent_running--;

	if (ssi.m_max_concurrent_running) {
		ssi.m_content_size = sizeof(struct scheduler_share_affinity_content) + ssi.m_max_concurrent_running * sizeof(scheduler_share_affinity_register);
		
		sd->m_sharemem = AllocateShareMem(NULL,
			NULL,
			PAGE_READWRITE, 
			sizeof(struct scheduler_share_affinity_manager) + 2 * ssi.m_content_size,
			L"\\BaseNamedObjects\\SoraAffinity",
			SECTION_MAP_READ|SECTION_MAP_WRITE,
			initialize_sora_affinity,
			&ssi);
		
		if (sd->m_sharemem) {
			sd->m_ssa_manager = (struct scheduler_share_affinity_manager*)GetShareMemAddress(sd->m_sharemem);
			sd->m_ssa_content = get_content(sd->m_ssa_manager);			
			if (InitializeShareMemNamedSpinlock(sd->m_sharemem))
				return sd;
		}
	}
	free_scheduler_dispatcher(sd);
	return NULL;
}

void SORAAPI free_scheduler_dispatcher(struct scheduler_dispatcher* sd) {

	if (sd->m_sharemem) {
		FreeShareMem(sd->m_sharemem);
		sd->m_sharemem = NULL;
	}
	ExFreePoolWithTag(sd, 
		tag);
	sd = NULL;
}

long SORAAPI register_scheduler_no_latency(struct scheduler_dispatcher* sd, struct scheduler_no_latency* snl, char lock) {

	acquire_share_mem_lock(sd->m_sharemem,
		lock);

	restore_rollback(sd->m_ssa_manager);

	if (sd->m_ssa_content->m_concurrent_running + snl->m_concurrent_running > sd->m_ssa_manager->m_max_concurrent_running) {
		unsigned long i;
		for(i=0; i < sd->m_ssa_manager->m_max_concurrent_running; i++)
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
		if (sd->m_ssa_content->m_register[i].m_process_id.m_handle)
			continue;

		prepare_rollback(sd->m_ssa_manager);

		RtlZeroMemory(&sd->m_ssa_content->m_register[i], 
			sizeof(struct scheduler_share_affinity_register));
		
		sd->m_ssa_content->m_register[i].m_concurrent_running = snl->m_concurrent_running;
		sd->m_ssa_content->m_register[i].m_process_id.m_handle = PsGetCurrentProcessId();

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

char SORAAPI on_status_wait_0(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	safe_free_scheduler_switch(st->m_switch);

	if (remove_head(st->m_enlist, (void**)&st->m_switch)) {
		st->m_affinity = st->m_switch->m_affinity;
		if (st->m_affinity)
			KeSetSystemAffinityThread(st->m_affinity);
		if (st->m_priority)
			KeSetPriorityThread((PKTHREAD)st->m_thread_object, 
				st->m_priority);			
		set_callee_ready(st->m_switch);
		while(1) {
			if (get_caller_sleep(st->m_switch))
				break;			
			if (st->m_scheduler->m_stop)
				break;
		}
		KeQueryTickCount(&st->m_last_switch_timestamp);
		
		delref_scheduler_switch(st->m_switch);
		st->m_switch = NULL;		
	}
	else {
		st->m_affinity = acquire_affinity_k(st);
		if (st->m_affinity)
			KeSetSystemAffinityThread(st->m_affinity);
		if (st->m_priority)
			KeSetPriorityThread((PKTHREAD)st->m_thread_object, 
				st->m_priority);
	}
	st->m_doproc = do_scheduler_wakeup_callee;
	return true;
}

char SORAAPI on_status_wait_1(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	return false;
}

char SORAAPI on_default(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	return false;
}

void SORAAPI do_scheduler_process(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	while(1) {
		if (st->m_doproc(args)) {
			if (st->m_scheduler->m_stop) 
				break;
			struct scheduler_user_routine* sur;
			sur = (struct scheduler_user_routine*)InterlockedPopEntrySList(&st->m_scheduler->m_garbage_user_routine);
			if (sur)
				ExFreePoolWithTag(sur, tag);
			if (st->m_scheduler->m_scheduing)
				if (st->m_scheduler->m_scheduing(st->m_scheduler))
					continue;
			st->m_scheduler->m_stop = 1;
			KeSetEvent(&st->m_scheduler->m_event_control, IO_NO_INCREMENT, FALSE);
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

	PsTerminateSystemThread(STATUS_SUCCESS);
}

char SORAAPI do_scheduler_wakeup_callee(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	safe_free_scheduler_switch(st->m_switch);

	KAFFINITY affinity;
	affinity = acquire_affinity_k(st);
	if (affinity) {
		st->m_switch = allocate_scheduler_switch();
		st->m_switch->m_affinity = affinity;
		addref_scheduler_switch(st->m_switch);
		insert_tail(st->m_enlist, st->m_switch);

		KeReleaseSemaphore(&st->m_scheduler->m_semaphore_control, IO_NO_INCREMENT, 1, FALSE);

		st->m_doproc = do_scheduler_check_callee;
	}
	return true;
}

char SORAAPI do_scheduler_check_callee(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	if (st->m_switch)
		if (get_callee_ready(st->m_switch)) {			
			set_caller_sleep(st->m_switch);			
			delref_scheduler_switch(st->m_switch);
			st->m_switch = NULL;			
			return do_scheduler_wait_caller(args);
		}
	return true;
}

char SORAAPI do_scheduler_wait_caller(void* args) {

	struct scheduler_thread* st;
	st = (struct scheduler_thread*)args;

	release_affinity(st->m_scheduler, st->m_affinity);
	st->m_affinity = 0;

	KeSetSystemAffinityThread(KeQueryActiveProcessors() & ~1);
	KeSetPriorityThread((PKTHREAD)st->m_thread_object, 
		LOW_REALTIME_PRIORITY);

	void* object[2];
	object[0] = &st->m_scheduler->m_semaphore_control;
	object[1] = &st->m_scheduler->m_event_control;
	switch(KeWaitForMultipleObjects(2,
									object,
									WaitAny,
									Executive,
									KernelMode,
									FALSE,
									NULL, 
									NULL)) {
	case STATUS_WAIT_0:
		return on_status_wait_0(args);
		break;
	case STATUS_WAIT_1:
		return on_status_wait_1(args);
		break;
	default:
		return on_default(args);
		break;
	}
}

char SORAAPI initialize_scheduler_no_latency(struct scheduler_no_latency* snl, unsigned long concurrent_running, unsigned long total_runnable, PSORA_UTHREAD_PROC* proc, void** args, unsigned long count, SCHEDULING_TYPE type) {

	uninitialize_scheduler_no_latency(snl);
	snl->m_dispatcher = allocate_scheduler_dispatcher();
	snl->m_concurrent_running = concurrent_running;
	snl->m_total_runnable = total_runnable;
	snl->m_register_entry = -1;
	snl->m_user_routine = allocate_scheduler_user_routine(count);
	snl->m_user_routine->m_count = count;
	{
		unsigned long i;
		for(i=0; i < snl->m_user_routine->m_count; i++) {
			snl->m_user_routine->m_routine[i].m_user_proc = proc[i];
			snl->m_user_routine->m_routine[i].m_user_args = args[i];
			snl->m_user_routine->m_routine[i].m_user_return = TRUE;
		}
		switch(type) {
		case ROUND_ROBIN:
			snl->m_scheduing = Scheduling_RoundRobin;
			snl->m_scheduing_info.index = -1;
			break;
		default:
			snl->m_scheduing = NULL;
			snl->m_scheduing_info.index = -1;
			break;				
		}
	}
	if (snl->m_user_routine &&
		snl->m_scheduing) {
		KeInitializeSemaphore(&snl->m_semaphore_control, snl->m_concurrent_running, snl->m_total_runnable);
		KeInitializeEvent(&snl->m_event_control, NotificationEvent, FALSE);
		initialize_scheduler_enlist(&snl->m_enlist);
		unsigned long size;
		size = snl->m_total_runnable * sizeof(struct scheduler_thread);
		snl->m_thread_pool = (struct scheduler_thread*)ExAllocatePoolWithTag(PagedPool, size, tag);
		RtlZeroMemory(snl->m_thread_pool, size);
		unsigned long i;
		for(i=0; i < snl->m_total_runnable; i++) {
			scheduler_thread* st;
			st = snl->m_thread_pool + i;
			st->m_scheduler = snl;
			st->m_enlist = &snl->m_enlist;
			st->m_switch = NULL;
			st->m_doproc = do_scheduler_wait_caller;
			st->m_thread_object = NULL;
			st->m_priority = 0;
			st->m_affinity = 0;
			st->m_last_switch_timestamp.QuadPart = 0;
			st->m_fail_count = 0;
			st->m_validate_entry = 0;
		}
		snl->m_thread_init_dispatcher = NULL;
		snl->m_state |= state_initialized;	
		return true;
	}
	uninitialize_scheduler_no_latency(snl);
	return false;
}

void SORAAPI uninitialize_scheduler_no_latency(struct scheduler_no_latency* snl) {

	if (snl->m_state & state_initialized) {
		if (snl->m_dispatcher) {
			if (snl->m_dispatcher->m_sharemem) {
				FreeShareMem(snl->m_dispatcher->m_sharemem);
				snl->m_dispatcher->m_sharemem = NULL;
			}
			free_scheduler_dispatcher(snl->m_dispatcher);
			snl->m_dispatcher = NULL;
		}
		snl->m_concurrent_running = 0;
		snl->m_total_runnable = 0;
		if (snl->m_user_routine) {
			ExFreePoolWithTag(snl->m_user_routine, tag);
			snl->m_user_routine = NULL;
		}
		uninitialize_scheduler_enlist(&snl->m_enlist);
		if (snl->m_thread_pool) {
			ExFreePoolWithTag(snl->m_thread_pool, tag);
			snl->m_thread_pool = NULL;
		}
		snl->m_scheduing = NULL;
		snl->m_scheduing_info.index = -1;
	}
}

char SORAAPI start(struct scheduler_no_latency* snl, unsigned long access, POBJECT_ATTRIBUTES attr, char wait_init) {

	stop(snl, wait_init);
	if (snl->m_thread_pool) {
		snl->m_register_entry = register_scheduler_no_latency(snl->m_dispatcher, snl, 1);
		if (snl->m_register_entry != -1) {
			KeResetEvent(&snl->m_event_control);
			snl->m_stop = 0;
			unsigned long i;
			for(i=0; i < snl->m_total_runnable; i++) {
				NTSTATUS status;
				HANDLE handle;
				struct scheduler_thread* st;
				st = snl->m_thread_pool + i;
				status = PsCreateSystemThread(&handle, access, attr, NULL, NULL, do_scheduler_process, st);	
				if (status != STATUS_SUCCESS) {
					stop(snl, wait_init);
					return false;
				}
				status = ObReferenceObjectByHandle(handle, 
												   THREAD_ALL_ACCESS, 
												   NULL, 
												   KernelMode, 
												   &st->m_thread_object,
												   NULL);				
				KeSetPriorityThread((PKTHREAD)st->m_thread_object, 
					st->m_priority);
				ZwClose(handle);
				if (status != STATUS_SUCCESS) {
					stop(snl, wait_init);
					return false;
				}
			}
			return true;
		}
	}
	stop(snl, wait_init);
	return false;
}

void SORAAPI stop(struct scheduler_no_latency* snl, char wait_init) {

	if (snl->m_state & state_initialized) {
		snl->m_stop = 1;
		KeSetEvent(&snl->m_event_control, IO_NO_INCREMENT, FALSE);		
		if (snl->m_thread_pool) {
			if (snl->m_register_entry != -1) {
				unregister_scheduler_no_latency(snl->m_dispatcher, snl->m_register_entry, 1);
				snl->m_register_entry = -1;
			}
			unsigned long i;
			for(i=0; i < snl->m_total_runnable; i++) {
				scheduler_thread* st;
				st = snl->m_thread_pool + i;
				if (st->m_thread_object) {
					KeWaitForSingleObject(st->m_thread_object,
					                      Executive,
										  KernelMode,
										  FALSE,
										  NULL);
					ObDereferenceObject(st->m_thread_object);
					st->m_thread_object = NULL;
				}
			}
		}
		if (wait_init)
			if (snl->m_thread_init_dispatcher) {
				KeWaitForSingleObject(snl->m_thread_init_dispatcher,
									  Executive,
									  KernelMode,
									  FALSE,
									  NULL);
				ObDereferenceObject(snl->m_thread_init_dispatcher);
				snl->m_thread_init_dispatcher = NULL;
			}
	}
}

void SORAAPI set_priority(struct scheduler_no_latency* snl, unsigned long i, KPRIORITY priority) {

	if (i < snl->m_total_runnable)
		if (snl->m_thread_pool)
		   (snl->m_thread_pool+i)->m_priority = priority;
}

void SORAAPI set_affinity(struct scheduler_no_latency* snl, unsigned long i, KAFFINITY affinity) {

	if (i < snl->m_total_runnable)
		if (snl->m_thread_pool)
		   (snl->m_thread_pool+i)->m_affinity = affinity;
}

KAFFINITY SORAAPI acquire_affinity_k(struct scheduler_thread* st) {

	KAFFINITY affinity;
	affinity = 0;
	LARGE_INTEGER timestamp;
	KeQueryTickCount(&timestamp);
	if ((timestamp.QuadPart-st->m_last_switch_timestamp.QuadPart) >= (LONGLONG)schedule_switch_time*1000*10/KeQueryTimeIncrement() ||
		(st->m_last_switch_timestamp.QuadPart-timestamp.QuadPart) >= (LONGLONG)schedule_switch_time*1000*10/KeQueryTimeIncrement()) {
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

HANDLE SORAAPI SoraThreadAlloc() {
	
	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)ExAllocatePoolWithTag(NonPagedPool,
		sizeof(struct scheduler_no_latency),
		tag);
	RtlZeroMemory(snl, sizeof(struct scheduler_no_latency));
	KeInitializeMutex(&snl->m_lock, 0);
	ExInitializeSListHead(&snl->m_garbage_user_routine);
	return snl;
}

VOID SORAAPI SoraThreadFree(HANDLE Thread) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread;
	AutoMutex am(&snl->m_lock);
	SoraThreadStop(snl);
	am.Unlock();
	am.Detach();
	ExFreePoolWithTag(snl, tag);
}

extern "C"
NTSTATUS
ZwOpenDirectoryObject(
	__out PHANDLE DirectoryHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes
	);

BOOLEAN is_named_object_dir_valid() {

	UNICODE_STRING us;
	RtlInitUnicodeString(&us,
		L"\\BaseNamedObjects");
	
	OBJECT_ATTRIBUTES objattr;
	InitializeObjectAttributes(&objattr,
		&us,
		OBJ_OPENIF,
		NULL,
		NULL);
	
	HANDLE handle;
	handle = NULL;
	
	ZwOpenDirectoryObject(&handle,
		DIRECTORY_QUERY,
		&objattr);
	if (handle) {
		ZwClose(handle);
		return TRUE;
	}
	else
		return FALSE;
}

void do_init_dispatcher(void* context) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)context;
	LARGE_INTEGER li;
	li.QuadPart = (LONGLONG)-1*10*1000*1000;
	while(KeWaitForSingleObject(&snl->m_event_control,
		Executive,
		KernelMode,
		FALSE,
		&li) != STATUS_SUCCESS) {
		if (!snl->m_dispatcher)
			snl->m_dispatcher = allocate_scheduler_dispatcher();
			
		if ( snl->m_dispatcher)
			if (start(snl, SYNCHRONIZE|GENERIC_ALL|STANDARD_RIGHTS_ALL, NULL, 0))
				break;
	}
	PsTerminateSystemThread(STATUS_SUCCESS);
}

BOOLEAN SORAAPI SoraThreadStart(HANDLE Thread, PSORA_UTHREAD_PROC User_Routine, PVOID User_Context) {

	return SoraThreadStartEx(Thread, &User_Routine, &User_Context, 1, ROUND_ROBIN);
}

BOOLEAN SoraThreadStartEx(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, PVOID* User_Context, ULONG Count, SCHEDULING_TYPE Type) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread;
	AutoMutex am(&snl->m_lock);
	
	SoraThreadStop(snl);
	if (initialize_scheduler_no_latency(snl, 1, 2, User_Routine, User_Context, Count, Type)) {
		set_priority(snl, 0, HIGH_PRIORITY);
		set_priority(snl, 1, HIGH_PRIORITY);
		if (snl->m_dispatcher) {
			if (start(snl, SYNCHRONIZE|GENERIC_ALL|STANDARD_RIGHTS_ALL, NULL, 1))
				return TRUE;
		}
		else
		if (is_named_object_dir_valid())
			return FALSE;
		else {
			NTSTATUS status;
			HANDLE handle;
			status = PsCreateSystemThread(&handle, 
				SYNCHRONIZE|GENERIC_ALL|STANDARD_RIGHTS_ALL, 
				NULL, 
				NULL, 
				NULL, 
				do_init_dispatcher, 
				snl);
			
			if (status != STATUS_SUCCESS)
				return FALSE;
			
			status = ObReferenceObjectByHandle(handle, 
											   THREAD_ALL_ACCESS, 
											   NULL, 
											   KernelMode, 
											   &snl->m_thread_init_dispatcher,
											   NULL);
			ZwClose(handle);

			DbgPrint("SoraThreadStart Pending...\n");

			return TRUE;
		}
	}
	return FALSE;
}


VOID SORAAPI SoraThreadStop(HANDLE Thread) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread;
	AutoMutex am(&snl->m_lock);
	
	stop(snl, 1);
	uninitialize_scheduler_no_latency(snl);
}

BOOLEAN SoraThreadJoin(HANDLE Thread, ULONG Timeout) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)Thread;
	void** object;
	object = NULL;
	unsigned long count;
	count = 0;
	PKWAIT_BLOCK waitblk;
	waitblk = NULL;	
	{
		AutoMutex am(&snl->m_lock);
		if (snl->m_total_runnable) {
			object = (void**)ExAllocatePoolWithTag(
				PagedPool, 
				snl->m_total_runnable * sizeof(void*),
				'    ');
			unsigned long i;
			for(i=0; i < snl->m_total_runnable; i++) {
				scheduler_thread* st;
				st = snl->m_thread_pool + i;
				if (st->m_thread_object) {
					ObReferenceObject(st->m_thread_object);
					object[count] = st->m_thread_object;
					count++;					
				}
			}
		}
	}
	if (object &&
		count) {
		if (count > THREAD_WAIT_OBJECTS)
			waitblk = (PKWAIT_BLOCK)ExAllocatePoolWithTag(
				NonPagedPool,
				count*sizeof(KWAIT_BLOCK),
				'    ');
		LARGE_INTEGER l;
		l.QuadPart = -(LONGLONG)Timeout*10*1000;
		NTSTATUS status;
		status = KeWaitForMultipleObjects(
			count, 
			object, 
			WaitAll,
			Executive,
			KernelMode,
			FALSE,
			Timeout != INFINITE ? &l : NULL,
			waitblk);
		unsigned long i;
		for(i=0; i < count; i++)
			if (object[count])
				ObDereferenceObject(object[count]);
		ExFreePoolWithTag(
			object, 
			'    ');
		if (status != STATUS_SUCCESS)
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
				new_sur->m_count++;
			}
		for(i=0; i<Count; i++) {
			new_sur->m_routine[new_sur->m_count].m_user_proc = User_Routine[i];
			new_sur->m_routine[new_sur->m_count].m_user_args = User_Context[i];
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
				if (sur->m_routine[i].m_user_proc != User_Routine[j])
					continue;
				match = true;
				break;
			}
			if (match)
				continue;
			if (sur->m_routine[i].m_user_return) {
				new_sur->m_routine[new_sur->m_count] = sur->m_routine[i];
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


