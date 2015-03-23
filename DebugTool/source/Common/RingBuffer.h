#pragma once

#include <Windows.h>
#include <exception>
#include <functional>

template <typename T>
class RingBuffer
{
public:
	RingBuffer(size_t count);
	~RingBuffer();
	size_t Write(const T * ptr, size_t count);
	bool Read(size_t start, size_t len, T * addr, size_t maxLen, size_t & sizeRead);
	T operator[](size_t index);
	size_t Size();
	size_t Capacity();
	size_t WriteIndex();
	void Reset();

	typedef std::function<void(const T *, size_t)> ExportFunc;
	bool Export(const ExportFunc &);
	bool ExportRange(size_t start, size_t length, const ExportFunc &);

private:
	T * _buffer;
	T * _pWrite;
	size_t _dataCount;
	size_t _maxCount;
};


template <typename T>
RingBuffer<T>::RingBuffer(size_t count) {
	_maxCount = count;
	_buffer = new T[_maxCount];
	_pWrite = _buffer;
	_dataCount = 0;
}

template <typename T>
RingBuffer<T>::~RingBuffer()
{
	delete [] _buffer;
}

template <typename T>
size_t RingBuffer<T>::Write(const T * ptr, size_t count)
{
	if (count <= 0)
		return 0;

	size_t cntToWrite = min(count, _maxCount);
	_dataCount = min(_dataCount + cntToWrite, _maxCount);

	size_t retCnt = cntToWrite;

	size_t cntToBufEnd = _buffer + _maxCount - _pWrite;
	size_t count1 = min(cntToBufEnd, cntToWrite);
	memcpy(_pWrite, ptr, count1 * sizeof(T));

	cntToWrite -= count1;
	_pWrite += count1;
	if (_pWrite == _buffer + _maxCount)
		_pWrite = _buffer;

	if (cntToWrite > 0)
	{
		ptr = ptr + count1;
		memcpy(_pWrite, ptr, cntToWrite * sizeof(T));
		_pWrite += cntToWrite;
	}

	return retCnt;
}

template <typename T>
bool RingBuffer<T>::Read(size_t start, size_t len, T * addr, size_t maxLen, size_t & sizeRead)
{
	size_t sizeToRead = min(maxLen, len);
	sizeRead = sizeToRead;

	size_t s1 = (_maxCount - start);
	s1 = min(s1, len);
	memcpy(addr, _buffer + start, sizeof(T) * s1);
	addr += s1;
	sizeToRead -= s1;
	if (sizeToRead > 0)
	{
		size_t s2 = sizeToRead;
		memcpy(addr, _buffer, sizeof(T) * s2);
	}

	return true;
}

template <typename T>
T RingBuffer<T>::operator[](size_t index) {
	if (index < 0 || index >= _dataCount)
		throw std::exception("index out of range");

	T * ptr = _pWrite - 1 - index;
	if (ptr < _buffer)
		ptr += _maxCount;

	return *ptr;
}

template <typename T>
size_t RingBuffer<T>::Size()
{
	return _dataCount;
}

template <typename T>
size_t RingBuffer<T>::Capacity()
{
	return _maxCount;
}

template <typename T>
size_t RingBuffer<T>::WriteIndex()
{
	return _pWrite - _buffer;
}

template <typename T>
void RingBuffer<T>::Reset()
{
	_pWrite = _buffer;
	_dataCount = 0;
}

template <typename T>
bool RingBuffer<T>::Export(const ExportFunc & f)
{
	return ExportRange(0, _dataCount, f);
}

template <typename T>
bool RingBuffer<T>::ExportRange(size_t start, size_t length, const ExportFunc & f)
{
	int lengthToProcess = length;

	if (lengthToProcess <= 0)
		return false;

	int indexStart = _pWrite - _buffer - start - lengthToProcess;
	if (indexStart < 0)
	{
		indexStart += _maxCount;
		int length1 = _maxCount - indexStart;	// till end of buffer
		f(_buffer + indexStart, length1); 
		indexStart += length1;
		indexStart = indexStart % _maxCount;
		lengthToProcess -= length1;
	}

	if (lengthToProcess <= 0)
		return true;

	f(_buffer + indexStart, lengthToProcess);

	return true;
}
