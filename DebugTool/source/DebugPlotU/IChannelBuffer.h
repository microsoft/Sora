#pragma once
#include "BlockBufferAllocator.h"

typedef bool (__stdcall *ProcessFunc)(const char * buffer, int size, void * userData, __int32 userId);

class IChannelBufferReadable
{
public:
	virtual void ReadData(ProcessFunc func, void * userData) = 0;
	virtual ~IChannelBufferReadable() {}
	virtual int BufferSizeAvailable() = 0;
};

class IChannelBufferWritable
{
public:
	virtual int WriteData(const char * buffer, size_t size) = 0;
	virtual ~IChannelBufferWritable() {}
	virtual int BufferSizeAvailable() = 0;
};

