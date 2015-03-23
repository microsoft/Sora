#include "Sync.h"

WaitObj::WaitObj()
{
	hEvent = 0;
	enabled = 0;
}

WaitObj::~WaitObj()
{
	if (hEvent)
	{
		CloseHandle(hEvent);
	}
}

void WaitObj::Enable()
{
	enabled = true;

	if (hEvent == 0)
	{
		hEvent = CreateEvent( 
			NULL,               // default security attributes
			TRUE,               // manual-reset event
			FALSE,              // initial state is nonsignaled
			TEXT("SyncEvent")  // object name
			);
	}
}

void WaitObj::Disable()
{
	enabled = false;
}

void WaitObj::Wait()
{
	if (!enabled)
		return;

	if (hEvent)
	{
		DWORD dwWaitResult = WaitForSingleObject( 
			hEvent, // event handle
			INFINITE);    // indefinite wait	

		CloseHandle(hEvent);
		hEvent = 0;
	}
}

void WaitObj::Signal()
{
	if (!enabled)
		return;

	SetEvent(hEvent);
}
