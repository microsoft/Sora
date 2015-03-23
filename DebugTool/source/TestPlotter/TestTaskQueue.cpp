#include "WaitableLatestTaskQueue.h"

using namespace SoraDbgPlot::Task;

int TestTaskQueue()
{
	WaitableLatestTaskQueue queue;
	
	

	for (int i = 0; i < 100; i++)
	{
		queue.QueueTask(std::function<void (void)>([i](){
			printf("%d\n", i);
		}));

		::Sleep(15);
	}

	return 0;
}
