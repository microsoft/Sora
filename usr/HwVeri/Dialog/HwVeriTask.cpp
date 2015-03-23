#include "HwVeriTask.h"
#include "Message.h"

TaskPrint::TaskPrint(Logger * logger, int timems)
{
	this->logger = logger;
	this->timems = timems;
}

void TaskPrint::Run()
{
	logger->Log(LOG_DEFAULT, L"Begin task, sleep for %d ms...\n", timems);
	::Sleep(timems);
	logger->Log(LOG_DEFAULT, L"End task\n");
}

HRESULT TaskPrint::Reset()
{
	return S_OK;
}

void TaskVoid::Run()
{
	context->task = this;
	context->state = TASK_STARTED;
	::PostMessage(context->hwnd, WM_TASK_DONE, TASK_VOID, (LPARAM)context);

	::Sleep(2000);

	context->state = TASK_DONE_SUCCESS;
	::PostMessage(context->hwnd, WM_TASK_DONE, TASK_VOID, (LPARAM)context);
}

TaskVoid::TaskVoid(TaskVoidContext * context)
{
	this->context = context;	
}
