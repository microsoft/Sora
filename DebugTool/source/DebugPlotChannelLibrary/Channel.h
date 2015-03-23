#pragma once

//#include "DpFixedSizeSharedMemAllocator.h"
//#include "ChannelTableStruct.h"
//#include "RollbackManager.h"

namespace SharedChannelTable
{
	struct ChannelTableItem;
}

struct ChannelImpl;
struct MemRollbackManager;
class DpFixedSizeSharedMemAllocator;

class Channel
{
	friend class ShareChannelManager;
public:
	~Channel();
	bool Write(const char * buf, int length);
	int Read(char * buf, int length);
	int Peek(char * buf, int length);
	unsigned Size();
	void Close();
	int Type();
	bool IsWriterAlive();
	void SetBufferLimit(int size);

private:
	Channel(SharedChannelTable::ChannelTableItem * shareChannel, DpFixedSizeSharedMemAllocator * allocator, MemRollbackManager * rollbackManager);
	SharedChannelTable::ChannelTableItem * ShareStruct();

private:
	ChannelImpl * _channel;
};
