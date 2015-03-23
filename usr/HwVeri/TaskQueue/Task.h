#pragma once

#include <windows.h>

class Task
{
public:
	virtual void Run() = 0;
	//virtual HRESULT Reset() = 0;
};
