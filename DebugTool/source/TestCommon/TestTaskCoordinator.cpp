#include <assert.h>
#include <memory>
#include "TaskQueue.h"
#include "TaskSimple.h"
#include "TaskCoordinator.h"

using namespace std;
using namespace SoraDbgPlot::Task;

int Test1()
{
	auto count = 0;
	auto res1 = make_shared<int>(0);

	auto queue1 = make_shared<TaskQueue>();
	auto queue2 = make_shared<TaskQueue>();

	auto coordinator = make_shared<TaskCoordinator>(queue2);

	for (int i = 0; i < 10; i++)
	{
		coordinator->AddTask(make_shared<TaskSimple>(queue1, [res1](){
			::Sleep(100);
			printf("%d\n", (*res1)++);
		}));
	}
	
	coordinator->ContinueWith(make_shared<TaskSimple>(queue1, [res1](){
		printf("Result: %d\n", *res1);
	}))->ContinueWith(make_shared<TaskSimple>(queue2, [&count](){
		printf("Count 0: %d\n", count++);
	}))->ContinueWith(make_shared<TaskSimple>(queue1, [&count](){
		printf("Count 1: %d\n", count++);
	}))->ContinueWith(make_shared<TaskSimple>(queue2, [&count](){
		printf("Count 2: %d\n", count++);
	}))->ContinueWith(make_shared<TaskSimple>(queue1, [&count](){
		printf("Count 3: %d\n", count++);
	}))->ContinueWith(make_shared<TaskSimple>(queue2, [&count](){
		printf("Count 4: %d\n", count++);
	}));

	coordinator->ContinueWith(make_shared<TaskSimple>(queue2, [&count](){
		printf("Count 5: %d\n", count++);
	}))->ContinueWith(make_shared<TaskSimple>(queue2, [&count](){
		printf("Count 6: %d\n", count++);
	}));


	coordinator->Run();

	coordinator.reset();
	queue1.reset();
	res1.reset();
	queue2.reset();

	while(1)
	{
		if (TaskQueue::WaitAndClean(1000) == true)
			break;

		int c = coordinator.use_count();
		c = queue1.use_count();
		c = res1.use_count();
		int i = 10;
	}

	return 0;
}


class A : public enable_shared_from_this<A>
{
public:
	A()
	{
		;
	}
	~A()
	{
		;
	}
	void AccessSharedThis()
	{
		auto p = shared_from_this();
	}

	void AccessSharedThis2()
	{
		auto p = shared_from_this();
	}
};

int Test2()
{
	auto pa = make_shared<A>();
	pa->AccessSharedThis();
	pa->AccessSharedThis2();
	pa.reset();
	return 0;
}

int TestTaskCoordinator()
{
	Test1();
	return 0;
}
