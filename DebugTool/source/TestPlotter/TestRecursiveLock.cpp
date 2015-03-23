#include <assert.h>
#include <process.h>
#include "CSRecursiveLock.h"

using SoraDbgPlot::Lock::CSRecursiveLock;

struct TParam
{
	int Target;
	HANDLE Event;
	CSRecursiveLock lock;
};

static unsigned int __stdcall ThreadTest(PVOID param)
{
	auto p = (TParam *)param;

	::WaitForSingleObject(p->Event, INFINITE);
	srand(::GetCurrentThreadId());
	int count = rand() % 32 + 1;
	for (int i = 0; i < count; i++)
		p->lock.Lock();

	p->Target++;

	for (int i = 0; i < count; i++)
		p->lock.Unlock();

	//_endthreadex(0);

	return 0;
}


int TestRecursiveLock()
{
	const int MAX_COUNT = 64;
	const int ITERATION = 64;
	HANDLE hThread[MAX_COUNT];

	TParam tParam;
	tParam.Target = 0;
	tParam.Event = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	for (int count = 0; count < ITERATION; count++)
	{

		::ResetEvent(tParam.Event);

		for (int i = 0; i < MAX_COUNT; i++)
		{
			hThread[i] = (HANDLE) _beginthreadex(NULL, 0, ThreadTest, &tParam, 0, 0);
			if (hThread[i] == 0)
				abort();
		}

		::SetEvent(tParam.Event);

		DWORD res = ::WaitForMultipleObjects(MAX_COUNT, hThread, TRUE, INFINITE);

		for (int i = 0; i < MAX_COUNT; i++)
		{
			CloseHandle(hThread[i]);
		}


	}

	::CloseHandle(tParam.Event);

	assert(tParam.Target == MAX_COUNT * ITERATION);

	return 0;
}

