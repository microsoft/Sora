#include <Windows.h>
#include <stdio.h>
#include "Channel.h"
#include "DpSharedMemAllocator.h"
#include "DpFixedSizeSharedMemAllocator.h"
#include "RollbackManager.h"
#include "ShareChannelManager.h"
#include "Config.h"

using namespace SharedChannelTable;

struct _EnumPara
{
	ChannelEnumCallback callback;
	void * userData;
};

typedef bool (* ChannelTableEnumCallback)(ChannelTableItem * item, void * userData);
void SafeEnumChannelTable(ChannelTableItem * base, ChannelTableEnumCallback callback, void * userData);

void SafeEnumChannelTable(ChannelTableItem * base, ChannelTableEnumCallback callback, void * userData)
{
	for (int i = 0; i < MAX_CHANNEL; ++i)
	{
		bool cont = true;

		ChannelTableItem * item = base + i;
		
		PreTableOperation(item);
		
		callback(item, userData);

		PostTableOperation(item);

		if (!cont)
			break;
	}
}

static bool ChannelTableEnumCallbackFunc(ChannelTableItem * item, void * userData)
{
	_EnumPara * para = (_EnumPara *)userData;
	ChannelInfo2 info;
	info.pidPloter = item->channelInfo.pidPlotter;
	info.type = item->channelInfo.type;
	strcpy_s(info.name, 256, item->channelInfo.name);

	para->callback(&info, para->userData);

	return true;
}

ShareChannelManager * ShareChannelManager::__instance = 0;

ShareChannelManager * __stdcall ShareChannelManager::Instance()
{
	if (__instance == 0)
	{
		__instance = new ShareChannelManager();
	}

	return __instance;
}

void __stdcall ShareChannelManager::Release()
{
	if (__instance != 0)
	{
		delete __instance;
		__instance = 0;
	}
}

Channel * ShareChannelManager::OpenForRead(int pid, const char * name, int type)
{
	ChannelTableItem * itemSelected = 0;
	int channelId = -1;

	for (int i = 0; i < MAX_CHANNEL; ++i)
	{
		bool bBreak = false;

		ChannelTableItem * item = this->_pChannelTableBase + i;
		
		PreTableOperation(item);

		if (
			item->channelInfo.pidViewer == 0		&&		// no one is reading
			item->channelInfo.pidPlotter == pid		&&		// writer pid matches
			item->channelInfo.type == type			&&		// type matches
			strcmp(item->channelInfo.name, name) == 0		// name matches
			)
		{
			itemSelected = item;
			channelId = i;
			item->channelInfo.pidViewer = ::GetCurrentProcessId();
			bBreak = true;
		}

		PostTableOperation(item);

		if (bBreak) break;
	}
	
	if (itemSelected != 0)
	{
		Channel * channel = new Channel(itemSelected, _allocator, _memRollbackManager);
		return channel;
	}

	return 0;
}

void ShareChannelManager::CloseForRead(Channel * pCh)
{
	ChannelTableItem * item = pCh->ShareStruct();

	PreTableOperation(item);

	item->channelInfo.pidViewer = 0;

	PostTableOperation(item);

	delete pCh;
}

Channel * ShareChannelManager::OpenForWrite(const char * name, int type)
{
	if (type == -1)
		return 0;

	ChannelTableItem * itemSelected = 0;
	int channelId = -1;

	for (int i = 0; i < MAX_CHANNEL; ++i)
	{
		bool bBreak = false;

		ChannelTableItem * item = this->_pChannelTableBase + i;

		PreTableOperation(item);

		if (item->channelInfo.type == -1) // no reading && no writing
		{
			itemSelected = item;
			channelId = i;
			item->channelInfo.pidPlotter = ::GetCurrentProcessId();
			item->channelInfo.type = type;
			bBreak = true;
		}

		PostTableOperation(item);

		if (bBreak) break;
	}
	
	if (itemSelected != 0)
	{
		Channel * channel = new Channel(itemSelected, _allocator, _memRollbackManager);

		return channel;
	}

	return 0;
}

void ShareChannelManager::CloseForWrite(Channel * pCh)
{
	ChannelTableItem * item = pCh->ShareStruct();

	PreTableOperation(item);

	item->channelInfo.pidPlotter = 0;

	PostTableOperation(item);

	delete pCh;
}

ShareChannelManager::ShareChannelManager()
{
	_allocator = DpFixedSizeSharedMemAllocator::Create(L"/DbgPlot/Allocator", Config::ALLOC_BLOCK_SIZE(), Config::ALLOC_BLOCK_COUNT());

	_smChannel = DpShareMemAllocator::Create(
		L"/DbgPlot/ChannelList", 
		sizeof(RollbackStruct) + sizeof(ChannelTableItem) * MAX_CHANNEL, 
		ChannelListInitFunc, 
		this	);

	_rollbackStruct = (RollbackStruct *)_smChannel->GetAddress();
	_pChannelTableBase = (ChannelTableItem *)(_rollbackStruct + 1);

	_memRollbackManager = new MemRollbackManager();
	_memRollbackManager->RollBackStruct = _rollbackStruct;
	_memRollbackManager->tableBasePtr = _pChannelTableBase;
}

ShareChannelManager::~ShareChannelManager()
{
	DpFixedSizeSharedMemAllocator::Release(_allocator);
	delete _memRollbackManager;
	ShareMem::FreeShareMem(_smChannel);
}

BOOL ShareChannelManager::ChannelListInitFunc(ShareMem* sm, void* context)
{
	RollbackStruct * rollbackStruct = (RollbackStruct *)sm->GetAddress();
	ChannelTableItem * channelTableBase = (ChannelTableItem *)(rollbackStruct + 1);

	Init(rollbackStruct);

	for (int i = 0; i < MAX_CHANNEL; ++i)
	{
		ChannelTableItem * item = channelTableBase + i;
		Init(item);
	}

	return TRUE;
}

void ShareChannelManager::Enumerate(ChannelEnumCallback callback, void * userData)
{
	GcSession * gcSession = new GcSession();

	for (int i = 0; i < MAX_CHANNEL; ++i)
	{
		ChannelTableItem * item = this->_pChannelTableBase + i;

		GC_Channel(item, _memRollbackManager, _allocator, gcSession);

		PreTableOperation(item);

		if (item->channelInfo.type != -1)
		{
			ChannelInfo2 chInfo;

			chInfo.pidPloter = item->channelInfo.pidPlotter;
			chInfo.type = item->channelInfo.type;
			chInfo.name = item->channelInfo.name;

			callback(&chInfo, userData);
		}

		PostTableOperation(item);
	}

	delete gcSession;
}

void ShareChannelManager::Gc()
{
	GcSession * gcSession = new GcSession();

	for (int i = 0; i < MAX_CHANNEL; ++i)
	{
		ChannelTableItem * item = this->_pChannelTableBase + i;
		GC_Channel(item, _memRollbackManager, _allocator, gcSession);
	}

	delete gcSession;
}

int ShareChannelManager::FreeBlockCount()
{
	int freeCount = 0;

	Lock_Mem(_memRollbackManager);
	if (IsDirty_Mem(_memRollbackManager))
	{
		Rollback_Mem(_memRollbackManager);
		SetClean_Mem(_memRollbackManager);
	}

	freeCount = _allocator->FreeCount();

	Unlock_Mem(_memRollbackManager);

	return freeCount;
}
