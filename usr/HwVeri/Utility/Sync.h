#pragma once

#include <Windows.h>

class WaitObj
{
public:
	WaitObj();
	~WaitObj();

	void Enable();
	void Disable();
	void Wait();
	void Signal();

private:
	HANDLE hEvent;
	bool enabled;
};
