#include <memory>
#include "Event.h"
#include "TaskQueue.h"

using namespace std;
using namespace SoraDbgPlot::Event;
using namespace SoraDbgPlot::Task;

class A : public enable_shared_from_this<A>
{
public:
	Event<bool> EventClose;

	A()
	{
		_taskQueue = make_shared<TaskQueue>();
	}
	~A()
	{
		;
	}

	void Close()
	{
		auto SThis = shared_from_this();
		_taskQueue->QueueTask([SThis](){
			SThis->EventClose.Raise(0, true);
		});
	}

private:
	shared_ptr<TaskQueue> _taskQueue;
};

static int Test1()
{
	auto a = make_shared<A>();
	a->EventClose.Subscribe([&a](void * sender, bool dummy) mutable {
		a.reset();
	});
	
	a->Close();



	getchar();

	int n = a.use_count();

	return 0;
}

int TestSelfDelete()
{
	Test1();
	return 0;
}
