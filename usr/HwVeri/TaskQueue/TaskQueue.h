#pragma once

#include <Windows.h>
#include <process.h>
#include <list>
#include <vector>
#include "Task.h"
#include "Logger.h"

class TaskQueue
{
public:
	TaskQueue();
	~TaskQueue();
	void AddTask(Task * task);
	Task * GetTask();
	void AskAllThreadsToExit();
	void AddWorkerThread(int count);

private:
	std::list<Task *> queue;
	std::vector<HANDLE> threadHandles;

	HANDLE eventNewTask;
	HANDLE mutex;

	bool stopFlag;

	Logger * logger;

	static unsigned __stdcall TaskQueue::Thread(void *argList);

};
