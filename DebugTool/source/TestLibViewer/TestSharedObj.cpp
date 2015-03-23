#include <assert.h>
#include <map>
#include <memory>
#include <vector>
#include <algorithm>
#include "SharedChannelManager.h"
#include "SharedProcess.h"
#include "SharedChannel.h"
#include "TaskQueue.h"
#include "SharedNameManagement.h"
#include "SharedSerialNumGenerator.h"

using namespace std;
using namespace SoraDbgPlot::SharedObj;
using SoraDbgPlot::Task::TaskQueue;

static 	SoraDbgPlot::Common::SharedSerialNumGenerator generator(
		SharedNameManager::GetSerialNumGeneratorName()
		);

static void PrintMap(const map<int, shared_ptr<SharedChannel> > & map)
{
	printf("Channel Map:\n");
	for_each(map.begin(), map.end(), [](pair<int, shared_ptr<SharedChannel> > p){
		printf("%d\t|\t%d|\t%S\t\n", p.second->Id(), p.second->Pid(), p.second->Name());
	});
}

static void PrintProcess(const map<int, shared_ptr<SharedProcess> > & map)
{
	printf("Process Map:\n");
	for_each(map.begin(), map.end(), [](pair<int, shared_ptr<SharedProcess> > p){
		printf("%d\t\n", p.second->Pid());
	});
}

static VOID CALLBACK TimerCallBack(
  __in  PVOID lpParameter,
  __in  BOOLEAN TimerOrWaitFired
)
{
	auto manager = (SharedChannelManager *)lpParameter;
	manager->Update();
	//manager->DoGarbageCollection();
}

int TestSharedObj()
{
	auto manager = make_shared<SharedChannelManager>();
	map<int, shared_ptr<SharedChannel> > chMap;
	map<int, shared_ptr<SharedProcess> > proMap;
	auto taskQueue = make_shared<TaskQueue>();

	manager->EventDiscoverdChannel.Subscribe([&chMap, taskQueue](const void * sender, const vector<shared_ptr<SharedChannel> > & vecCh){

		//const vector<shared_ptr<SharedChannel> > vecChCopy = vecCh;
		map<int, shared_ptr<SharedChannel> > & __chMap = chMap;

		taskQueue->QueueTask([vecCh, &__chMap](){
			for (auto iter = vecCh.begin(); iter != vecCh.end(); ++iter)
			{
				auto iterFindInMap = __chMap.find((*iter)->Id());
				if (iterFindInMap != __chMap.end())
				{
					printf("%d\n", (*iter)->Id());
					assert(false);
				}
				assert(iterFindInMap == __chMap.end());
				__chMap.insert(make_pair((*iter)->Id(), *iter));
			}
			
			printf("Channel discovered:\n");
			PrintMap(__chMap);
			printf("\n");
		});
	});

	manager->EventChannelClosed.Subscribe([&chMap, taskQueue](const void * sender, const vector<shared_ptr<SharedChannel> > & vecCh){

		map<int, shared_ptr<SharedChannel> > & __chMap = chMap;

		taskQueue->QueueTask([vecCh, &__chMap](){
			for (auto iter = vecCh.begin(); iter != vecCh.end(); ++iter)
			{
				auto iterFindInMap = __chMap.find((*iter)->Id());
				assert(iterFindInMap != __chMap.end());
				__chMap.erase(iterFindInMap);
			}

			printf("Channel closed:\n");
			PrintMap(__chMap);
			printf("\n");
		});
	});

	manager->EventDiscoverdProcess.Subscribe([&proMap, taskQueue](const void * sender, const vector<shared_ptr<SharedProcess> > & vecPro){
		map<int, shared_ptr<SharedProcess> > & __proMap = proMap;
		taskQueue->QueueTask([vecPro, &__proMap](){
			for (auto iter = vecPro.begin(); iter != vecPro.end(); ++iter)
			{
				auto iterFindInMap = __proMap.find((*iter)->Pid());
				if (iterFindInMap != __proMap.end())
				{
					printf("%d\n", (*iter)->Pid());
					assert(false);
				}
				__proMap.insert(make_pair((*iter)->Pid(), *iter));
			}

			printf("Process dicovered:\n");
			PrintProcess(__proMap);
			printf("\n");
		});
	});

	manager->EventProcessClosed.Subscribe([&proMap, taskQueue](const void * sender, const vector<shared_ptr<SharedProcess> > & vecPro){
		map<int, shared_ptr<SharedProcess> > & __proMap = proMap;
		taskQueue->QueueTask([vecPro, &__proMap](){
			for (auto iter = vecPro.begin(); iter != vecPro.end(); ++iter)
			{
				auto iterFindInMap = __proMap.find((*iter)->Pid());
				assert(iterFindInMap != __proMap.end());
				__proMap.erase(iterFindInMap);
			}

			printf("Process closed\n");
			PrintProcess(__proMap);
			printf("\n");
		});
	});

	manager->Update();
	::Sleep(1000);
	manager->Update();
	::Sleep(1000);

	HANDLE hTimerQueueTimer;
	BOOL succ = ::CreateTimerQueueTimer(
		&hTimerQueueTimer,
		NULL,
		TimerCallBack,
		manager.get(),
		15,
		15,
		WT_EXECUTEDEFAULT);
	assert(succ);

	printf("Press any key to exit\n");
	getchar();

	succ = ::DeleteTimerQueueTimer(NULL, hTimerQueueTimer, NULL);
	assert(succ);

	//::Sleep(5000);

	//manager.reset();
	//printf("Before destroy taskqueue\n");
	//taskQueue.reset();
	//printf("After destroy taskqueue\n");

	manager.reset();
	taskQueue.reset();
	chMap.clear();
	proMap.clear();

	while(1)
	{
		printf("Wait for all task to finish\n");
		if (TaskQueue::WaitAndClean(1000) == 0)
			break;
	}

	return 0;
}


