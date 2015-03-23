#pragma once

#include <functional>
#include <Windows.h>

namespace SoraDbgPlot { namespace Task {

	class WaitableLatestTaskQueue
	{
		typedef std::function<void(void)> TaskType;
	
	public:
		WaitableLatestTaskQueue() {
			::InitializeCriticalSection(&_cs);
			_eventTaskComplete = ::CreateEvent(NULL, FALSE, TRUE, NULL);
			_bTaskRunning = false;
			_bTaskInQueue = false;
		}

		~WaitableLatestTaskQueue()
		{
			while(1)
			{
				bool bBreak = false;
				::WaitForSingleObject(_eventTaskComplete, INFINITE);

				::EnterCriticalSection(&_cs);
				if (_bTaskRunning == false && _bTaskInQueue == false)
				{
					bBreak = true;
				}
				::LeaveCriticalSection(&_cs);

				if (bBreak) break;
			}
			::CloseHandle(_eventTaskComplete);
			::DeleteCriticalSection(&_cs);
		}

		void QueueTask(const TaskType & task)
		{
			bool bExecute = false;

			::EnterCriticalSection(&_cs);
			if (!_bTaskRunning)
			{
				_bTaskRunning = true;
				_taskRunning = task;
				bExecute = true;
			}
			else
			{
				_bTaskInQueue = true;
				_taskInQueue = task;
			}

			::LeaveCriticalSection(&_cs);

			if (bExecute)
				ExecuteTask();
		}

	private:
		void ExecuteTask()
		{
			QueueUserWorkItem(ThreadExecuteTask, this, WT_EXECUTEDEFAULT);
		}

		static DWORD WINAPI ThreadExecuteTask(__in  LPVOID lpParameter)
		{
			auto queue = (WaitableLatestTaskQueue *)lpParameter;
			queue->_taskRunning();

			bool bExecute = false;

			::EnterCriticalSection(&queue->_cs);
			if (queue->_bTaskInQueue)
			{
				bExecute = true;
				queue->_taskRunning = queue->_taskInQueue;
				queue->_bTaskInQueue = false;
			}
			else
			{
				queue->_bTaskRunning = false;
			}

			::LeaveCriticalSection(&queue->_cs);

			if (bExecute)
				queue->ExecuteTask();

			::SetEvent(queue->_eventTaskComplete);

			return 0;
		}

	private:
		volatile bool _bTaskInQueue;
		volatile bool _bTaskRunning;
		TaskType _taskInQueue;
		TaskType _taskRunning;

		CRITICAL_SECTION _cs;

		HANDLE _eventTaskComplete;
	};
}}
