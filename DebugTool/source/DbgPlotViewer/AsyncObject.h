#pragma once

#include <memory>
#include <functional>
#include "TaskQueue.h"

class AsyncObject : public std::enable_shared_from_this<AsyncObject>
{
public:
	AsyncObject();
	virtual ~AsyncObject();
	void DoLater(const std::function<void(void)> & f);
	void DoLater(const std::function<void(void)> & f, const std::function<void(void)> & dtor);
	void DoNow(const std::function<void(void)> & f);
	std::shared_ptr<SoraDbgPlot::Task::TaskQueue> TaskQueue();

private:
	std::shared_ptr<SoraDbgPlot::Task::TaskQueue> _taskQueue;
};
