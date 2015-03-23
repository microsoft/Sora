#include <stdio.h>
#include "DpFixedSizeSharedMemAllocator.h"
#include "Channel.h"
#include "RollbackManager.h"
#include "ChannelTableStruct.h"
#include "Config.h"

using namespace SharedChannelTable;

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct ChannelImpl
{
	ChannelTableItem * _pShareChannel;
	DpFixedSizeSharedMemAllocator * _allocator;
	MemRollbackManager * _rollbackManager;
	int _lastGcTickCount;
	int _id;
};

void Rollback_Mem(ChannelImpl * channel);
void PreOperation_Mem(ChannelImpl * channel);
bool Alloc(ChannelImpl * channel, int size);
void Free(ChannelImpl * channel);
void Rollback_Mem(ChannelImpl * channel);
void PrepareRollback(ChannelImpl * channel);
void PreOperation_RW(ChannelImpl * channel);
void PostOperation_RW(ChannelImpl * channel);
void Rollback_RW(ChannelImpl * channel);
void PostOperation_Mem(ChannelImpl * channel);
void Peek(ChannelImpl * channel, char * buf, int length, int * pLengthPeeked, int * pReadOffset, int * pReadBlockOffset);

Channel::Channel(ChannelTableItem * shareChannel, DpFixedSizeSharedMemAllocator * allocator, MemRollbackManager * rollbackManager)
{
	_channel = new ChannelImpl();
	_channel->_pShareChannel = shareChannel;
	_channel->_allocator = allocator;
	_channel->_rollbackManager = rollbackManager;
	_channel->_lastGcTickCount = 0;
}

Channel::~Channel()
{
	delete _channel;
}

bool Channel::Write(const char * buf, int length)
{
	// rollback memory operation
	::Lock_Mem(_channel->_rollbackManager);
	::Rollback_Mem(_channel);
	::Unlock_Mem(_channel->_rollbackManager);

	// rollback write operation
	::Rollback_RW(_channel);

	// op
	const char * srcPtr = buf;
	int lenToCopy = length;
	int writeBlockOffset = _channel->_pShareChannel->channelRWInfo.writeBlockOffset;
	int offsetInBlock = _channel->_pShareChannel->channelRWInfo.writeOffset;
	
	int sizeLeft = writeBlockOffset != -1 ?
		_channel->_allocator->BlockDataCapacity() - sizeof(ChannelHead) - offsetInBlock : 0;

	// alloc if not enough memory
	if (sizeLeft < length)
	{
		if (!Alloc(_channel, length - sizeLeft))
			return false;

		if (writeBlockOffset == -1)
			writeBlockOffset = _channel->_pShareChannel->channelMemInfo.firstBlockOffset;
	}


	sizeLeft = _channel->_allocator->BlockDataCapacity() - sizeof(ChannelHead) - offsetInBlock;

	while (lenToCopy > 0)
	{
		if (sizeLeft == 0)
		{
			ChannelHead * head = (ChannelHead *)_channel->_allocator->Ptr(writeBlockOffset);
			writeBlockOffset = head->nextOffset;
			offsetInBlock = 0;
			sizeLeft = _channel->_allocator->BlockDataCapacity() - sizeof(ChannelHead) - offsetInBlock;
		}

		char * chPtr = (char *)_channel->_allocator->Ptr(writeBlockOffset) + offsetInBlock + sizeof(ChannelHead);
		
		int lenCopy = MIN(lenToCopy, sizeLeft);

		memcpy(chPtr, srcPtr, lenCopy);

		srcPtr += lenCopy;
		lenToCopy -= lenCopy;
		offsetInBlock += lenCopy;
		sizeLeft -= lenCopy;
	}
	
	PreOperation_RW(_channel);
	_channel->_pShareChannel->channelRWInfo.writeBlockOffset = writeBlockOffset;
	_channel->_pShareChannel->channelRWInfo.writeOffset = offsetInBlock;
	_channel->_pShareChannel->channelRWInfo.readableSize += length;
	PostOperation_RW(_channel);

	return true;
}

int Channel::Read(char * buf, int length)
{
	int lengthRead, readOffset, readBlockOffset;
	::Peek(_channel, buf, length, &lengthRead, &readOffset, &readBlockOffset);

	// store
	PreOperation_RW(_channel);
	_channel->_pShareChannel->channelRWInfo.readableSize -= lengthRead;
	_channel->_pShareChannel->channelRWInfo.readBlockOffset = readBlockOffset;
	_channel->_pShareChannel->channelRWInfo.readOffset = readOffset;
	PostOperation_RW(_channel);

	Free(_channel);

	return lengthRead;
}

int Channel::Peek(char * buf, int length)
{
	int lengthPeeked;
	::Peek(_channel, buf, length, &lengthPeeked, 0, 0);
	return lengthPeeked;
}

unsigned Channel::Size()
{
	int size = 0;
	
	PreOperation_RW(_channel);
	size = _channel->_pShareChannel->channelRWInfo.readableSize;
	PostOperation_RW(_channel);

	return size;
}

int Channel::Type()
{
	return this->_channel->_pShareChannel->channelInfo.type;
}

bool Channel::IsWriterAlive()
{
	SharedChannelTable::PreTableOperation(_channel->_pShareChannel);
	int pidPlotter = _channel->_pShareChannel->channelInfo.pidPlotter;
	SharedChannelTable::PostTableOperation(_channel->_pShareChannel);

	if (pidPlotter != 0)
	{
		int tickCount = ::GetTickCount();
		if (tickCount - _channel->_lastGcTickCount > 10)
		{
			GcSession * session = new GcSession();
			
			GC_Channel(_channel->_pShareChannel, _channel->_rollbackManager, _channel->_allocator, session);
			delete session;
			_channel->_lastGcTickCount = tickCount;

			SharedChannelTable::PreTableOperation(_channel->_pShareChannel);
			int pidPlotter = _channel->_pShareChannel->channelInfo.pidPlotter;
			SharedChannelTable::PostTableOperation(_channel->_pShareChannel);
		}
	}

	return pidPlotter != 0;
}

void Channel::SetBufferLimit(int size)
{
	PreOperation_Mem(_channel);

	_channel->_pShareChannel->channelMemInfo.sizeLimit = size;

	PostOperation_Mem(_channel);
}

void Channel::Close()
{
	
}

SharedChannelTable::ChannelTableItem * Channel::ShareStruct()
{
	return _channel->_pShareChannel;
}

static void PreOperation_Mem(ChannelImpl * channel)
{
	::Lock_Mem(channel->_rollbackManager);

	Rollback_Mem(channel);
	
	SetClean_Mem(channel->_rollbackManager);

	PrepareRollback_Mem(channel->_rollbackManager, channel->_pShareChannel);

	SetDirty_Mem(channel->_rollbackManager);
}

static bool Alloc(ChannelImpl * channel, int size)
{
	void ** buffer = 0;

	bool ret = true;

	PreOperation_Mem(channel);

	int sizeLimit = channel->_pShareChannel->channelMemInfo.sizeLimit;

	int blockCapacity = channel->_allocator->BlockDataCapacity() - sizeof(ChannelHead);

	int count = size / blockCapacity;
	if (count * blockCapacity < size)
		count++;

	if (channel->_pShareChannel->channelMemInfo.blockCount + count > 2)		// more than 2 blocks will be allocated, so we need to check whether we should alloc the memary
	{
		if (channel->_allocator->FreeCount() - count < (SharedChannelTable::MAX_CHANNEL - 1) * 2)	// leave at least 2 blocks for each channel
		{
			ret = false;
			goto EXIT;
		}
		else if ((sizeLimit != -1) && (channel->_pShareChannel->channelMemInfo.blockCount * blockCapacity >= sizeLimit))	// allocate too much memory
		{
			ret = false;
			goto EXIT;
		}
	}
	
	channel->_pShareChannel->channelMemInfo.blockCount += count;

	buffer = new void *[count];

	bool succ = channel->_allocator->Allocate(buffer, count);

	if (!succ)
	{
		ret = false;
		goto EXIT;
	}

	ChannelHead * tailBlock = 0;
	if (channel->_pShareChannel->channelMemInfo.lastBlockOffset == -1)
	{
		tailBlock = 0;
		channel->_pShareChannel->channelMemInfo.firstBlockOffset = channel->_allocator->Offset(buffer[0]);
		channel->_pShareChannel->channelMemInfo.lastBlockOffset = channel->_allocator->Offset(buffer[0]);
	}
	else
	{
		tailBlock = (ChannelHead *)channel->_allocator->Ptr(channel->_pShareChannel->channelMemInfo.lastBlockOffset);
	}

	for (int i = 0; i < count; ++i)
	{
		if (tailBlock != 0)
			tailBlock->nextOffset = channel->_allocator->Offset(buffer[i]);

		tailBlock = (ChannelHead *)buffer[i];
	}

	tailBlock->nextOffset = -1;

	channel->_pShareChannel->channelMemInfo.lastBlockOffset = channel->_allocator->Offset(tailBlock);


EXIT:
	PostOperation_Mem(channel);

	if (buffer != 0)
		delete [] buffer;

	return ret;
}

static void Free(ChannelImpl * channel)
{
	PreOperation_Mem(channel);

	int headOffset = channel->_pShareChannel->channelMemInfo.firstBlockOffset;
	while (headOffset != channel->_pShareChannel->channelRWInfo.readBlockOffset)
	{
		ChannelHead * headPtr = (ChannelHead *)channel->_allocator->Ptr(headOffset);
		headOffset = headPtr->nextOffset;
		channel->_allocator->Free(headPtr);
		channel->_pShareChannel->channelMemInfo.blockCount--;
		//printf("One block freed\n");
	}
	
	channel->_pShareChannel->channelMemInfo.firstBlockOffset = headOffset;

	PostOperation_Mem(channel);
}

static void Rollback_Mem(ChannelImpl * channel)
{
	if (IsDirty_Mem(channel->_rollbackManager))
	{
		Rollback_Mem(channel->_rollbackManager);
		channel->_allocator->RollBack();
		SetClean_Mem(channel->_rollbackManager);
	}	
}

static void PrepareRollback(ChannelImpl * channel)
{
	channel->_allocator->PrepareRollBack();
	PrepareRollback_Mem(channel->_rollbackManager, channel->_pShareChannel);
}

static void PreOperation_RW(ChannelImpl * channel)
{
	Lock_RW(channel->_pShareChannel);

	if (IsDirty_RW(channel->_pShareChannel))
	{
		Rollback_RW(channel->_pShareChannel);
		SetCleen_RW(channel->_pShareChannel);
	}

	PrepareRollback_RW(channel->_pShareChannel);

	SetDirty_RW(channel->_pShareChannel);
}

static void PostOperation_RW(ChannelImpl * channel)
{
	SetCleen_RW(channel->_pShareChannel);

	Unlock_RW(channel->_pShareChannel);
}

static void Rollback_RW(ChannelImpl * channel)
{
	Lock_RW(channel->_pShareChannel);

	if (IsDirty_RW(channel->_pShareChannel))
	{
		Rollback_RW(channel->_pShareChannel);
		SetCleen_RW(channel->_pShareChannel);
	}

	Unlock_RW(channel->_pShareChannel);
}


static void PostOperation_Mem(ChannelImpl * channel)
{

	SetClean_Mem(channel->_rollbackManager);
	
	::Unlock_Mem(channel->_rollbackManager);
}

static void Peek(ChannelImpl * channel, char * buf, int length, int * pLengthPeeked, int * pReadOffset, int * pReadBlockOffset)
{
	// rollback memory operation
	::Lock_Mem(channel->_rollbackManager);
	::Rollback_Mem(channel);
	::Unlock_Mem(channel->_rollbackManager);

	int readBlockOffsetForFree;
	int readBlockOffset, readOffset, writeBlockOffset, writeOffset, readableSize;
	
	// load
	PreOperation_RW(channel);

	readBlockOffsetForFree = readBlockOffset	= channel->_pShareChannel->channelRWInfo.readBlockOffset;
	readOffset									= channel->_pShareChannel->channelRWInfo.readOffset;
	writeBlockOffset							= channel->_pShareChannel->channelRWInfo.writeBlockOffset;
	writeOffset									= channel->_pShareChannel->channelRWInfo.writeOffset;
	readableSize								= channel->_pShareChannel->channelRWInfo.readableSize;

	PostOperation_RW(channel);

	if (readBlockOffsetForFree == -1)
	{
		readBlockOffsetForFree = readBlockOffset = channel->_pShareChannel->channelMemInfo.firstBlockOffset;
	}

	// op
	length = MIN(length, readableSize);
	int sizeToRead = length;
	char * dstPtr = buf;

	while(sizeToRead > 0)
	{
		ChannelHead * blockPtr = (ChannelHead *)channel->_allocator->Ptr(readBlockOffset);
		int readableSizeThisBlock = channel->_allocator->BlockDataCapacity() - sizeof(ChannelHead) - readOffset;
		if (readableSizeThisBlock == 0)
		{
			readBlockOffset = blockPtr->nextOffset;
			readOffset = 0;
			blockPtr = (ChannelHead *)channel->_allocator->Ptr(readBlockOffset);
			continue;
		}

		char * srcPtr = (char *)blockPtr + sizeof(ChannelHead) + readOffset;

		int readSize = MIN(sizeToRead, readableSizeThisBlock);

		memcpy(dstPtr, srcPtr, readSize);
		dstPtr += readSize;

		sizeToRead -= readSize;
		readOffset += readSize;
	}

	if (pLengthPeeked != 0)
		*pLengthPeeked = length;
	
	if (pReadOffset != 0)
		*pReadOffset = readOffset;

	if (pReadBlockOffset != 0)
		*pReadBlockOffset = readBlockOffset;

	return;
}
