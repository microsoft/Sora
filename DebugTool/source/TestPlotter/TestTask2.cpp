#include "TaskQueue.h"

using namespace SoraDbgPlot::Task;

#if 0

int TestTask2()
{
	TaskQueue queue;

	for (int i = 0; i < 10; i++)
	{
		int len = 20;
		int * ptr = new int[len];

		for (int j = 0; j < len; j++)
		{
			ptr[j] = 100*i + j;
		}

		std::function<void(void)> f = [ptr, len](bool exit){

			 if (exit)
				 delete [] ptr;
			 else
			 {
				 for (int i = 0; i < len; i++)
				 {
					 printf("%d\n", ptr[i]);
				 }

				 delete [] ptr;
			 }

		};

		queue.QueueTask(f);
	}

	return 0;
}

#endif