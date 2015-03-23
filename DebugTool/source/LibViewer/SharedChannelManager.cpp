#include <vector>
#include <memory>
#include "SharedNameManagement.h"
#include "SharedChannelManager.h"
#include "SmChannel.h"
#include "AppSettings.h"

using namespace std;
using namespace SoraDbgPlot::SharedObj;
using namespace SoraDbgPlot::Task;

SharedChannelManager::SharedChannelManager()
{
	_updateInstanceCnt = 0;
	_gcInstanceCnt = 0;

	_channelBufferReadable = std::shared_ptr<IChannelBufferReadable>(
		ChannelBuffer::OpenForRead(
		SettingGetGlobalBufferName(),
		SettingGetGlobalBufferBlockNum(),
		SettingGetGlobalBufferBlockSize())
		);

	// event
	wchar_t * name = SharedNameManager::GenEventName();
	_syncEvent = AllocateSharedEvent(TRUE, TRUE, name);
	delete [] name;	

	_taskQueue = make_shared<TaskQueue>();
}

SharedChannelManager::~SharedChannelManager()
{
	if (_syncEvent)
		ShareLock::FreeShareLock(_syncEvent);
}

void SharedChannelManager::Clear()
{
	auto SThis = shared_from_this();

	auto clearTask = [SThis](){
		SThis->EventChannelClosed.Reset();
		SThis->EventDiscoverdChannel.Reset();
		SThis->EventDiscoverdProcess.Reset();
		SThis->EventProcessClosed.Reset();
	};

	_taskQueue->QueueTask(clearTask, clearTask);
}

void SharedChannelManager::Update()
{
	if (::InterlockedIncrement(&_updateInstanceCnt) == 1)
	{
		auto SThis = shared_from_this();
		_taskQueue->QueueTask([SThis](){
			SThis->DoTask_Update();
			::InterlockedDecrement(&SThis->_updateInstanceCnt);
		});
	}
	else
	{
		::InterlockedDecrement(&_updateInstanceCnt);
	}
}

void SharedChannelManager::DoGarbageCollection()
{
	if (::InterlockedIncrement(&_gcInstanceCnt) == 1)
	{
		auto SThis = shared_from_this();
		_taskQueue->QueueTask([SThis](){
			SThis->DoTask_GarbageCollection();
			::InterlockedDecrement(&SThis->_gcInstanceCnt);
		});
	}
	else
	{
		::InterlockedDecrement(&_gcInstanceCnt);
	}
}

void SharedChannelManager::DoTask_Update() {
	_channelBufferReadable->ReadData(ReadDataFunction, this);

	int bufSizeAvailable = _channelBufferReadable->BufferSizeAvailable();

	if (_channelBufferReadable->BufferSizeAvailable() > SettingGetGlobalBufferThreadHoldHigh())
	{
		_syncEvent->SetShareEvent();
	}

	if (_newDiscoverdProcess.size() > 0)
	{
		EventDiscoverdProcess.Raise(this, _newDiscoverdProcess);
		_newDiscoverdProcess.clear();
	}

	if (_newDiscoverdChannel.size() > 0)
	{
		EventDiscoverdChannel.Raise(this, _newDiscoverdChannel);
		_newDiscoverdChannel.clear();
	}
}

bool __stdcall SharedChannelManager::ReadDataFunction(const char * buffer, int size, void * userData, __int32 userId)
{
	auto manager = (SharedChannelManager *)userData;
	auto iter = manager->_activeChannelMap.find(userId);
	if (iter != manager->_activeChannelMap.end())
	{
		return iter->second->Write(buffer, size);
	}
	else
	{
		//char * buf = new char[size];
		//memcpy(buf, buffer, size);

		auto smChannel = std::make_shared<SmChannel>(userId);

		if (!smChannel->IsValid())
			return false;

		auto channel = std::make_shared<SharedChannel>(smChannel);
		manager->_activeChannelMap.insert(
			std::make_pair(userId, channel)
			);

		manager->_newDiscoverdChannel.push_back(channel);

		auto iterProcess = manager->_activeProcessMap.find(channel->Pid());
		if (iterProcess == manager->_activeProcessMap.end())
		{
			auto process = make_shared<SharedProcess>(channel->Pid());
			if (process->IsValid())
			{
				manager->_newDiscoverdProcess.push_back(process);
				manager->_activeProcessMap.insert(
					make_pair(process->Pid(), process)
					);
			}
		}

		return channel->Write(buffer, size);
	}
}

void SharedChannelManager::DoTask_GarbageCollection()
{
	// slow work
	std::vector<std::shared_ptr<SharedChannel> > vecSmToBeRemoved;
	std::map<int, bool> pidMap;

	// for std::list remove while iterating, refer to
	// http://stackoverflow.com/questions/596162/can-you-remove-elements-from-a-stdlist-while-iterating-through-it

	auto iter = this->_activeChannelMap.begin();

	while (iter != this->_activeChannelMap.end())
	{
		bool bDead = false;
		int pid = (*iter).second->Pid();

		auto iterPid = pidMap.find(pid);
		if (iterPid != pidMap.end())
		{
			bDead = iterPid->second;
		}
		else
		{
			HANDLE processHandle = ::OpenProcess(SYNCHRONIZE, FALSE, pid);
			if (processHandle == NULL)
				bDead = true;
			else
				::CloseHandle(processHandle);

			pidMap.insert(std::make_pair(pid, bDead));
		}

		if (bDead)
		{
			vecSmToBeRemoved.push_back((*iter).second);
			this->_activeChannelMap.erase(iter++);
		}
		else
		{
			++iter;
		}
	}

	vector<shared_ptr<SharedProcess> > vecProcessToBeRemoved;

	auto iterProcess = this->_activeProcessMap.begin();
	while (iterProcess != this->_activeProcessMap.end())
	{
		bool bDead = false;
		int pid = iterProcess->first;
		auto iterFound = pidMap.find(pid);
		if (iterFound != pidMap.end())
		{
			bDead = iterFound->second;
		}
		else
		{
			HANDLE processHandle = ::OpenProcess(SYNCHRONIZE, FALSE, pid);
			if (processHandle == NULL)
				bDead = true;
			else
				::CloseHandle(processHandle);

			pidMap.insert(std::make_pair(pid, bDead));		
		}

		if (bDead)
		{
			vecProcessToBeRemoved.push_back(iterProcess->second);
			this->_activeProcessMap.erase(iterProcess++);
		}
		else
			++iterProcess;
	}

	// fire channel event first
	if (vecSmToBeRemoved.size() > 0)
		this->EventChannelClosed.Raise(this, vecSmToBeRemoved);

	if (vecProcessToBeRemoved.size() > 0)
		this->EventProcessClosed.Raise(this, vecProcessToBeRemoved);
}
