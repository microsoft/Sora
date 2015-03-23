#pragma once

#include <functional>
#include <vector>
#include <list>
#include <memory>
#include "SharedChannel.h"
#include "SharedProcess.h"
#include "IChannelBuffer.h"
#include "ChannelBufferImpl.h"
#include "Event.h"

namespace SoraDbgPlot { namespace SharedObj {

	class SharedChannelManager : public std::enable_shared_from_this<SharedChannelManager>
	{
	public:
		SoraDbgPlot::Event::Event<std::vector<std::shared_ptr<SharedChannel> > > EventDiscoverdChannel;
		SoraDbgPlot::Event::Event<std::vector<std::shared_ptr<SharedChannel> > > EventChannelClosed;
		SoraDbgPlot::Event::Event<std::vector<std::shared_ptr<SharedProcess> > > EventDiscoverdProcess;
		SoraDbgPlot::Event::Event<std::vector<std::shared_ptr<SharedProcess> > > EventProcessClosed;

		SharedChannelManager();
		~SharedChannelManager();

		void Update();
		void DoGarbageCollection();
		void Clear();

	private:

		void DoTask_Update();

		static bool __stdcall ReadDataFunction(const char * buffer, int size, void * userData, __int32 userId);

		void DoTask_GarbageCollection();

		std::shared_ptr<IChannelBufferReadable> _channelBufferReadable;
		
		std::map<int, std::shared_ptr<SharedChannel> > _activeChannelMap;
		std::vector<std::shared_ptr<SharedChannel> > _newDiscoverdChannel;

		std::map<int, std::shared_ptr<SharedProcess> > _activeProcessMap;
		std::vector<std::shared_ptr<SharedProcess> > _newDiscoverdProcess;

		std::shared_ptr<SoraDbgPlot::Task::TaskQueue> _taskQueue;

		volatile unsigned long _updateInstanceCnt;
		volatile unsigned long _gcInstanceCnt;

		ShareLock * _syncEvent;
	};
}}
