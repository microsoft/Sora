#pragma once

//#include "SharedStruct.h"
#include <memory>
#include <list>
#include "Strategy.h"
#include "TaskQueue.h"
#include "ShareMemHelper.h"
#include "SmChannel.h"
#include "Event.h"
#include "TaskSimple.h"

namespace SoraDbgPlot { namespace SharedObj {

	struct NewDataParam {
		const char * ptr;
		size_t length;
	};

	class SharedChannel : public std::enable_shared_from_this<SharedChannel>
	{
	public:
		SoraDbgPlot::Strategy::Strategy<NewDataParam, bool> StrategyNewData;

		int Id() const;
		int Pid() const;
		int SpectrumDataSize() const;
		const wchar_t * Name() const;
		ChannelType Type() const;

		SharedChannel(std::shared_ptr<SmChannel> sm);
		~SharedChannel();

		bool Write(const char * ptr, size_t length);

		//std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskGetData(shared_ptr<std::list<NewDataParam> > &);

		std::shared_ptr<std::list<NewDataParam> > CheckOut();

	private:

		std::shared_ptr<SmChannel> _smChannel;
		std::shared_ptr<std::list<NewDataParam> > _dataList;
		std::shared_ptr<SoraDbgPlot::Task::TaskQueue> _taskQueue;
		size_t _dataSize;
	};
}}
