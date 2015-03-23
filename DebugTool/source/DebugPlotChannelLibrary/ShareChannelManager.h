#pragma once

#include "Channel.h"
#include "_share_mem_if.h"

namespace SharedChannelTable
{
	struct RollbackStruct;
	struct ChannelTableItem;
}

struct ChannelInfo2
{
	char * name;
	int type;
	int pidPloter;
};

typedef bool (* ChannelEnumCallback)(ChannelInfo2 * info, void * userData);

class ShareChannelManager
{
public:
	static ShareChannelManager * __stdcall Instance();
	static void __stdcall Release();

private:
	static BOOL ChannelListInitFunc(ShareMem* sm, void* context);

private:
	static ShareChannelManager * __instance;

public:
	ShareChannelManager();
	~ShareChannelManager();

	Channel * OpenForRead(int pid, const char * name, int type);
	void CloseForRead(Channel * pCh);

	Channel * OpenForWrite(const char * name, int type);
	void CloseForWrite(Channel * ch);

	void Gc();

	void Enumerate(ChannelEnumCallback callback, void * userData);

public: // monitor API
	int FreeBlockCount();

private:
	DpFixedSizeSharedMemAllocator * _allocator;
	ShareMem * _smChannel;
	SharedChannelTable::RollbackStruct * _rollbackStruct;
	MemRollbackManager * _memRollbackManager;
	SharedChannelTable::ChannelTableItem * _pChannelTableBase;
};
