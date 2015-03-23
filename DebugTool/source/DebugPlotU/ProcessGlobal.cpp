#include "SharedNameManagement.h"
#include "AppSettings.h"
#include "SharedStruct.h"
#include "ProcessGlobal.h"
#include "SmProcess.h"

ProcessGlobal * ProcessGlobal::_instance = 0;
SpinLock ProcessGlobal::_lock;

ProcessGlobal * ProcessGlobal::Instance()
{
	if (_instance == 0)
	{
		_lock.Lock();
		if (_instance == 0)
		{
			_instance = new ProcessGlobal();
		}
		_lock.Unlock();
	}

	return _instance;
}

void ProcessGlobal::Release()
{
	_lock.Lock();
	if (_instance)
	{
		delete _instance;
		_instance = 0;
	}
	_lock.Unlock();
}

ProcessGlobal::ProcessGlobal()
{
	//
	_smProcess = new SmProcess(::GetCurrentProcessId(), SettingGetSourceBufferSize());

}

ProcessGlobal::~ProcessGlobal()
{
	delete _smProcess;
}

SmProcess * ProcessGlobal::GetProcess()
{
	return _smProcess;
}
