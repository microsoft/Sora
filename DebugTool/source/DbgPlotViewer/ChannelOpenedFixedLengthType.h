#pragma once

#include <memory>
#include "ChannelOpened.h"
#include "RingBufferWithTimeStamp.h"
#include "FrameWithSizeFilter.h"
#include "TaskSimple.h"
#include "AppSettings.h"

template <typename T>
class ChannelOpenedFixedLengthType : public ChannelOpened
{
protected:
	virtual void WriteData(const char * data, size_t length);
	virtual void ClearData();

public:
	ChannelOpenedFixedLengthType();
	~ChannelOpenedFixedLengthType();

	bool IndexToTimestamp(size_t index, unsigned long long & timestamp);
	bool Data(size_t index, T & data);
	size_t DataSize();
	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskUpdateDataSize(std::shared_ptr<size_t>);

protected:
	SoraDbgPlot::FrameWithSizeInfoWriter<T> * _filter;
	RingBufferWithTimeStamp<T> * _ringBuffer;
	size_t _latestIndex;
	size_t _range;
};

template <typename T>
void ChannelOpenedFixedLengthType<T>::WriteData(const char * data, size_t length)
{
	auto SThis = std::dynamic_pointer_cast<ChannelOpenedFixedLengthType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, data, length](){
		size_t dummy;
		SThis->_filter->Write(data, length, dummy);
		delete [] data;
	}, [data](){
		delete [] data;
	});
}

template <typename T>
ChannelOpenedFixedLengthType<T>::ChannelOpenedFixedLengthType()
{
	_ringBuffer = new RingBufferWithTimeStamp<T>(::SettingGetReplayBufferSize() / sizeof(T));
	_filter = new SoraDbgPlot::FrameWithSizeInfoWriter<T>(_ringBuffer);
	_latestIndex = 0;
	_range = 0;
}

template <typename T>
ChannelOpenedFixedLengthType<T>::~ChannelOpenedFixedLengthType()
{
	delete _filter;
	delete _ringBuffer;
}

template <typename T>
bool ChannelOpenedFixedLengthType<T>::Data(size_t index, T & data)
{
	data = (*_ringBuffer)[index];
	return true;
}

template <typename T>
size_t ChannelOpenedFixedLengthType<T>::DataSize()
{
	return _ringBuffer->Size();
}

template <typename T>
bool ChannelOpenedFixedLengthType<T>::IndexToTimestamp(size_t index, unsigned long long & timestamp)
{
	return _ringBuffer->GetTimeStampBySample(index, timestamp);
}

template <typename T>
std::shared_ptr<SoraDbgPlot::Task::TaskSimple> ChannelOpenedFixedLengthType<T>::TaskUpdateDataSize(std::shared_ptr<size_t> size)
{
	auto SThis = std::dynamic_pointer_cast<ChannelOpenedFixedLengthType, AsyncObject>(shared_from_this());
	return std::make_shared<SoraDbgPlot::Task::TaskSimple>(TaskQueue(), [SThis, size](){
		size_t oldSize = *size;
		size_t newSize = max(oldSize, SThis->_ringBuffer->Size());
		*size = newSize;
	});
}

template <typename T>
void ChannelOpenedFixedLengthType<T>::ClearData()
{
	_ringBuffer->Reset();
}
