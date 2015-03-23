#pragma once

#include <Windows.h>
#include <assert.h>
//#include "_share_mem_if.h"
#include "ShareMemHelper.h"

namespace SoraDbgPlot { namespace Common
{
	class SharedSerialNumGenerator
	{
	public:
		SharedSerialNumGenerator(const wchar_t * sharedName)
		{
			_sm = AllocateSharedMem(
				sharedName,
				4,
				SharedMemInit,
				0);
		}

		~SharedSerialNumGenerator()
		{
			ShareMem::FreeShareMem(_sm);
		}

		unsigned int GetSerialNum()
		{
			volatile unsigned int * addr = (unsigned int *)_sm->GetAddress();
			return ::InterlockedIncrement(addr);
		}

		static BOOL SharedMemInit(ShareMem* sm, void* context)
		{
			int * addr = (int *)sm->GetAddress();
			*addr = 0;
			return TRUE;
		}

		ShareMem * _sm;
	};
}}
