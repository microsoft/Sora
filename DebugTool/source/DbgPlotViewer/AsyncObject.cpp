#include "stdafx.h"
#include "AsyncObject.h"

using namespace std;

AsyncObject::AsyncObject()
{
	_taskQueue = make_shared<SoraDbgPlot::Task::TaskQueue>();
}

AsyncObject::~AsyncObject()
{

}

void AsyncObject::DoLater(const std::function<void(void)> & f)
{
	_taskQueue->QueueTask([f](){
		f();
	});
}

void AsyncObject::DoLater(const std::function<void(void)> & f, const std::function<void(void)> & dtor)
{
	_taskQueue->QueueTask([f](){
		f();
	}, [dtor](){
		dtor();
	});
}

void AsyncObject::DoNow(const std::function<void(void)> & f)
{
	_taskQueue->DoTask([f](){
		f();
	});
}

std::shared_ptr<SoraDbgPlot::Task::TaskQueue> AsyncObject::TaskQueue()
{
	return _taskQueue;
}
