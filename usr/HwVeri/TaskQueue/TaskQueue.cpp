#include "TaskQueue.h"


TaskQueue::TaskQueue()
{
	mutex = CreateMutex(
        NULL,
        FALSE,
        NULL);

	eventNewTask = CreateEvent( 
        NULL,
        FALSE,						// auto reset event
        FALSE,						// initial state is reset
        TEXT("EventNewTask")
        );

	stopFlag = false;

	logger = Logger::GetLogger(L"TaskQueue");
}

TaskQueue::~TaskQueue()
{
	CloseHandle(mutex);
	CloseHandle(eventNewTask);
}

void TaskQueue::AddTask(Task * task)
{
	if (task == 0)
		return;

	WaitForSingleObject(mutex, INFINITE);
	queue.push_back(task);
	ReleaseMutex(mutex);

	SetEvent(eventNewTask);
}

Task * TaskQueue::GetTask()
{
	Task * task = 0;

	while(1)
	{
		if (stopFlag)
			return 0;

		WaitForSingleObject(mutex, INFINITE);
		if (!queue.empty())
		{
			task = queue.front();
			queue.pop_front();
		}
		ReleaseMutex(mutex);

		if (task)
			return task;
		else
		{
			WaitForSingleObject(eventNewTask, INFINITE);
		}
	}
}

unsigned __stdcall TaskQueue::Thread(void *argList) {

	DWORD threadID = GetCurrentThreadId();
	Task * task;
	TaskQueue * taskQueue = (TaskQueue *)argList;
	while(1)
	{
		taskQueue->logger->Log(LOG_DEFAULT, L"Thread[%d] Before GetTask().\n", threadID);
		task = taskQueue->GetTask();
		taskQueue->logger->Log(LOG_DEFAULT, L"Thread[%d] After GetTask().\n", threadID);

		if (task)
		{
			task->Run();
			taskQueue->logger->Log(LOG_DEFAULT, L"Thread[%d] Task complete.\n", threadID);
		}
		else
			break;
	}

	taskQueue->logger->Log(LOG_DEFAULT, L"Thread[%d] Before exit.\n", threadID);

	return 0;
}

void TaskQueue::AddWorkerThread(int count)
{
	if (count <= 0)
		return;

	HANDLE hThread;
	unsigned int threadId;
	for (int i = 0; i < count; i++)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, Thread, this, 0, &threadId);
		threadHandles.push_back(hThread);
		logger->Log(LOG_DEFAULT, L"Add thread %d\n", threadId);
	}
}

void TaskQueue::AskAllThreadsToExit()
{
	stopFlag = true;
	WaitForMultipleObjects(threadHandles.size(), &threadHandles[0], true, INFINITE);
}
