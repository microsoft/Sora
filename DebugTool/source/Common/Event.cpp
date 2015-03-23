#include "Event.h"

static volatile long __eventHandlerCount = 0;

using namespace SoraDbgPlot::Event;

#ifdef _DEBUG

int __stdcall SoraDbgPlot::Event::EventHandlerCount()
{
	return __eventHandlerCount;
}

void __stdcall SoraDbgPlot::Event::EventHandlerCountModify(int count, int sign)
{
	if (count == 1)
	{
		if (sign)
			::InterlockedIncrement(&__eventHandlerCount);
		else
			::InterlockedDecrement(&__eventHandlerCount);
	}
	else
	{
		if (sign == 0)
			count = -count;

		while(1)
		{
			int ori = __eventHandlerCount;
			int target = __eventHandlerCount + count;
			if (::InterlockedCompareExchange(&__eventHandlerCount, target, ori) == ori)
				break;
		}
	}
}

#endif
