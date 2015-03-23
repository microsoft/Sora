#pragma once

#include <Windows.h>

struct SM_SpinLock
{
	int gcCount;
	volatile long pid;
};

inline void Init(SM_SpinLock * lock, int gcCount)
{
	lock->pid = 0;
	lock->gcCount = gcCount;
}


inline void GC(SM_SpinLock * lock)
{
	HANDLE processHandle = ::OpenProcess(SYNCHRONIZE, FALSE, lock->pid);
	if (processHandle == NULL)
	{
		long compare = lock->pid;
		::InterlockedCompareExchange(&lock->pid, 0, compare);
	}
	else
		::CloseHandle(processHandle);
}

inline void Lock(SM_SpinLock * lock)
{
	long pid = ::GetCurrentProcessId();
	int gcCount = 0;
	while(1)
	{
		long old = ::InterlockedCompareExchange(&lock->pid, pid, 0);
		if (old == 0)	// succeed
			break;
		else
		{
			++gcCount;
		}

		if (gcCount == lock->gcCount) // spin too much, check if the lock holder is dead
		{
			GC(lock);
			gcCount = 0;
		}
	}
}


inline void Unlock(SM_SpinLock * lock)
{
	lock->pid = 0;
}

