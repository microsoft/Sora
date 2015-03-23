#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include "_share_mem_if.h"
#include "DpSharedMemAllocator.h"
#include "DpFixedSizeSharedMemAllocator.h"


struct BufferControlBlock
{
	volatile int freeCount;
	int nextFree;
};

struct MemControlBlock
{
	int offsetNext;
};

struct ControlContent
{
	int freeCount;
	int head;
};

struct ControlHeader
{
	volatile int bDirty;
	ControlContent content;
};

//*****************
//
// allocator impl
//
//*****************

struct AllocatorImpl
{
	ShareMem * _sm;
	char * _startAddr;
	BufferControlBlock * _controlBlock;
	BufferControlBlock * _controlBlockRollback;
	int _blockCount;
	int _blockSize;
	int _blockPlusControlSize;
};

BOOL BufferInitFunc(ShareMem* sm, void* context);
void * Ptr(AllocatorImpl * allocator, int offset);
int Offset(AllocatorImpl *allocator, void * ptr);
bool Allocate(AllocatorImpl * allocator, void ** buf, int count);
void Free(AllocatorImpl * allocator, void * ptr);
void PrepareRollback(AllocatorImpl * allocator);


DpFixedSizeSharedMemAllocator * DpFixedSizeSharedMemAllocator::Create(const wchar_t * name, int blockSize, int count)
{
	DpFixedSizeSharedMemAllocator * allocator = new DpFixedSizeSharedMemAllocator(blockSize, count);
	int totolSize = 
		sizeof(BufferControlBlock) * 2 +
		(sizeof(MemControlBlock) + blockSize) * count
		;

	ShareMem * sm = DpShareMemAllocator::Create(name, totolSize, BufferInitFunc, (void *)allocator->_allocator);

	allocator->_allocator->_sm = sm;
	allocator->_allocator->_startAddr = (char *)sm->GetAddress();
	allocator->_allocator->_controlBlock = (BufferControlBlock *)(allocator->_allocator->_startAddr);
	allocator->_allocator->_controlBlockRollback = allocator->_allocator->_controlBlock + 1;

	return allocator;
}

void DpFixedSizeSharedMemAllocator::Release(DpFixedSizeSharedMemAllocator * allocator)
{
	delete allocator;
}

DpFixedSizeSharedMemAllocator::DpFixedSizeSharedMemAllocator(int blockSize, int count)
{
	_allocator = new AllocatorImpl();
	_allocator->_blockCount = count;
	_allocator->_blockSize = blockSize;
	_allocator->_blockPlusControlSize = blockSize + sizeof(MemControlBlock);
}

DpFixedSizeSharedMemAllocator::~DpFixedSizeSharedMemAllocator()
{
	ShareMem::FreeShareMem(_allocator->_sm);
	delete _allocator;
}

void * DpFixedSizeSharedMemAllocator::Ptr(int offset)
{
	return ::Ptr(_allocator, offset);
}

int DpFixedSizeSharedMemAllocator::Offset(void * ptr)
{
	return ::Offset(_allocator, ptr);
}

bool DpFixedSizeSharedMemAllocator::Allocate(void ** buf, unsigned count)
{
	bool succ = ::Allocate(_allocator, buf, count);

	//if (!succ)
	//	printf("allocation failed, %d wanted, %d remaining\n", count, _allocator->_controlBlock->freeCount);

	return succ;
}

void DpFixedSizeSharedMemAllocator::Free(void * ptr)
{
	::Free(_allocator, ptr);
}

void DpFixedSizeSharedMemAllocator::PrepareRollBack()
{
	::PrepareRollback(_allocator);
}

void DpFixedSizeSharedMemAllocator::RollBack()
{
	_allocator->_controlBlock->freeCount = _allocator->_controlBlockRollback->freeCount;
	_allocator->_controlBlock->nextFree = _allocator->_controlBlockRollback->nextFree;
	assert(_allocator->_controlBlock->freeCount <= _allocator->_blockCount);
}

int DpFixedSizeSharedMemAllocator::BlockDataCapacity()
{
	return _allocator->_blockSize;
}

int DpFixedSizeSharedMemAllocator::FreeCount()
{
	assert(_allocator->_controlBlock->freeCount <= _allocator->_blockCount);
	return _allocator->_controlBlock->freeCount;
}

static BOOL BufferInitFunc(ShareMem* sm, void* context)
{
	AllocatorImpl * allocator = (AllocatorImpl *)context;

	// init freelist
	BufferControlBlock * bufferControlblock = (BufferControlBlock *)sm->GetAddress();
	BufferControlBlock * bufferControlblockRollback = bufferControlblock + 1;

	MemControlBlock * memBlockBase = (MemControlBlock *)(bufferControlblockRollback + 1);
	MemControlBlock * block = memBlockBase;

	for (int i = 1; i < allocator->_blockCount; ++i)
	{
		MemControlBlock * nextBlock = (MemControlBlock *)((char *)memBlockBase + i * allocator->_blockPlusControlSize);
		block->offsetNext = (char *)nextBlock - (char *)sm->GetAddress();
		block = nextBlock;
	}

	block->offsetNext = -1;

	bufferControlblock->nextFree = (char *)memBlockBase - (char *)sm->GetAddress();
	bufferControlblock->freeCount = allocator->_blockCount;
	assert(bufferControlblock->freeCount <= allocator->_blockCount);

	return TRUE;
}

static void * Ptr(AllocatorImpl * allocator, int offset)
{
	return allocator->_startAddr + offset;
}

static int Offset(AllocatorImpl *allocator, void * ptr)
{
	return (char *)ptr - allocator->_startAddr;
}

static bool Allocate(AllocatorImpl * allocator, void ** buf, int count)
{
	if (count <= 0)
		return false;

	if (allocator->_controlBlock->freeCount < count)
	{
		//printf("Allocation failed, free count: %d\n", allocator->_controlBlock->freeCount);
		return false;
	}

	int offset = allocator->_controlBlock->nextFree;
	for (int i = 0; i < count; ++i)
	{
		MemControlBlock * block = (MemControlBlock *)(allocator->_startAddr + offset);
		buf[i] = (char *)(block + 1);
		offset = block->offsetNext;
	}

	int offsetTaken = allocator->_controlBlock->nextFree;
	allocator->_controlBlock->nextFree = offset;
	allocator->_controlBlock->freeCount -= count;
	assert(allocator->_controlBlock->freeCount <= allocator->_blockCount);

	return true;
}

static void Free(AllocatorImpl * allocator, void * ptr)
{
	MemControlBlock * block = (MemControlBlock *)((char *)ptr - sizeof(MemControlBlock));
	unsigned offset = ::Offset(allocator, block);

	block->offsetNext = allocator->_controlBlock->nextFree;
	allocator->_controlBlock->nextFree = offset;
	allocator->_controlBlock->freeCount++;
	assert(allocator->_controlBlock->freeCount <= allocator->_blockCount);
	//printf("Free block count: %d\n", allocator->_controlBlock->freeCount);
}

static void PrepareRollback(AllocatorImpl * allocator)
{
	allocator->_controlBlockRollback->freeCount = allocator->_controlBlock->freeCount;
	allocator->_controlBlockRollback->nextFree = allocator->_controlBlock->nextFree;
}
