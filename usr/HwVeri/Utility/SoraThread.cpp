#include "SoraThread.h"

SoraThread::SoraThread()
{
	thread = 0;
	threadID = 0;	
	flagStop = false;
	threadCallback = 0;
	state = STOPPED;
}

void SoraThread::SetCallback(IThreadControl * callback)
{
	threadCallback = callback;
}

bool SoraThread::CheckStatus()
{
	syncSuspendObj.Wait();

	if (flagStop)
	{
		flagStop = false;
		return true;
	}


	return false;
}

HRESULT SoraThread::Start(bool syncEnabled)
{
	HRESULT ret = S_OK;

	if (state != STOPPED)
	{
		ret = -1;
		goto EXIT;
	}

	if (syncEnabled)
		syncStartObj.Enable();

	thread = CreateThread(
		NULL,              // default security
		0,                 // default stack size
		ThreadProc,        // name of the thread function
		this,              // no thread parameters
		0,                 // default startup flags
		&threadID);

	syncStartObj.Wait();

EXIT:
	return ret;
}

HRESULT SoraThread::Stop(bool syncEnabled)
{
	HRESULT ret = S_OK;

	if (state == STOPPED)
	{
		ret = -1;
		goto EXIT;
	}

	if (syncEnabled)
		syncEndObj.Enable();

	flagStop = true;
	if (state == SUSPENDED)
		Resume();

	syncEndObj.Wait();
EXIT:
	return ret;
}

HRESULT SoraThread::Suspend()
{
	HRESULT ret = S_OK;

	if (state != RUNNING)
	{
		ret = -1;
		goto EXIT;
	}

	syncSuspendObj.Enable();
	state = SUSPENDED;

EXIT:
	return ret;
}

HRESULT SoraThread::Resume()
{
	HRESULT ret = S_OK;

	if (state != SUSPENDED)
	{
		ret = -1;
		goto EXIT;
	}

	syncSuspendObj.Signal();
	state = RUNNING;

EXIT:
	return ret;
}

DWORD WINAPI SoraThread::ThreadProc(PVOID arglist)
{
	SoraThread * threadContext = (SoraThread *)arglist;

	threadContext->state = RUNNING;

	threadContext->syncStartObj.Signal();

	threadContext->RunThreadStartCallback();
	threadContext->RunThreadFunction();
	threadContext->RunThreadEndCallback();

	threadContext->syncEndObj.Signal();

	threadContext->state = STOPPED;

	return 0;
}

void SoraThread::RunThreadStartCallback()
{
	if (threadCallback != 0)
	{
		threadCallback->ThreadStartCallback();
	}
}

void SoraThread::RunThreadEndCallback()
{
	if (threadCallback != 0)
	{
		threadCallback->ThreadExitCallback();
	}
}

void SoraThread::RunThreadFunction()
{
	Threadfunc();
}

SoraThread::STATE SoraThread::GetState()
{
	return state;
}
