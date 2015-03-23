#include <assert.h>
#include <memory>
#include <list>
#include "TaskQueue.h"
#include "TaskSimple.h"

using namespace std;
using namespace SoraDbgPlot::Task;

static int Test1()
{
	auto queue1 = make_shared<TaskQueue>();
	int res1 = 0;

	auto queue2 = make_shared<TaskQueue>();
	int res2 = 0;

	auto task1 = make_shared<TaskSimple>(queue1, [&res1](){
		printf("func1 begin\n");
		::Sleep(1000);
		printf("func1 end, %d\n", res1++);
	});

	auto task2 = make_shared<TaskSimple>(queue2, [&res2](){
		printf("func2 begin\n");
		::Sleep(1000);
		printf("func2 end, %d\n", res2++);
	});

	task1->ContinueWith(task2);

	task1->Run();

	return 0;
}

int TestTaskSimple()
{
	Test1();
	return 0;
};

