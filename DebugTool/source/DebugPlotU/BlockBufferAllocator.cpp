#include "sora.h"
//#include "_share_mem_if.h"
#include "ShareMemHelper.h"
#include "BlockBufferAllocator.h"
#include "ShareMemHelper.h"

MemBufferBlock::MemBufferBlock()
{}

void MemBufferBlock::Init(_MemBlock * ptr, int size) {
	_ptrBlock = ptr;
	_sizeOfBlock = size;

	int offset = sizeof(_MemBlock);

	_ptrMemInBlock = (char *)ptr + offset;
	_sizeOfBufferInBlock = _sizeOfBlock - offset;
}

bool MemBufferBlock::WriteData(const char * buffer, size_t size, size_t & sizeWritten) {

	// reset all block to reuse if wSize == rSize

	size_t sizeToWrite = min(size, _sizeOfBufferInBlock - _preWriteIdx);

	if (sizeToWrite > 0)
	{
		char * wPtr = _ptrMemInBlock + _preWriteIdx;
		memcpy(wPtr, buffer, sizeToWrite);
		_preWriteIdx += sizeToWrite;
		size_t playWithDebugger = _preWriteIdx;
		sizeWritten = sizeToWrite;
	}

	return false;
}

void MemBufferBlock::BeginWrite()
{
	_preWriteIdx = _ptrBlock->wSize;
}

void MemBufferBlock::EndWrite()
{
	_ptrBlock->wSize = _preWriteIdx;
}

size_t MemBufferBlock::FreeSize()
{
	return _sizeOfBufferInBlock - _ptrBlock->wSize;
}

bool MemBufferBlock::ReadData(char * buffer, int size, int& sizeRead) {
	_int32 sizeToRead = _ptrBlock->wSize - _ptrBlock->rSize;
	sizeToRead = min(size, sizeToRead);
	if (sizeToRead > 0)
	{
		memcpy(buffer, _ptrMemInBlock + _ptrBlock->rSize, sizeToRead);
		_int32 rSizeAfterRead = _ptrBlock->rSize + sizeToRead;
		_ptrBlock->rSize = rSizeAfterRead;
		_preReadidx = rSizeAfterRead;
		sizeRead = sizeToRead;
		if (rSizeAfterRead == _sizeOfBufferInBlock)
			return true;
	}
	else
		sizeRead = 0;

	return false;
}

void MemBufferBlock::FinishReadData()
{
	_ptrBlock->rSize = _preReadidx;
}

bool MemBufferBlock::IsFull()
{
	return
		(_ptrBlock->rSize == _sizeOfBufferInBlock) &&
		(_ptrBlock->wSize == _sizeOfBufferInBlock);
}

void MemBufferBlock::TryReset()
{
	if (_ptrBlock->wSize == _ptrBlock->rSize)
	{
		if (_ptrBlock->wSize < _sizeOfBufferInBlock)
			Reset();
	}
}

void MemBufferBlock::Reset()
{
	// the order is important here!!
	_ptrBlock->wSize = 0;	
	_ptrBlock->rSize = 0;
	_preWriteIdx = 0;
}

_int32 MemBufferBlock::Size()
{
	return _sizeOfBlock;
}

_int32 MemBufferBlock::DataSize()
{
	return _ptrBlock->wSize - _ptrBlock->rSize;
}

int MemBufferBlock::UserId()
{
	return _ptrBlock->_userId;
}

BlockBufferAllocator * BlockBufferAllocator::Open(const wchar_t * name, _int32 numOfBlock, _int32 sizeOfBlock, bool bRead)
{
	if (numOfBlock <= 0)
		return 0;

	if (sizeOfBlock <= sizeof(MemBufferBlock::_MemBlock))
		return 0;

	_SharedBufferInitStruct sbis;

	sbis.sizeOfAllocatorHeader = sizeof(_AllocatorHeader);
	sbis.sizeOfFreeFifo = FixedSizeFifo::CalcSharedBufferSize(numOfBlock);
	sbis.sizeOfInUseFifo = FixedSizeFifo::CalcSharedBufferSize(numOfBlock);
	sbis.sizeOfBlockBuffer = numOfBlock * sizeOfBlock;
	sbis.numOfBlock = numOfBlock;
	sbis.sizeOfBlock = sizeOfBlock;
	sbis.sizeOfFreeBlockCounter = sizeof(long) * 2;

	BlockBufferAllocator * allocator = new BlockBufferAllocator();
	sbis.current = allocator;

	_int32 sizeInTotal = 
		sbis.sizeOfAllocatorHeader +
		sbis.sizeOfBlockBuffer + 
		sbis.sizeOfFreeFifo + 
		sbis.sizeOfInUseFifo +
		sbis.sizeOfFreeBlockCounter;

	assert( (sbis.sizeOfAllocatorHeader +
		sbis.sizeOfBlockBuffer + 
		sbis.sizeOfFreeFifo + 
		sbis.sizeOfInUseFifo) % 4 == 0);

	ShareMem * sm = AllocateSharedMem(
			name,
			sizeInTotal,
			BlockBufferShareMemInit, &sbis);

	if (sm == 0)
		return 0;

	allocator->_sm = sm;
	char * smPtr = (char *)sm->GetAddress();
	
	// Get header info, may be different from parameters
	_AllocatorHeader * header = (_AllocatorHeader *)smPtr;
	smPtr += sizeof(_AllocatorHeader);

	// Create MemBufferBlock objs
	allocator->_memBufferBlocks = new MemBufferBlock[header->_numOfBlock];
	for (int i = 0; i < header->_numOfBlock; i++)
	{
		allocator->_memBufferBlocks[i].Init((MemBufferBlock::_MemBlock *)smPtr, header->_sizeOfBlock);
		smPtr += header->_sizeOfBlock;
	}

	// Create free block fifo
	__int32 sizeForFreeBlockInfoSM = FixedSizeFifo::CalcSharedBufferSize(header->_numOfBlock);
	allocator->_freeBlockFifo = FixedSizeFifo::Create(
		smPtr, 
		sizeForFreeBlockInfoSM
		);
	if (allocator->_freeBlockFifo == 0)
		goto ERROR_EXIT;
	smPtr += sizeForFreeBlockInfoSM;

	// create in use block fifo
	__int32 sizeForInUseBlockInfoSM = FixedSizeFifo::CalcSharedBufferSize(header->_numOfBlock);
	allocator->_inUseNotInQueueBlockFifo = FixedSizeFifo::Create(
		smPtr, 
		sizeForInUseBlockInfoSM
		);
	if (allocator->_inUseNotInQueueBlockFifo == 0)
		goto ERROR_EXIT;
	smPtr += sizeForInUseBlockInfoSM;

	allocator->_pFreeBlockCnt = (volatile long *)smPtr;

	allocator->_queueBitmap = new char[header->_numOfBlock];
	memset(allocator->_queueBitmap, 0, header->_numOfBlock*sizeof(char));

	// Gabage collection
	if (bRead) {
		int bitmapSize = header->_numOfBlock;
		char * bitmap = new char[bitmapSize];
		for (int i = 0; i < bitmapSize; i++)
			bitmap[i] = 0;

		allocator->_freeBlockFifo->FillBitmap(bitmap, 1);
		allocator->_inUseNotInQueueBlockFifo->FillBitmap(bitmap, 2);

		for (int i = 0; i < bitmapSize; i++)
		{
			if (bitmap[i] == 0)
			{
				MemBufferBlock * block = allocator->GetBlock(i);
				allocator->AddToList(block);
			}
		}

		delete [] bitmap;
	}

	allocator->_totalBlockCount = header->_numOfBlock;
	allocator->_blockSize = sizeOfBlock;

	return allocator;

ERROR_EXIT:
	delete allocator;
	return 0;
}

BOOL BlockBufferAllocator::BlockBufferShareMemInit(ShareMem* sm, void* context)
{
	_SharedBufferInitStruct * sbis = (_SharedBufferInitStruct *)context;
	BlockBufferAllocator * allocator = sbis->current;

	char * smAddr = (char *)sm->GetAddress();

	_AllocatorHeader * header = (_AllocatorHeader *)smAddr;
	header->_numOfBlock = sbis->numOfBlock;
	header->_sizeOfBlock = sbis->sizeOfBlock;
	smAddr += sizeof(_AllocatorHeader);

	for (int i = 0; i < sbis->numOfBlock; i++)
	{
		MemBufferBlock::_MemBlock * blockInSM = (MemBufferBlock::_MemBlock *)smAddr;
		blockInSM->rSize = blockInSM->wSize = 0;
		smAddr += sbis->sizeOfBlock;
	}
	
	FixedSizeFifo::InitShareMem(smAddr, sbis->sizeOfFreeFifo);
	// init free list
	FixedSizeFifo * tempFifo = FixedSizeFifo::Create(smAddr, sbis->sizeOfFreeFifo);
	int idxFifo;
	for (idxFifo = 0;;idxFifo++)
	{
		bool succ = tempFifo->Put(idxFifo);
		if (!succ)
			break;
	}
	delete tempFifo;

	smAddr += sbis->sizeOfFreeFifo;

	FixedSizeFifo::InitShareMem(smAddr, sbis->sizeOfInUseFifo);

	smAddr += sbis->sizeOfInUseFifo;

	*(long *)smAddr = idxFifo;

	return TRUE;
}

BlockBufferAllocator::BlockBufferAllocator() :
	_memBufferBlocks(0),
	_sm(0),
	_freeBlockFifo(0),
	_inUseNotInQueueBlockFifo(0),
	_listHead(0),
	_listTail(0),
	_sizeInPrivateQueue(0)
{}

BlockBufferAllocator::~BlockBufferAllocator()
{
	if (_memBufferBlocks)
		delete [] _memBufferBlocks;

	if (_freeBlockFifo)
		delete _freeBlockFifo;

	if (_inUseNotInQueueBlockFifo)
		delete _inUseNotInQueueBlockFifo;

	if (_queueBitmap)
		delete [] _queueBitmap;

	if (_sm)
		ShareMem::FreeShareMem(_sm);
}

MemBufferBlock * BlockBufferAllocator::AllocFreeMemBlock(__int32 userId)
{
	_int32 index;
	bool succ = _freeBlockFifo->Get(index);
	if (succ)
	{
		MemBufferBlock * block = _memBufferBlocks + index;
		block->_ptrBlock->_userId = userId;

		_inUseNotInQueueBlockFifo->Put(index);

		::InterlockedDecrement(_pFreeBlockCnt);

		return _memBufferBlocks + index;
	}

	return 0;
}

void BlockBufferAllocator::FreeMemBufferBlock(MemBufferBlock * block)
{
	_int32 index = block - _memBufferBlocks;

	block->Reset();

	_freeBlockFifo->Put(index);
	::InterlockedIncrement(_pFreeBlockCnt);
}

int BlockBufferAllocator::FreeBufferCount()
{
	return *_pFreeBlockCnt;
}

int BlockBufferAllocator::TotalBlockCount()
{
	return _totalBlockCount;
}

int BlockBufferAllocator::BlockSize()
{
	return _blockSize;
}

void BlockBufferAllocator::ForEachInUseBlock(BlockBufferProcessFunc func, void * userData)
{
	_int32 index;
	while(_inUseNotInQueueBlockFifo->Get(index))
	{
		MemBufferBlock * block = GetBlock(index);
		AddToList(block);
	}

	ForEach(func, userData);
}

MemBufferBlock * BlockBufferAllocator::GetBlock(_int32 index)
{
	return _memBufferBlocks + index;
}

void BlockBufferAllocator::AddToList(MemBufferBlock * block)
{
	size_t idx = block - _memBufferBlocks;
	assert(_queueBitmap[idx] == 0);

	if (_listHead == 0)
	{
		_listHead = _listTail = block;
		block->_prev = 0;
		block->_next = 0;
	}
	else
	{
		block->_prev = _listTail;
		block->_next = 0;
		_listTail->_next = block;
		_listTail = block;
	}

	_queueBitmap[idx] = 1;
	_sizeInPrivateQueue++;
}

void BlockBufferAllocator::RemoveFromList(MemBufferBlock * block)
{
	size_t idx = block - _memBufferBlocks;
	assert(_queueBitmap[idx] != 0);


	if (block->_prev == 0)	// head
		_listHead = block->_next;
	else
		block->_prev->_next = block->_next;

	if (block->_next == 0)
		_listTail = block->_prev;
	else
		block->_next->_prev = block->_prev;

	_queueBitmap[idx] = 0;
	_sizeInPrivateQueue--;
}

void BlockBufferAllocator::ForEach(BlockBufferProcessFunc func, void * userData)
{
	MemBufferBlock * block = _listHead;
	while(block != 0)
	{

		func(block, userData);
		MemBufferBlock * nextBlock = block->_next;

		if (block->IsFull())
		{
			RemoveFromList(block);
			FreeMemBufferBlock(block);
		}

		block = nextBlock;
	}
}
