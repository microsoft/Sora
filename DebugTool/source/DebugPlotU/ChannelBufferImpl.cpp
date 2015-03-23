#include <assert.h>
#include "ChannelBufferImpl.h"
#include "ShareMemHelper.h"


SpinLock ChannelBuffer::__lock;
BlockBufferAllocator * ChannelBuffer::_BlockBufferAllocator = 0;
volatile unsigned long ChannelBuffer::__openCount = 0;

IChannelBufferReadable * ChannelBuffer::OpenForRead(const wchar_t * name, _int32 numBlock, _int32 sizeBlock)
{
	ChannelBuffer * channelBuffer = ChannelBuffer::Open(name, numBlock, sizeBlock, true);
	channelBuffer->_readBuffer = new char[SIZE];

	return (IChannelBufferReadable *)channelBuffer;
}

IChannelBufferWritable * ChannelBuffer::OpenForWrite(const wchar_t * name, _int32 numBlock, _int32 sizeBlock, __int32 userId)
{
	ChannelBuffer * channelBuffer = ChannelBuffer::Open(name, numBlock, sizeBlock, false);
	channelBuffer->SetUserId(userId);

	return (IChannelBufferWritable *)channelBuffer;
}

ChannelBuffer * ChannelBuffer::Open(const wchar_t * name, _int32 numBlock, _int32 sizeBlock, bool bRead)
{
	//BlockBufferAllocator * BlockBufferAllocator = 0;
	
	__lock.Lock();
	if (::InterlockedIncrement(&__openCount) == 1)
	{
		_BlockBufferAllocator = BlockBufferAllocator::Open(name, numBlock, sizeBlock, bRead);
	}
	else
	{
		while(_BlockBufferAllocator == 0);
	}
	__lock.Unlock();

	//if (BlockBufferAllocator == 0)
	//	return 0;

	ChannelBuffer * channelBuffer = new ChannelBuffer();
	//channelBuffer->_BlockBufferAllocator = BlockBufferAllocator;

	const wchar_t NAME_APPEND[] = L"|controller";
	wchar_t * controllerName = new wchar_t[wcslen(name) + 1 + wcslen(NAME_APPEND)];
	controllerName[0] = 0;
	wcscat(controllerName, name);
	wcscat(controllerName, NAME_APPEND);
	channelBuffer->_channelController = new ChannelController(controllerName);
	channelBuffer->_userId = channelBuffer->_channelController->TakeNewChannelId();
	delete [] controllerName;

	return channelBuffer;
}

ChannelBuffer::ChannelBuffer() :
	//_BlockBufferAllocator(0),
	_currentBlock(0),
	_readBuffer(0)
{
}

ChannelBuffer::~ChannelBuffer()
{
	if (_channelController)
		delete _channelController;

	if (_readBuffer)
		delete [] _readBuffer;

	//if (_BlockBufferAllocator)
	//	delete _BlockBufferAllocator;
	__lock.Lock();

	if (::InterlockedDecrement(&__openCount) == 0)
	{
		delete _BlockBufferAllocator;
		_BlockBufferAllocator = 0;
	}

	__lock.Unlock();
}

void ChannelBuffer::SetUserId(__int32 userId)
{
	_userId = userId;
}

int ChannelBuffer::WriteData(const char * buffer, size_t size)
{
	__int16 id = 0;

	if (size <= 0)
		return 0;

	unsigned long long timestamp = _channelController->TakeNewTimeStamp();

	size_t sizeToWrite = size;
	char * ptr = (char *)buffer;

	while (sizeToWrite > 0)
	{
		MemBufferBlock * block = CurrentBlock();
		if (block)
		{
			block->TryReset();

			size_t sizeWrittenInThisCall = 0;
			size_t sizeFree = block->FreeSize();

			size_t sizeHSize = sizeof(__int16);
			size_t sizeHId = sizeof(__int16);
			size_t sizeHTimeStamp = sizeof(unsigned long long);
			size_t sizeEControl = sizeof(char);

			size_t size_H_plus_E = sizeHSize + sizeHId + sizeHTimeStamp + sizeEControl;

			if (sizeFree > size_H_plus_E)
			{	// write a frame
				size_t sizeData = min(sizeFree - size_H_plus_E, sizeToWrite);
				size_t sizePayload = sizeHId + sizeHTimeStamp + sizeEControl + sizeData;
				block->BeginWrite();
				block->WriteData((char *)&sizePayload, sizeHSize, sizeWrittenInThisCall);
				block->WriteData((char *)&id, sizeHId, sizeWrittenInThisCall);
				block->WriteData((char *)&timestamp, sizeHTimeStamp, sizeWrittenInThisCall);
				block->WriteData((char *)ptr, sizeData, sizeWrittenInThisCall);
				
				sizeToWrite -= sizeData;
				char controlFlag = sizeToWrite > 0 ? 0 : 1;
				block->WriteData((char *)&controlFlag, sizeEControl, sizeWrittenInThisCall);

				ptr += sizeData;
				block->EndWrite();
				id++;

				if (block->FreeSize() == 0)
					AllocNewBlockAndWriteMetaData();

			}
			else
			{
				char padding = 0;
				block->BeginWrite();
				for (size_t i = 0; i < sizeFree; i++)
				{
					block->WriteData(&padding, 1, sizeWrittenInThisCall);
				}
				block->EndWrite();
				AllocNewBlockAndWriteMetaData();
			}
		}
		else
			break;
	}

	return size - sizeToWrite;
}

void ChannelBuffer::ReadData(ProcessFunc func, void * userData)
{
	_func = func;
	_userData = userData;
	this->_BlockBufferAllocator->ForEachInUseBlock(BlockBufferProcessFunc, this);
}

int ChannelBuffer::BufferSizeAvailable()
{
	return _BlockBufferAllocator->FreeBufferCount() * _BlockBufferAllocator->BlockSize();
}

MemBufferBlock * ChannelBuffer::CurrentBlock()
{
	if (!_currentBlock)
	{
		AllocNewBlockAndWriteMetaData();
	}

	return _currentBlock;
}


void ChannelBuffer::AllocNewBlockAndWriteMetaData()
{
	_currentBlock = _BlockBufferAllocator->AllocFreeMemBlock(_userId);
}

void ChannelBuffer::FreeBlock(MemBufferBlock * memBlock)
{
	_BlockBufferAllocator->FreeMemBufferBlock(memBlock);
}

void ChannelBuffer::BlockBufferProcessFunc(MemBufferBlock * memBlock, void * userData)
{
	ChannelBuffer * channelBuffer = (ChannelBuffer *)userData;
	_int32 size = memBlock->DataSize();
	_int32 sizeRead;
	bool isEmpty = memBlock->ReadData(channelBuffer->_readBuffer, size, sizeRead);
	
	if (sizeRead > 0)
		channelBuffer->_func(channelBuffer->_readBuffer, sizeRead, channelBuffer->_userData, memBlock->UserId()); 
}

