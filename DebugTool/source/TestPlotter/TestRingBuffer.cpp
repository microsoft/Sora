#include <assert.h>
#include "RingBuffer.h"

#define TEST_BUF_SIZE 1024
int testBuffer[TEST_BUF_SIZE];

static void PrepareData()
{
	for (int i = 0; i < TEST_BUF_SIZE; i++)
	{
		testBuffer[i] = i;
	}
}

int TestRingBuffer()
{
	PrepareData();

	RingBuffer<int> ringBuffer(100);

	// test 1, test overwrite
	size_t numWritten = ringBuffer.Write(testBuffer, 1024);
	assert(ringBuffer.Capacity() == 100);
	assert(ringBuffer.Size() == 100);
	for (int i = 0; i < 100; i++)
	{
		assert(ringBuffer[i] == testBuffer[numWritten - 1 - i]);
	}

	// test 2, test partial write
	ringBuffer.Reset();
	assert(ringBuffer.Capacity() == 100);
	assert(ringBuffer.Size() == 0);	
	numWritten = ringBuffer.Write(testBuffer, 10);
	numWritten = ringBuffer.Write(testBuffer + 10, 10);
	numWritten = ringBuffer.Write(testBuffer + 20, 80);
	for (int i = 0; i < 100; i++)
	{
		assert(ringBuffer[i] == 99 - i);
	}

	assert(ringBuffer.WriteIndex() == 0);

	ringBuffer.Write(testBuffer, 10);
	assert(ringBuffer.WriteIndex() == 10);

	return 0;
};
