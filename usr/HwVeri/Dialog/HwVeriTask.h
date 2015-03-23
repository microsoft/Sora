#pragma once

#include <vector>
#include "Task.h"
#include "Logger.h"
#include "Message.h"

class TaskPrint : public Task
{
public:
	void Run();
	HRESULT Reset();

	TaskPrint(Logger * logger, int timems);
private:
	Logger * logger;
	int timems;
};

struct TaskVoidContext
{
	TASK_TYPE type;
	Task * task;
	HWND hwnd;
	TASK_STATE state;
};

class TaskVoid : public Task
{
public:
	void Run();
	TaskVoid(TaskVoidContext *);
private:
	TaskVoidContext * context;
};
