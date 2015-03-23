#pragma once

#include <Windows.h>
#include "Sync.h"

class IThreadControl
{
public:
	virtual void ThreadStartCallback() = 0;
	virtual void ThreadExitCallback() = 0;
};

class SoraThread
{
public:
	SoraThread();

	HRESULT Start(bool sync);
	HRESULT Stop(bool sync);
	HRESULT Suspend();
	HRESULT Resume();

	void SetCallback(IThreadControl * callback);

	virtual void Threadfunc() = 0;
	bool CheckStatus();

	enum STATE {
		RUNNING,
		STOPPED,
		SUSPENDED,
	} state;

	enum STATE GetState();

private:
	static DWORD WINAPI ThreadProc(PVOID);
	HANDLE thread;
	DWORD threadID;

	WaitObj syncStartObj;
	WaitObj syncEndObj;
	WaitObj syncSuspendObj;

	bool flagStop;

	IThreadControl * threadCallback;

	void RunThreadStartCallback();
	void RunThreadEndCallback();
	void RunThreadFunction();

};

