#pragma once
#include "SpinLock2.h"
#include "Config.h"

namespace SharedChannelTable
{

	const int MAX_CHANNEL = 128;

	/************************************

	RollbackStruct : 
	ChannelTableItem * CHANNEL_COUNT

	*************************************/


	struct RollbackStruct
	{
		SM_SpinLock spinLock;
		volatile int memDirty;
		volatile int channelIdx;
	};

	inline void Init(RollbackStruct * rollbackStruct)
	{
		Init(&rollbackStruct->spinLock, 5000);
		rollbackStruct->memDirty = 0;
		rollbackStruct->channelIdx = 0;
	}

	struct ChannelMemInfo
	{
		volatile int firstBlockOffset;
		volatile int lastBlockOffset;
		volatile int blockCount;
		volatile int sizeLimit;
	};

	inline void Init(ChannelMemInfo * channelMemInfo)
	{
		channelMemInfo->firstBlockOffset = -1;
		channelMemInfo->lastBlockOffset = -1;
		channelMemInfo->blockCount = 0;
		channelMemInfo->sizeLimit = Config::CHANNEL_BUFFER_LIMIT_INITIAL_VALUE();
	}

	struct ChannelRWInfo
	{
		volatile int readBlockOffset;
		volatile int writeBlockOffset;
		volatile int readOffset;
		volatile int writeOffset;
		volatile int readableSize;
	};

	inline void Init(ChannelRWInfo * channelRWInfo)
	{
		memset(channelRWInfo, 0, sizeof(ChannelRWInfo));
		channelRWInfo->writeBlockOffset = -1;
		channelRWInfo->readBlockOffset = -1;
	}

	struct ChannelItemInfo
	{
		volatile int pidViewer;
		volatile int pidPlotter;
		int type;
		char name[256];
	};

	inline void Init(ChannelItemInfo * channelInfo)
	{
		memset(channelInfo, 0, sizeof(ChannelItemInfo));
		channelInfo->type = -1;
	}

	struct ChannelTableItem
	{
		// channel data & rollback
		SM_SpinLock spinLockTableItem;
		volatile int dirtyFlagTableItem;
		ChannelItemInfo channelInfo;
		ChannelItemInfo channelInfoRollback;

		// rw data & rollback
		int rwDirtyFlag;
		SM_SpinLock spinLockRW;
		ChannelRWInfo channelRWInfo;
		ChannelRWInfo channelRWInfoRollback;

		// memry data & rollback
		ChannelMemInfo channelMemInfo;
		ChannelMemInfo channelMemInfoRollback;
	};

	inline void Init(ChannelTableItem * item)
	{
		Init(&item->spinLockTableItem, 5000);
		item->dirtyFlagTableItem = 0;
		Init(&item->channelInfo);
		Init(&item->channelInfoRollback);
	
		item->rwDirtyFlag = 0;
		Init(&item->spinLockRW, 5000);
		Init(&item->channelRWInfo);
		Init(&item->channelRWInfoRollback);
	
		Init(&item->channelMemInfo);
		Init(&item->channelMemInfoRollback);
	}


	/****************

	block structure

	*****************/
	struct ChannelHead
	{
		int nextOffset;
	};
	
	/******
	
	Table Operation

	*******/

	
	void PreTableOperation(ChannelTableItem * item);

	void PostTableOperation(ChannelTableItem * item);
}
