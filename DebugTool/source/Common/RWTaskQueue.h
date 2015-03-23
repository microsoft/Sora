#pragma once

#include <assert.h>
#include <functional>
#include <list>
#include "ReadWriteLock.h"

#if 0

namespace SoraDbgPlot { namespace Task {

	class RWTaskQueue
	{
	public:
		typedef std::function<void(void)> TaskFunc;
	private:
		enum TaskType
		{
			TYPE_READ,
			TYPE_WRITE,
		};

		struct Task
		{
			TaskType _type;
			TaskFunc _func;
		};

	public:
		RWTaskQueue()
		{
			_exeToken = 0;
			_activeCount = 0;
		}

		~RWTaskQueue()
		{
			while(1)
			{
				bool bBreak = false;
				_lockList.Lock();
				if (_taskList.size() == 0)
					bBreak = true;
				_lockList.Unlock();

				if (bBreak) break;
			}
		}

		void DoTaskWrite(const TaskFunc & f)
		{
			::InterlockedIncrement(&_activeCount);

			_lockRunning.LockWrite();
			f();
			_lockRunning.UnlockWrite();

			if (::InterlockedDecrement(&_activeCount) == 0)
				ExecuteTaskAsync(false);
		}

		void QueueTaskRead();
		void QueueTaskWrite();

	private:
		void ExecuteTaskAsync(bool doPop)
		{
			std::list<TaskFunc *> _newTaskList;

			_lockList.Lock();

			if (doPop)
			{
				_exeToken = false;
			}

			if (_exeToken == false)
			{
				if (_taskList.size() > 0)
				{


					_exeToken = true;

				}
			}

			_lockList.Unlock();

			while(_newTaskList.si

			if (_newTaskList.size() > 0)
			{
				BOOL succ = QueueUserWorkItem(ThreadExecuteTask, this, WT_EXECUTEDEFAULT);
				assert(succ != 0);
			}
		}

		static DWORD WINAPI ThreadExecuteTask(__in  LPVOID lpParameter)
		{
			auto queue = (TaskQueue *)lpParameter;

			//::InterlockedIncrement(&queue->_activeCnt);

			queue->_lockRunning.Lock();
			queue->_task();
			queue->_lockRunning.Unlock();

			//queue->_lockList.Lock();
			//queue->_taskList.pop_front();
			//queue->_lockList.Unlock();

			//if (::InterlockedDecrement(&queue->_activeCnt) == 0)
				queue->ExecuteTaskAsync(true);
			
			return 0;
		}

	private:
		TaskFunc _task;
		std::list<Task> _taskList;
		SoraDbgPlot::Lock::RWLock _lockRunning;
		SoraDbgPlot::Lock::CSLock _lockList;
		//SoraDbgPlot::Lock::CSRecursiveLock _lockRunning;
		//SoraDbgPlot::Lock::CSLock _lockList;
		//HANDLE _evenTaskComplete;
		//volatile unsigned long _activeCnt;
		volatile unsigned long _exeToken;
		volatile unsigned long _activeCount;
		TaskType _taskType;
	};

}}


#endif
