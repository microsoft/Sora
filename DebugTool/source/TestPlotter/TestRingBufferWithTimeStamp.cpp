#include <assert.h>
#include <deque>
#include "RingBufferWithTimeStamp.h"

using namespace std;

#define TEST_BUF_SIZE 1024
static int testBuffer[TEST_BUF_SIZE];

#define RING_BUF_SIZE 512;

static void PrepareData()
{
	for (int i = 0; i < TEST_BUF_SIZE; i++)
	{
		testBuffer[i] = i;
	}
}

static int Test1()
{
	RingBufferWithTimeStamp<int> buffer(512);

	// test 1
	int * ptr = testBuffer;

	buffer.Write(ptr, 256, 0);
	ptr += 256;

	buffer.Write(ptr, 128, 1);
	ptr += 128;

	buffer.Write(ptr, 128, 2);

	bool found = false;
	unsigned long long timestamp = 0;
	found = buffer.GetTimeStampBySample(0, timestamp);
	assert(found == true && timestamp == 2);

	found = buffer.GetTimeStampBySample(127, timestamp);
	assert(found == true && timestamp == 2);

	found = buffer.GetTimeStampBySample(128, timestamp);
	assert(found = true && timestamp == 1);

	found = buffer.GetTimeStampBySample(255, timestamp);
	assert(found = true && timestamp == 1);

	found = buffer.GetTimeStampBySample(256, timestamp);
	assert(found = true && timestamp == 0);

	found = buffer.GetTimeStampBySample(511, timestamp);
	assert(found = true && timestamp == 0);

	return 0;
}

template <typename T>
static void TrimDeq(deque<T> & deq, size_t size)
{
	if (deq.size() > size)
	{				
		auto iter = deq.begin() + deq.size() - size;
		deq.erase(deq.begin(), iter);
		assert(deq.size() == size);
	}
}

static int Test2(int round, size_t dataSize)
{
	RingBufferWithTimeStamp<int> buffer(dataSize);
	int * ptr = testBuffer;

	deque<int> deqData;
	deque<unsigned long long> deqTime;

	for (int i = 0; i < round; i++)
	{
		size_t count = rand() % dataSize;
		unsigned long long timestamp = rand();
		size_t cntWritten = buffer.Write(testBuffer, count, timestamp);
		for (size_t j = 0; j < cntWritten; j++)
		{
			deqData.push_back(testBuffer[j]);
			deqTime.push_back(timestamp);
		}

		TrimDeq(deqData, dataSize);
		TrimDeq(deqTime, dataSize);

		size_t actualSize = deqData.size();
		for (size_t j = 0; j < actualSize; j++)
		{
			assert(buffer[j] == deqData[actualSize-1-j]);
			unsigned long long timestamp;
			bool found = buffer.GetTimeStampBySample(j, timestamp);
			assert(found == true);
			assert(timestamp = deqTime[actualSize-1-j]);
		}
	}

	return 0;
}

int TestRingBufferWithTimeStamp()
{
	PrepareData();

	Test1();
	Test2(1000, 1024);

	return 0;
}
