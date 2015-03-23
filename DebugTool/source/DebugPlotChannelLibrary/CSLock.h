#pragma once

#include <Windows.h>

namespace SoraDbgPlot { namespace Lock {

	class CSLock
	{
	public:
		CSLock() {
			
			::InitializeCriticalSection(&_cs);
		}
		~CSLock()
		{
			::DeleteCriticalSection(&_cs);
		}
		void Lock()
		{
			::EnterCriticalSection(&_cs);
		}

		void Unlock()
		{
			::LeaveCriticalSection(&_cs);
		}
	private:
		CRITICAL_SECTION _cs;
	};

}}
