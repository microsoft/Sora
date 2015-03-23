#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "sora.h"
//#include "_share_mem_if.h"
#include "SharedStruct.h"
#include "AppSettings.h"
#include "ShareMemHelper.h"
#include "HashTable.h"

/************************************

Shared Source Info

*************************************/

BOOL SharedSourceInfo::Init(ShareMem * sm, void* context)
{
	//TODO
	SharedSourceInfo * sourceInfo = (SharedSourceInfo *)sm->GetAddress();
	sourceInfo->rIdx = 0;
	sourceInfo->wIdx = 0;
	sourceInfo->bufLen = ::SettingGetSourceBufferSize();
	sourceInfo->singleStepFlag = false;

	return TRUE;
}
