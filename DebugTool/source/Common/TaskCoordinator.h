#pragma once

#include <memory>
#include <list>
#include <assert.h>
#include "Runnable.h"
#include "TaskSimple.h"

namespace SoraDbgPlot { namespace Task {

	class TaskCoordinator : public Runnable
	{
	public:
		TaskCoordinator(std::shared_ptr<SoraDbgPlot::Task::TaskQueue> queue)
		{
			_taskQueue = queue;
		}

		~TaskCoordinator()
		{
			int debug = 10;
		}

		void AddTask(std::shared_ptr<TaskSimple> t)
		{
			_taskList.push_back(t);
		}

		void AddTask(std::shared_ptr<TaskSimple> t[], int count)
		{
			for (int i = 0; i < count; i++)
			{
				_taskList.push_back(t[i]);
			}
		}

		void Run()
		{
			if (_taskList.size() == 0)
				_runnableContinue->Run();

			auto shared_me = std::dynamic_pointer_cast<TaskCoordinator, Runnable>(shared_from_this());
			_remainCount = 0;
			std::for_each(_taskList.begin(), _taskList.end(), [shared_me](std::shared_ptr<TaskSimple> task){
				shared_me -> _remainCount++;
				auto shared_me_2 = shared_me;

				task->ContinueWith(std::make_shared<SoraDbgPlot::Task::TaskSimple>(shared_me_2->_taskQueue, [shared_me_2](){
					if (::InterlockedDecrement(&shared_me_2->_remainCount) == 0)
					{
						if (shared_me_2->_runnableContinue)
							shared_me_2->_runnableContinue->Run();

						shared_me_2->_taskList.clear();
					}					
				}));

				task->Run();
			});
		}

		virtual std::shared_ptr<Runnable> ContinueWith(std::shared_ptr<Runnable> t)
		{
			if (_runnableContinue)
				return _runnableContinue->ContinueWith(t);
			else
			{
				_runnableContinue = t;
				return t;
			}
		}

	private:
		TaskCoordinator(const TaskCoordinator &) {}
		TaskCoordinator(TaskCoordinator &&) {}

	private:
		std::shared_ptr<Runnable> _runnableContinue;
		std::list<std::shared_ptr<TaskSimple> > _taskList;
		volatile unsigned long _remainCount;
		std::shared_ptr<SoraDbgPlot::Task::TaskQueue> _taskQueue;
	};
}}
