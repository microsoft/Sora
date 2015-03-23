#pragma once

#include <functional>
#include <list>
#include <memory>
#include <assert.h>
#include "CSLock.h"
#include "CSRecursiveLock.h"
#include "ReadWriteLock.h"

namespace SoraDbgPlot { namespace Task {

	class TaskQueue : public std::enable_shared_from_this<TaskQueue>
	{
	public:
		typedef std::function<void(void)> TaskFunc;

		TaskQueue()
		{
			::InterlockedIncrement(&TaskQueue::__countQueue);
			_exeToken = TASK_TOKEN_NOT_TAKEN;
		}

	private:
		TaskQueue(const TaskQueue &) {}
		TaskQueue(TaskQueue &&) {}

	public:
		~TaskQueue()
		{
			::SetEvent(TaskQueue::__eventWait);
			::InterlockedDecrement(&TaskQueue::__countQueue);
		}

		void DoTask(const TaskFunc & f)
		{
			_lockRunning.Lock();
			f();
			_lockRunning.Unlock();

			ExecuteTaskAsync();
		}

		void QueueTask(const TaskFunc & f)
		{
#ifdef _DEBUG
			::InterlockedIncrement(&__taskQueueOpCount);
#endif

			::InterlockedIncrement(&__taskNum);
			::InterlockedIncrement(&__sid);

			TaskStruct taskStruct;
			taskStruct._func = f;
			taskStruct._doClean = false;

			_lockList.Lock();
			_taskList.push_back(taskStruct);
#ifdef _DEBUG
			::InterlockedIncrement(&__taskCntInQueue);
#endif
			_lockList.Unlock();

			ExecuteTaskAsync();

#ifdef _DEBUG
			::InterlockedDecrement(&__taskQueueOpCount);
#endif
		}

		void QueueTask(const TaskFunc & f, const TaskFunc & cleanFunc)
		{

			::InterlockedIncrement(&__taskNum);
			::InterlockedIncrement(&__sid);

			TaskStruct taskStruct;
			taskStruct._func = f;
			taskStruct._cleanFunc = cleanFunc;
			taskStruct._doClean = true;

			_lockList.Lock();
			_taskList.push_back(taskStruct);

#ifdef _DEBUG
			::InterlockedIncrement(&__taskCntInQueue);
#endif
			_lockList.Unlock();

			ExecuteTaskAsync();
		}

	private:
		static DWORD WINAPI ThreadExecuteTask(__in  LPVOID lpParameter)
		{
#ifdef _DEBUG
			::InterlockedIncrement(&__taskCntRunning);
			::InterlockedDecrement(&__taskCntToRun);
#endif			

			auto queue = (TaskQueue *)lpParameter;
			auto SThis = queue->_me;

			queue->_lockRunning.Lock();

			if (!__clean)
			{
				queue->_task._func();
			}
			else
			{
				if (queue->_task._doClean)
					queue->_task._cleanFunc();				
			}

			queue->_lockRunning.Unlock();

			queue->_task._func = [](){};
			queue->_task._cleanFunc = [](){};
			queue->_task._doClean = false;
			queue->_me.reset();

			queue->_lockList.Lock();
			queue->_exeToken = TASK_TOKEN_NOT_TAKEN;
			queue->_lockList.Unlock();

			queue->ExecuteTaskAsync();
			
#ifdef _DEBUG
			::InterlockedDecrement(&__taskCntRunning);
#endif
			::InterlockedDecrement(&__taskNum);

			return 0;
		}

		void ExecuteTaskAsync()
		{
			bool takeATask = false;

			_lockList.Lock();

			if (_exeToken == TASK_TOKEN_NOT_TAKEN)
			{
				if (_taskList.size() > 0)
				{
					_task = _taskList.front();
					_taskList.pop_front();
#ifdef _DEBUG
					::InterlockedDecrement(&__taskCntInQueue);
#endif
					_exeToken = TASK_TOKEN_TAKEN;
					_me = shared_from_this();
					takeATask = true;
				}
			}

			_lockList.Unlock();

			if (takeATask)
			{
				BOOL succ = QueueUserWorkItem(ThreadExecuteTask, this, WT_EXECUTEDEFAULT);
				if (succ != 0)
				{
#ifdef _DEBUG
					::InterlockedIncrement(&__taskCntToRun);
#endif
				}
				else
				{
					assert(false);
				}
			}
		}


		struct TaskStruct
		{
			TaskFunc _func;
			TaskFunc _cleanFunc;
			bool _doClean;
		};

		TaskStruct _task;
		std::list<TaskStruct> _taskList;
		SoraDbgPlot::Lock::CSLock _lockRunning;
		SoraDbgPlot::Lock::CSLock _lockList;
		volatile unsigned long _exeToken;
		std::shared_ptr<TaskQueue> _me;

		enum {
			TASK_TOKEN_TAKEN,
			TASK_TOKEN_NOT_TAKEN
		};

	// resource management
	public:
		static int __stdcall WaitAndClean(DWORD time)
		{
			__clean = true;
			while(1)
			{
				::WaitForSingleObject(__eventWait, time);
				int activeCount = __countQueue;
				if (activeCount == 0)
				{
					::CloseHandle(__eventWait);
				}

				return activeCount;
			}
		}

#ifdef _DEBUG
		static unsigned long MonitorQueueNum()
		{
			return __countQueue;
		}

		static unsigned long MonitorTaskInQueue()
		{
			return __taskCntInQueue;
		}

		static unsigned long MonitorTaskCntToRun()
		{
			return __taskCntToRun;
		}

		static unsigned long MonitorTaskCntRunning()
		{
			return __taskCntRunning;
		}

		static unsigned long MonitorTaskQueueOpCnt()
		{
			return __taskQueueOpCount;
		}

#endif

		static unsigned long Sid()
		{
			return __sid;
		}

		static unsigned long RemainingTask()
		{
			return __taskNum;
		}

	private:
#ifdef _DEBUG
		static volatile unsigned long __taskCntInQueue;
		static volatile unsigned long __taskCntToRun;
		static volatile unsigned long __taskCntRunning;
		static volatile unsigned long __taskQueueOpCount;
#endif
		static HANDLE __eventWait;
		static volatile unsigned long __countQueue;
		static volatile bool __clean;
		static volatile unsigned long __sid;
		static volatile unsigned long __taskNum;
	};

}}
