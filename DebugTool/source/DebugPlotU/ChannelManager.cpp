#include "ChannelManager.h"
#include "AppSettings.h"

int BaseChannel::_pid = ::GetCurrentProcessId();

BaseChannel::BaseChannel(_int32 id)
{
	_id = id;
	_writableBuffer = ChannelBuffer::OpenForWrite(
		SettingGetGlobalBufferName(),
		SettingGetGlobalBufferBlockNum(),
		SettingGetGlobalBufferBlockSize(),
		id
		);
}

BaseChannel::~BaseChannel()
{
	delete _writableBuffer;
}

_int32 BaseChannel::GetId()
{
	return _id;
}

int BaseChannel::Write(const char * buf, int size)
{
	return _writableBuffer->WriteData(buf, size);
}

int BaseChannel::BufferSizeAvailable()
{
	return _writableBuffer->BufferSizeAvailable();
}
