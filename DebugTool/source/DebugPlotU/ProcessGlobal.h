#pragma once

#include "_share_lock_if.h"
#include "_share_mem_if.h"
#include "SpinLock.h"
#include "SmProcess.h"

class ProcessGlobal
{
public:
	static ProcessGlobal * Instance();
	static void Release();

public:
	SmProcess * GetProcess();

private:
	static ProcessGlobal * _instance;
	static SpinLock _lock;

private:
	ProcessGlobal();
	~ProcessGlobal();
	SmProcess * _smProcess;
};
 