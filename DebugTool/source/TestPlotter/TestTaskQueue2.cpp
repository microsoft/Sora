#include <windows.h>
#include <process.h>
#include "TaskQueue.h"

using namespace SoraDbgPlot::Task;

// only test compilation
int Test1()
{
	TaskQueue queue;

	std::function<void(int)> f = [&queue](int start){
		queue.QueueTask([start](){
			TaskQueue queue1;
			int tag = start;
			for (int i = 0; i < 10; i++)
			{
				queue1.QueueTask([&tag](){
					printf("queue 1 %d\n", tag++);
				});
			}
			int start2 = start;
			queue1.DoTask([start2](){
				printf("queue 1 Do Task %d\n", start2);
			});
		});
	};

	while(1)
	{
		::Sleep(1000);
		f(0);
	}
	//f(1000);
	//f(1000000);

	queue.DoTask([](){
		printf("do task\n");
	});

	return 0;
}

struct TestPara
{
	TaskQueue * queue;
	int * testNum;
	int id;
};

//static void __cdecl ThreadTest2(PVOID param)
static unsigned int __stdcall ThreadTest2(PVOID param)
{
	auto testParam = (TestPara *)param;
	TaskQueue * queue = testParam->queue;
	int * num = testParam->testNum;
	int id = testParam->id;
	for (int i = 0; i < 10; i++)
	{
		queue->QueueTask([num, id](){
			printf("q %d, %d\n", id, (*num)++);
		});
	}

	for (int i = 0; i < 10; i++)
	{
		queue->DoTask([num, id](){
			printf("d %d, %d\n", id, (*num)++);
		});
	}

	//_endthread();
	//_endthreadex(0);
	return 0;
}

int Test2()
{
	TaskQueue queue;

	int tag = 0;
	const int T_COUNT = 64;
	HANDLE hThread[T_COUNT];
	TestPara param[T_COUNT];

	do
	{
		for (int i = 0; i < T_COUNT; i++)
		{
			param[i].id = i;
			param[i].queue = &queue;
			param[i].testNum = &tag;
			//hThread[i] = (HANDLE) _beginthread(ThreadTest2, 0, param + i);
			hThread[i] = (HANDLE) _beginthreadex(NULL, 0, ThreadTest2, param + i, 0, NULL);
			assert(hThread[i] != 0);
		}

		::WaitForMultipleObjects(T_COUNT, hThread, TRUE, INFINITE);
		for (int i = 0; i < T_COUNT; i++)
		{
			::CloseHandle(hThread[i]);
			hThread[i] = 0;
		}
	} while(0);

	return 0;
}

static int Test3()
{
	int count = 0;
	auto queue = std::make_shared<SoraDbgPlot::Task::TaskQueue>();
	for (int i = 0; i < 128; i++)
	{
		queue->QueueTask([&count]() mutable {
			printf("count --- %d\n", count++);
		});
	}

	getchar();
	queue.reset();

	while(1)
	{
		int dummy;
		if (TaskQueue::WaitAndClean(1000) == 0)
			break;
		printf("Wait for queue to complete\n");
	}

	return 0;
}

int TestTaskQueue2()
{
	//Test2();
	//Test1();
	Test3();
	return 0;
}
