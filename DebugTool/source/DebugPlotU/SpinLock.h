#pragma once

#include <Windows.h>

class SpinLock
{
public:
	SpinLock() {
		value = 0;
	}
	void Lock() {
		while(::InterlockedCompareExchange(&value, 1, 0) != 0);
	}
	void Unlock() {
		InterlockedExchange(&value, 0);
	}
private:
	volatile LONG value;
};
