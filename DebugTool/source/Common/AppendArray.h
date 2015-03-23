#pragma once
#include <vector>
#include <deque>

using namespace std;

class AppendArray
{
public:
	AppendArray();
	~AppendArray();
	void Add(char *);
	char * GetPtr(size_t index);
	size_t RecordCount();
	size_t DataSize();
private:
	static const size_t MIN_ALLOC_COUNT = 1024;
	size_t allocCnt;
	vector<char *> buffer;
	int currentBufIdx;
	size_t currentCapacity;
	size_t currentFreeSize;
	char * currentPtrInBuf;

	deque<char *> indexList;

	size_t dataSize;
};
