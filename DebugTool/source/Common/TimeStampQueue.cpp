#include "TimeStampQueue.h"

TimeStampQueue::TimeStampQueue(size_t boundaryValue) {
	_recordBoundary = boundaryValue;
}

bool TimeStampQueue::Search(size_t index, unsigned long long & timestamp)
{
	size_t resIndex;
	bool found = Search(index, resIndex);
	if (found)
	{
		timestamp = _records[resIndex]._timestamp;
		return true;
	}

	return false;
}


bool TimeStampQueue::SearchNearestTimeStamp(unsigned long long in, unsigned long long & out, size_t & outIdx)
{
	if (_records.size() == 0)
		return false;

	DataRecord baseRecord = _records[0];

	// binary search
	size_t begin = 0;
	size_t end = _records.size();
	size_t middle;

	while(end - begin > 1)
	{
		middle = (end + begin) / 2;
		DataRecord record = _records[middle];		
		if (record._timestamp == in)
		{
			out = in;
			outIdx = _records.size() - 1 - middle;
			return true;
		}
		else if (record._timestamp < in)
		{
			begin = middle;
		}
		else
		{
			end = middle;
		}
	}

	middle = begin;
	DataRecord record = _records[middle];
	out = in;
	outIdx = _records.size() - 1 - middle;
	if (record._timestamp < in)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void TimeStampQueue::AddRecord(const DataRecord & record)
{
	size_t resIndex;
	bool found = Search( record._start + record._size - 1, resIndex);
	if (found)
	{
		size_t newStart = record._start + record._size;
		auto iter = _records.begin() + resIndex;

		size_t dist = newStart - iter->_start;
		if (dist < 0)
			dist += _recordBoundary;
		else if (dist > _recordBoundary)
			dist -= _recordBoundary;

		if (dist < iter->_size)	// modify the record
		{
			iter->_start = newStart % _recordBoundary;
			iter->_size -= dist;
			_records.erase(_records.begin(), _records.begin() + resIndex);
		}
		else										// discard the record
		{
			_records.erase(_records.begin(), _records.begin() + resIndex + 1);
		}
	}
	_records.push_back(record);
}

void TimeStampQueue::Reset()
{
	_records.clear();
}

bool TimeStampQueue::GetDataRecord(size_t idx, DataRecord & record)
{
	size_t size = Size();

	if (idx >= size)
		return false;

	record = _records[size - idx - 1];

	return true;
}

size_t TimeStampQueue::Size()
{
	return _records.size();
}

bool TimeStampQueue::Search(size_t index, size_t & outIndex)
{
	if (_records.size() == 0)
		return false;

	DataRecord baseRecord = _records[0];

	// binary search
	size_t begin = 0;
	size_t end = _records.size();
	if (index < baseRecord._start)
		index += _recordBoundary;
	if (index > baseRecord._start + _recordBoundary)
		index -= _recordBoundary;

	while(end - begin > 1)
	{
		size_t middle = (begin + end) / 2;
		DataRecord record = _records[middle];
		if (record._start < baseRecord._start)
			record._start += _recordBoundary;

		if (index < record._start)
		{
			end = middle;
		}
		else if (index >= record._start + record._size)
		{
			begin = middle;
		}
		else	// find it
		{
			outIndex = middle;
			return true;
		}
	}

	DataRecord beginRecord = _records[begin];

	if (beginRecord._start <= index &&
		beginRecord._start + beginRecord._size > index)
	{
		outIndex = begin;
		return true;
	}

	return false;
}
