#pragma once

#include <Windows.h>
#include "BlockBufferAllocator.h"
#include "IChannelBuffer.h"
#include "ShareMemHelper.h"
#include "SpinLock.h"

struct ChannelBufferControlData
{
	volatile unsigned __int32 channelId;
	volatile unsigned long long timestamp;
};

class ChannelController
{
public:
	ChannelController(const wchar_t * name)
	{
		_sm = AllocateSharedMem(
			name,
			sizeof(ChannelBufferControlData),
			ChannelBufferControlDataInit, 0);

		_sm->InitializeNamedSpinlock();
	}

	~ChannelController()
	{
		if (_sm)
			ShareMem::FreeShareMem(_sm);
	}

	unsigned long long TakeNewTimeStamp()
	{
		ChannelBufferControlData * controlData = (ChannelBufferControlData *)_sm->GetAddress();
		//return InterlockedIncrement64(&controlData->timestamp);
		_sm->Lock(INFINITE);
		unsigned long long ret = controlData->timestamp++;	// TODO
		_sm->Unlock();
		return ret;
	}

	__int32 TakeNewChannelId()
	{
		ChannelBufferControlData * controlData = (ChannelBufferControlData *)_sm->GetAddress();
		return InterlockedIncrement(&controlData->channelId);
	}

	static BOOL ChannelBufferControlDataInit(ShareMem* sm, void* context)
	{
		ChannelBufferControlData * controlData = (ChannelBufferControlData *)sm->GetAddress();
		controlData->channelId = 0;
		controlData->timestamp = 0;
		return TRUE;
	}

private:
	ShareMem * _sm;
};


class ChannelBuffer : public IChannelBufferReadable, IChannelBufferWritable
{
public:
	static const _int32 SIZE = 4096;
	static IChannelBufferReadable * _stdcall OpenForRead(const wchar_t * name, _int32 numBlock, _int32 sizeBlock);
	static IChannelBufferWritable * _stdcall OpenForWrite(const wchar_t * name, _int32 numBlock, _int32 sizeBlock, __int32 userId);
	virtual void ReadData(ProcessFunc func, void * userData);
	virtual int WriteData(const char * buffer, size_t size);
	int BufferSizeAvailable();
//	//ChannelBuffer();
private:
	static ChannelBuffer * __stdcall Open(const wchar_t * name, _int32 numBlock, _int32 sizeBlock, bool bRead);
	ChannelBuffer();
	~ChannelBuffer();
	void SetUserId(__int32 userId);
	MemBufferBlock * CurrentBlock();
	void AllocNewBlockAndWriteMetaData();
	void FreeBlock(MemBufferBlock * memBlock);
	//BlockBufferAllocator * _BlockBufferAllocator;
	MemBufferBlock * _currentBlock;
//
	static void __stdcall BlockBufferProcessFunc(MemBufferBlock * memBlock, void * userData);
	ProcessFunc _func;
//
	char * _readBuffer;
	void * _userData;
	__int32 _userId;

	ChannelController * _channelController;

private:
	static SpinLock __lock;
	static BlockBufferAllocator * _BlockBufferAllocator;
	static volatile unsigned long __openCount;
};
