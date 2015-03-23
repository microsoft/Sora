#pragma once

#include <wchar.h>
#include "sora.h"
//#include "_share_mem_if.h"
#include "ShareMemHelper.h"
#include "FixedSizeFifo.h"

class ShareMem;

class MemBufferBlock
{
	friend class BlockBufferAllocator;
private:
	// MemBlock in shared memroy
	struct _MemBlock
	{
		// first 4 bytes
		volatile size_t wSize;
		// second 4 bytes
		volatile size_t rSize;

		__int32 _userId;
		// the rest data
		//...
	};

public:
	bool WriteData(const char * buffer, size_t size, size_t & sizeWritten);
	void BeginWrite();
	void EndWrite();
	size_t FreeSize();
	bool ReadData(char * buffer, int size, int& sizeRead);
	void FinishReadData();
	bool IsFull();
	void TryReset();
	void Reset();
	_int32 Size();
	_int32 DataSize();
	int UserId();

private:
	MemBufferBlock();
	void Init(_MemBlock * ptr, int size);
	_MemBlock * _ptrBlock;
	size_t _sizeOfBlock;
	char * _ptrMemInBlock;
	size_t _sizeOfBufferInBlock;

	size_t _preWriteIdx;

	size_t _preReadidx;

private:	// Queue related
	MemBufferBlock * _prev;
	MemBufferBlock * _next;
};

typedef void (__stdcall *BlockBufferProcessFunc)(MemBufferBlock * memBlock, void * userData);

class BlockBufferAllocator
{
public:
	static BlockBufferAllocator * Open(const wchar_t * name, _int32 numOfBlock, _int32 sizeOfBlock, bool bRead);
	~BlockBufferAllocator();
	MemBufferBlock * AllocFreeMemBlock(__int32 userId);
	void ForEachInUseBlock(BlockBufferProcessFunc func, void * userData);
	void FreeMemBufferBlock(MemBufferBlock * block);
	int FreeBufferCount();
	int TotalBlockCount();
	int BlockSize();

private:

	struct _AllocatorHeader
	{
		_int32 _numOfBlock;
		_int32 _sizeOfBlock;
	};

	struct _SharedBufferInitStruct
	{
		_int32 sizeOfAllocatorHeader;
		_int32 sizeOfFreeFifo;
		_int32 sizeOfInUseFifo;
		_int32 sizeOfBlockBuffer;
		_int32 sizeOfBlock;
		_int32 numOfBlock;
		_int32 sizeOfFreeBlockCounter;
		BlockBufferAllocator * current;
	};

	static BOOL BlockBufferShareMemInit(ShareMem* sm, void* context);
	BlockBufferAllocator();
	MemBufferBlock * GetBlock(_int32 index);

private:
	MemBufferBlock * _memBufferBlocks;
	ShareMem * _sm;
	FixedSizeFifo * _freeBlockFifo;
	FixedSizeFifo * _inUseNotInQueueBlockFifo;

private:	// Queue Related
	MemBufferBlock * _listHead;
	MemBufferBlock * _listTail;

	char * _queueBitmap;

	void AddToList(MemBufferBlock * block);
	void RemoveFromList(MemBufferBlock * block);
	void ForEach(BlockBufferProcessFunc func, void * userData);

private:
	int _sizeInPrivateQueue;

private:
	int _blockSize;
	int _totalBlockCount;
	volatile long * _pFreeBlockCnt;
};
