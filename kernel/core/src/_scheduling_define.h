#pragma once

#include "_scheduler.h"

inline char Scheduling_RoundRobin(void* args) {

	struct scheduler_no_latency* snl;
	snl = (struct scheduler_no_latency*)args;

	struct scheduler_user_routine* sur;
	sur = snl->m_user_routine;

	unsigned long i;
	for(i=0; i<sur->m_count; i++) {
		snl->m_scheduing_info.index++;
		if (snl->m_scheduing_info.index >= sur->m_count)
			snl->m_scheduing_info.index  = 0;
		if (sur->m_routine[snl->m_scheduing_info.index].m_user_return) {
			sur->m_routine[snl->m_scheduing_info.index].m_user_return = sur->m_routine[snl->m_scheduing_info.index].m_user_proc(sur->m_routine[snl->m_scheduing_info.index].m_user_args);
			return TRUE;
		}
	}
	return FALSE;
}
