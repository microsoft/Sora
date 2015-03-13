#pragma once

#ifdef USER_MODE
#include "sora.h"
#else
#include <wdm.h>
#include "const.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _LIST_ENTRY_EX_
#define _LIST_ENTRY_EX_

struct LIST_ENTRY_EX {

	LIST_ENTRY m_entry;
	void* m_value;
};

#endif

struct thread_safe_enlist {

	NPAGED_LOOKASIDE_LIST m_lookaside;
	struct LIST_ENTRY_EX m_head;
	KSPIN_LOCK m_sync;
	LONG m_count;
};

struct thread_safe_enlist* 
SORAAPI 
allocate_thread_safe_enlist(
	);

void
SORAAPI 
release_thread_safe_enlist(
	struct thread_safe_enlist* tslist);

struct LIST_ENTRY_EX* 
SORAAPI 
insert_thread_safe_enlist_tail(
	struct thread_safe_enlist* tslist, 
	void*  data,
	char lock);

char 
SORAAPI 
remove_thread_safe_enlist_entry(
	struct thread_safe_enlist* tslist, 
	struct LIST_ENTRY_EX* entry,
	char lock);

char 
SORAAPI 
remove_thread_safe_enlist_head(
	struct thread_safe_enlist* tslist, 
	void** data,
	char lock);

LONG
SORAAPI
get_thread_safe_enlist_count(
	struct thread_safe_enlist* tslist);

#ifdef __cplusplus
}
#endif
