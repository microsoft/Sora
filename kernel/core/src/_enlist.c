#include "_enlist.h"

#define	Tag	'nest'

#define acquire_spinlock(lock, spinlock, irql) {	\
	if (lock)										\
		KeAcquireSpinLock(&spinlock, 				\
		&irql);										\
}

#define release_spinlock(lock, spinlock, irql) {	\
	if (lock)										\
		KeReleaseSpinLock(&spinlock,				\
		irql);										\
}

struct thread_safe_enlist* 
SORAAPI 
allocate_thread_safe_enlist() {

	struct thread_safe_enlist* tslist;
	tslist = (struct thread_safe_enlist*)ExAllocatePoolWithTag(NonPagedPool,
		sizeof(struct thread_safe_enlist),
		Tag);
	if (tslist) {
		ExInitializeNPagedLookasideList(&tslist->m_lookaside,
			NULL,
			NULL,
			0,
			sizeof(LIST_ENTRY_EX),
			Tag,
			0);
		InitializeListHead(&tslist->m_head.m_entry);
		KeInitializeSpinLock(&tslist->m_sync); 
		tslist->m_head.m_value = NULL;
		tslist->m_count = 0;
	}
	return tslist;
}

void 
SORAAPI 
release_thread_safe_enlist(struct thread_safe_enlist* tslist) {

	if (tslist) {
		ExDeleteNPagedLookasideList(&tslist->m_lookaside);
		ExFreePoolWithTag(tslist,
			Tag);
	}
}

struct LIST_ENTRY_EX* 
SORAAPI 
insert_thread_safe_enlist_tail(struct thread_safe_enlist* tslist, 
	void*  value,
	char lock) {

	KIRQL irql;
	acquire_spinlock(lock, 
		tslist->m_sync,
		irql);
	struct LIST_ENTRY_EX* entry;
	entry = (struct LIST_ENTRY_EX*)ExAllocateFromNPagedLookasideList (&tslist->m_lookaside);
	entry->m_value = value;
	InsertTailList(&tslist->m_head.m_entry, 
		&entry->m_entry);
	InterlockedIncrement(&tslist->m_count);
	release_spinlock(lock,
		tslist->m_sync,
		irql);
	return entry;
}

char 
SORAAPI 
remove_thread_safe_enlist_entry(struct thread_safe_enlist* tslist, 
	struct LIST_ENTRY_EX* entry,
	char lock) {

	KIRQL irql;
	acquire_spinlock(lock,
		tslist->m_sync,
		irql);
	if (IsListEmpty(&tslist->m_head.m_entry)) {		
		release_spinlock(lock, 
			tslist->m_sync,
			irql);
		return false;
	}
	RemoveEntryList(&entry->m_entry);
	InterlockedDecrement(&tslist->m_count);
	release_spinlock(lock, 
		tslist->m_sync,
		irql);	
	return true;
}

char 
SORAAPI 
remove_thread_safe_enlist_head(struct thread_safe_enlist* tslist, 
	void** data,
	char lock) {

	KIRQL irql;
	acquire_spinlock(lock, 
		tslist->m_sync,
		irql);	
	if (IsListEmpty(&tslist->m_head.m_entry)) {
		release_spinlock(lock, 
			tslist->m_sync,
			irql);
		return false;
	}
	struct LIST_ENTRY_EX* entry;
	entry = (LIST_ENTRY_EX*)RemoveHeadList(&tslist->m_head.m_entry);
	*data = entry->m_value;
	ExFreeToNPagedLookasideList(&tslist->m_lookaside,
		entry);
	InterlockedDecrement(&tslist->m_count);
	release_spinlock(lock, 
		tslist->m_sync,
		irql);
	return true;
}


LONG
SORAAPI
get_thread_safe_enlist_count(
	struct thread_safe_enlist* tslist) {

	return tslist->m_count;
}
