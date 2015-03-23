#pragma once

#include <vector>
#include <algorithm>
#include <memory.h>
#include "SeriesObj.h"
#include "IChannelBuffer.h"
#include "ChannelBufferImpl.h"
#include "Constant.h"

namespace DbgPlotViewer
{
	class ChannelManager
	{
	public:
		ChannelManager() {
			_channelBufferReadable = ChannelBuffer::OpenForRead(
				SoraDbgPlot::Common::GLOBAL_BUFFER_NAME,
				SoraDbgPlot::Common::GLOBAL_BUFFER_BLOCK_NUM,
				SoraDbgPlot::Common::GLOBAL_BUFFER_BLOCK_SIZE);
		}
		~ChannelManager() {
			delete _channelBufferReadable;
		}

		void UpdateAllChannels()
		{
			_channelBufferReadable->ReadData(ReadFunction, this);
		}

	private:
		static void __stdcall ReadFunction(const char * buffer, int size, void * userData, __int32 userId)
		{
			auto manager = (ChannelManager *)userData;
		}

		std::map<__int32, std::shared_ptr<SeriesProp *> > _channelsMap;
		std::vector<SeriesProp *> _channels;
		IChannelBufferReadable * _channelBufferReadable;

	};
};
