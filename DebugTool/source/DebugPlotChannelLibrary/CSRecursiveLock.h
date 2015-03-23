#pragma once

#include <Windows.h>
#include "CSLock.h"

namespace SoraDbgPlot { namespace Lock {
	class CSRecursiveLock
	{
	public:
		CSRecursiveLock()
		{
			_tid = 0;
			_recursiveCount = 0;
		}
		~CSRecursiveLock()
		{
		}

		void Lock()
		{
			DWORD tid = ::GetCurrentThreadId();
			
			if (tid != _tid)
			{
				_lock.Lock();
				_tid = tid;
			}
			
			::InterlockedIncrement(&_recursiveCount);
		}

		void Unlock()
		{
			if (::InterlockedDecrement(&_recursiveCount) == 0)
			{
				_tid = 0;
				_lock.Unlock();
			}
		}

	private:
		CSLock _lock;
		volatile DWORD _tid;
		volatile unsigned long _recursiveCount;
	};
}};
