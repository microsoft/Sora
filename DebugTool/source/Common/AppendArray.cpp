#include <Windows.h>
#include <algorithm>
#include <math.h>
#include "AppendArray.h"

using namespace std;

inline size_t RoundUpToPowerOf2(size_t num32)
{
	num32--;
	num32 |= num32 >> 1;
	num32 |= num32 >> 2;
	num32 |= num32 >> 4;
	num32 |= num32 >> 8;
	num32 |= num32 >> 16;
#if _WIN64
	num32 |= num32 >> 32; 
#endif
	num32++;

	return num32;
}

AppendArray::AppendArray()
{
	dataSize = 0;
	currentCapacity = MIN_ALLOC_COUNT;
	currentBufIdx = 0;
	currentFreeSize = currentCapacity;
	buffer.push_back(new char[currentCapacity]);
	currentPtrInBuf = buffer[0];
}

AppendArray::~AppendArray()
{
	for_each(buffer.begin(), buffer.end(), [](char * ptr){
		delete [] ptr;
	});
}

void AppendArray::Add(char * str)
{
	size_t sizeNeeded = strlen(str) + 1;
	if (sizeNeeded > currentFreeSize)
	{
		size_t sizeNeededRounded = RoundUpToPowerOf2(sizeNeeded);
		size_t newSize = max(sizeNeededRounded, currentFreeSize * 2);
		char * newBuf = new char[newSize];
		buffer.push_back(newBuf);
		currentBufIdx++;
		currentCapacity = newSize;
		currentFreeSize = newSize;
		currentPtrInBuf = newBuf;
	}

	memcpy(currentPtrInBuf, str, sizeNeeded);
	indexList.push_back(currentPtrInBuf);
	currentPtrInBuf += sizeNeeded;
	currentFreeSize -= sizeNeeded;

	dataSize += sizeNeeded;
}

char * AppendArray::GetPtr(size_t index)
{
	if (index >= indexList.size())
		return 0;
	else
		return indexList[index];
}

size_t AppendArray::RecordCount()
{
	return indexList.size();
}

size_t AppendArray::DataSize()
{
	return dataSize;
}
