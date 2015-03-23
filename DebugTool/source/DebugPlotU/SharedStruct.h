#pragma once
#include <Windows.h>
#include "_share_mem_if.h"

class SharedSourceInfo
{
public:
	static BOOL Init(ShareMem * sm, void* context);
	int bufLen;
	volatile int rIdx;
	volatile int wIdx;
	volatile bool singleStepFlag;
};

