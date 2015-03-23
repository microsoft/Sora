#pragma once

#include <assert.h>
#include <memory>
#include <algorithm>
#include "TaskQueue.h"
#include "Runnable.h"

namespace SoraDbgPlot { namespace Task
{
	class TaskSimple : public Runnable
	{
	public:
		TaskSimple(std::shared_ptr<TaskQueue> queue, const TaskQueue::TaskFunc & f)
		{
			_func = f;
			_dtor = [](){};
			_taskQueue = queue;
		}

		TaskSimple(std::shared_ptr<TaskQueue> queue, const TaskQueue::TaskFunc & f, const TaskQueue::TaskFunc & dtor)
		{
			_func = f;
			_dtor = dtor;
			_taskQueue = queue;
		}

		~TaskSimple()
		{
			;
		}
		
		std::shared_ptr<SoraDbgPlot::Task::Runnable> ContinueWith(std::shared_ptr<SoraDbgPlot::Task::Runnable> t)
		{
			if (_runnableContinue)
				return _runnableContinue->ContinueWith(t);
			else
			{
				_runnableContinue = t;
				return t;
			}
		}

		void Run()
		{
			auto func = _func;
			auto dtor = _dtor;
			auto runnableContinue = _runnableContinue;
			_taskQueue->QueueTask([func, runnableContinue](){
				func();
				if (runnableContinue)
					runnableContinue->Run();
			}, [dtor, runnableContinue](){
				dtor();
				if (runnableContinue)
					runnableContinue->Run();
			});
		}

	private:
		TaskSimple(const TaskSimple & other) {}
		TaskSimple(TaskSimple && other) {}

	private:
		TaskQueue::TaskFunc _func;
		TaskQueue::TaskFunc _dtor;
		std::shared_ptr<TaskQueue> _taskQueue;
		std::shared_ptr<Runnable> _runnableContinue;
	};
}}

