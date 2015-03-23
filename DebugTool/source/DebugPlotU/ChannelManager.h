#pragma once

#include <Windows.h>
#include "HashTable.h"
#include "IChannelBuffer.h"
#include "ChannelBufferImpl.h"

class BaseChannel
{
public:
	static int _pid;

	BaseChannel(_int32 id);
	~BaseChannel();
	_int32 GetId();
	int Write(const char * buf, int size);
	int BufferSizeAvailable();

private:
	_int32 _id;
	IChannelBufferWritable * _writableBuffer;
};
