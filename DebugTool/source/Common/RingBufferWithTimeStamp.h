#pragma once

#include <functional>
#include "RingBuffer.h"
#include "TimeStampQueue.h"

template <typename T>
class RingBufferWithTimeStamp
{
public:
	RingBufferWithTimeStamp(size_t size);
	~RingBufferWithTimeStamp();
	size_t Write(const T * ptr, size_t count, unsigned long long timeStamp);
	bool GetTimeStampBySample(size_t index, unsigned long long & timestamp);
	bool GetNearestOldTimeStamp(unsigned long long in, unsigned long long & out, size_t & outIdx);
	size_t Size();
	size_t RecordCount();
	bool GetDataSizeByTimeStampIdx(size_t idx, size_t & out);
	bool ReadDataByTimeStampIdx(size_t idx, T * addr, size_t maxLen, size_t & sizeRead);

	T operator[] (size_t index);
	void Reset();

	typedef std::function<void(const T *, size_t)> ExportFunc;
	bool Export(const ExportFunc &);
	bool ExportRange(size_t start, size_t length, const ExportFunc &);

private:
	RingBuffer<T> * _ringBuffer;
	TimeStampQueue * _timeStampQueue;
};


template <typename T>
RingBufferWithTimeStamp<T>::RingBufferWithTimeStamp(size_t size)
{
	_ringBuffer = new RingBuffer<T>(size);
	_timeStampQueue = new TimeStampQueue(size);
}

template <typename T>
RingBufferWithTimeStamp<T>::~RingBufferWithTimeStamp()
{
	delete _ringBuffer;
	delete _timeStampQueue;
}

template <typename T>
size_t RingBufferWithTimeStamp<T>::Write(const T * ptr, size_t count, unsigned long long timeStamp)
{
	if (count < 0)
		return 0;

	size_t writeIdx1 = _ringBuffer->WriteIndex();
	size_t cntWritten = _ringBuffer->Write(ptr, count);
	size_t writeIdx2 = _ringBuffer->WriteIndex();
	DataRecord record;
	record._start = writeIdx1;
	record._size = (writeIdx2 - writeIdx1) % _ringBuffer->Capacity();

	record._timestamp = timeStamp;

	_timeStampQueue->AddRecord(record);

	return cntWritten;
}

template <typename T>
bool RingBufferWithTimeStamp<T>::GetTimeStampBySample(size_t index, unsigned long long & timestamp)
{
	size_t targetIndex = (_ringBuffer->WriteIndex() - 1 - index) % _ringBuffer->Capacity();
	return _timeStampQueue->Search(targetIndex, timestamp);
}

template <typename T>
bool RingBufferWithTimeStamp<T>::GetNearestOldTimeStamp(unsigned long long timestamp, unsigned long long & out, size_t & outIdx)
{
	return _timeStampQueue->SearchNearestTimeStamp(timestamp, out, outIdx);
}

template <typename T>
size_t RingBufferWithTimeStamp<T>::Size()
{
	return _ringBuffer->Size();
}


template <typename T>
size_t RingBufferWithTimeStamp<T>::RecordCount()
{
	return _timeStampQueue->Size();
}

template <typename T>
T RingBufferWithTimeStamp<T>::operator[] (size_t index)
{
	return (*_ringBuffer)[index];
}

template <typename T>
void RingBufferWithTimeStamp<T>::Reset()
{
	_ringBuffer->Reset();
	_timeStampQueue->Reset();
}

template <typename T>
bool RingBufferWithTimeStamp<T>::GetDataSizeByTimeStampIdx(size_t idx, size_t & out)
{
	DataRecord record;
	bool found = _timeStampQueue->GetDataRecord(idx, record);
	if (found)
	{
		out = record._size;
		return true;
	}

	return false;
}

template <typename T>
bool RingBufferWithTimeStamp<T>::ReadDataByTimeStampIdx(size_t idx, T * addr, size_t maxLen, size_t & sizeRead)
{
	DataRecord record;
	bool found = _timeStampQueue->GetDataRecord(idx, record);
	if (!found)
	{
		return false;
	}

	return this->_ringBuffer->Read(record._start, record._size, addr, maxLen, sizeRead);
}

template <typename T>
bool RingBufferWithTimeStamp<T>::Export(const ExportFunc & f)
{
	return _ringBuffer->Export(f);
}

template <typename T>
bool RingBufferWithTimeStamp<T>::ExportRange(size_t start, size_t length, const ExportFunc & f)
{
	return _ringBuffer->ExportRange(start, length, f);
}
