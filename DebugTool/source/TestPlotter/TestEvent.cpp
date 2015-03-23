#include <Windows.h>
#include <assert.h>
#include "Event.h"

using namespace SoraDbgPlot::Event;

struct MouseEvent
{
	int x;
	int y;
};

int TestEvent()
{
	Event<MouseEvent> aEvent;

	HANDLE h1 = aEvent.Subscribe([](const void * sender, const MouseEvent & e){
		printf("h1: %d, %d\n", e.x, e.y);
	});

	HANDLE h2 = aEvent.Subscribe([](const void * sender, const MouseEvent & e){
		printf("h2: %d, %d\n", e.x, e.y);
	});

	MouseEvent e;
	e.x = 2; e.y = 3;

	aEvent.Raise((void *)0, e);

	bool succ = aEvent.UnSubscribe(h1);
	assert(succ);

	aEvent.Raise((void *)1, e);

	succ = aEvent.UnSubscribe(h2);
	assert(succ);

	aEvent.Raise((void *)2, e);

	return 0;
}
