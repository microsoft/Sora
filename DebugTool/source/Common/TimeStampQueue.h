#pragma once

#include <deque>
#include <algorithm>

struct DataRecord
{
	size_t _start;
	size_t _size;
	unsigned long long _timestamp;
};

class TimeStampQueue
{
public:
	TimeStampQueue(size_t boundaryValue);

	bool Search(size_t index, unsigned long long & timestamp);

	bool SearchNearestTimeStamp(unsigned long long in, unsigned long long & out, size_t & outIdx);

	void AddRecord(const DataRecord & record);

	void Reset();

	size_t Size();

	bool GetDataRecord(size_t idx, DataRecord & record);

private:
	std::deque<DataRecord> _records;
	size_t _recordBoundary;

	bool Search(size_t index, size_t & outIndex);
};

