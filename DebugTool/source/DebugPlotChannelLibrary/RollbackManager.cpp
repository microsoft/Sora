#include <stdio.h>
#include "ChannelTableStruct.h"
#include "RollbackManager.h"
#include "SpinLock2.h"
#include "DpFixedSizeSharedMemAllocator.h"
#include "HashTable.h"

using namespace SharedChannelTable;

int CheckPid(int pid);

/*******************************

mem

*******************************/

void Lock_Mem(MemRollbackManager * manager)
{
	::Lock(&manager->RollBackStruct->spinLock);
}

void Unlock_Mem(MemRollbackManager * manager)
{
	::Unlock(&manager->RollBackStruct->spinLock);
}


void Rollback_Mem(MemRollbackManager * manager)
{
	ChannelTableItem * channelInfo = (ChannelTableItem *)(manager->tableBasePtr) + manager->RollBackStruct->channelIdx;
	channelInfo->channelMemInfo.sizeLimit				= channelInfo->channelMemInfoRollback.sizeLimit;
	channelInfo->channelMemInfo.firstBlockOffset		= channelInfo->channelMemInfoRollback.firstBlockOffset;
	channelInfo->channelMemInfo.lastBlockOffset			= channelInfo->channelMemInfoRollback.lastBlockOffset;
	channelInfo->channelMemInfo.blockCount				= channelInfo->channelMemInfoRollback.blockCount;
}

void PrepareRollback_Mem(MemRollbackManager * manager, ChannelTableItem * sharedChannelInfo)
{
	int index = sharedChannelInfo - manager->tableBasePtr;
	sharedChannelInfo->channelMemInfoRollback.sizeLimit				= sharedChannelInfo->channelMemInfo.sizeLimit;
	sharedChannelInfo->channelMemInfoRollback.firstBlockOffset		= sharedChannelInfo->channelMemInfo.firstBlockOffset;
	sharedChannelInfo->channelMemInfoRollback.lastBlockOffset		= sharedChannelInfo->channelMemInfo.lastBlockOffset;
	sharedChannelInfo->channelMemInfoRollback.blockCount			= sharedChannelInfo->channelMemInfo.blockCount;
	manager->RollBackStruct->channelIdx = index;
}

void SetDirty_Mem(MemRollbackManager * manager)
{
	manager->RollBackStruct->memDirty = 1;
}

void SetClean_Mem(MemRollbackManager * manager)
{
	manager->RollBackStruct->memDirty = 0;
}

bool IsDirty_Mem(MemRollbackManager * manager)
{
	return manager->RollBackStruct->memDirty == 1;
}

void RollbackIfDirty(MemRollbackManager * manager)
{
	Lock_Mem(manager);

	if (IsDirty_Mem(manager))
	{
		Rollback_Mem(manager);
		SetClean_Mem(manager);
	}

	Unlock_Mem(manager);
}

/**************************

r & w

***************************/

void Lock_RW(ChannelTableItem * sharedChannelInfo)
{
	Lock(&sharedChannelInfo->spinLockRW);
}

void Unlock_RW(ChannelTableItem * sharedChannelInfo)
{
	Unlock(&sharedChannelInfo->spinLockRW);
}

bool IsDirty_RW(ChannelTableItem * sharedChannelInfo)
{
	return sharedChannelInfo->rwDirtyFlag != 0;
}

void Rollback_RW(ChannelTableItem * sharedChannelInfo)
{
	sharedChannelInfo->channelRWInfo.writeOffset			= sharedChannelInfo->channelRWInfoRollback.writeBlockOffset;
	sharedChannelInfo->channelRWInfo.writeBlockOffset		= sharedChannelInfo->channelRWInfoRollback.writeBlockOffset;

	sharedChannelInfo->channelRWInfo.readableSize			= sharedChannelInfo->channelRWInfoRollback.readableSize;
	sharedChannelInfo->channelRWInfo.readOffset			= sharedChannelInfo->channelRWInfoRollback.readOffset;
	sharedChannelInfo->channelRWInfo.readBlockOffset		= sharedChannelInfo->channelRWInfoRollback.readBlockOffset;
}

void PrepareRollback_RW(ChannelTableItem * sharedChannelInfo)
{
	sharedChannelInfo->channelRWInfoRollback.writeBlockOffset	= sharedChannelInfo->channelRWInfo.writeOffset;
	sharedChannelInfo->channelRWInfoRollback.writeBlockOffset	= sharedChannelInfo->channelRWInfo.writeBlockOffset;

	sharedChannelInfo->channelRWInfoRollback.readableSize		= sharedChannelInfo->channelRWInfo.readableSize;
	sharedChannelInfo->channelRWInfoRollback.readOffset		= sharedChannelInfo->channelRWInfo.readOffset;
	sharedChannelInfo->channelRWInfoRollback.readBlockOffset	= sharedChannelInfo->channelRWInfo.readBlockOffset;
}

void SetDirty_RW(ChannelTableItem * sharedChannelInfo)
{
	sharedChannelInfo->rwDirtyFlag = 1;
}

void SetCleen_RW(ChannelTableItem * sharedChannelInfo)
{
	sharedChannelInfo->rwDirtyFlag = 0;
}

void RollbackIfDirty_RW(ChannelTableItem * sharedChannelInfo)
{
	::Lock_RW(sharedChannelInfo);

	if (::IsDirty_RW(sharedChannelInfo))
	{
		::Rollback_RW(sharedChannelInfo);
		::SetCleen_RW(sharedChannelInfo);
	}

	::Unlock_RW(sharedChannelInfo);
}


/******************

GC

*******************/


static void RecordPid(int pid, HashType * hashTable)
{
	if (hashTable->Find(pid) == 0)
	{
		hashTable->Insert(pid, ::CheckPid(pid));
	}	
}

void GC_Channel(ChannelTableItem * channelInfo, MemRollbackManager * memRollbackManager, DpFixedSizeSharedMemAllocator * allocator, GcSession *gcSession)
{
	int pidPlotter = 0;
	int pidViewer = 0;
	int type = -1;

	bool bDone = true;

	while(1)
	{
		SharedChannelTable::PreTableOperation(channelInfo);
		pidPlotter = channelInfo->channelInfo.pidPlotter;
		pidViewer = channelInfo->channelInfo.pidViewer;
		type = channelInfo->channelInfo.type;
		SharedChannelTable::PostTableOperation(channelInfo);

		if (type == -1)
			break;

		RecordPid(pidPlotter, &gcSession->hashTable);
		RecordPid(pidViewer, &gcSession->hashTable);

		SharedChannelTable::PreTableOperation(channelInfo);

		if (pidPlotter != channelInfo->channelInfo.pidPlotter || pidViewer != channelInfo->channelInfo.pidViewer)
		{
			bDone = false;
		}
		else
		{
			// do actual work

			// correct writer pid info
			if (gcSession->hashTable.Find(pidPlotter)->value == 0)
			{
				channelInfo->channelInfo.pidPlotter = 0;
			}
			if (gcSession->hashTable.Find(pidViewer)->value == 0)
			{
				channelInfo->channelInfo.pidViewer = 0;
			}

			// gc memory if both reader and writer is dead
			if (channelInfo->channelInfo.pidPlotter == 0 && channelInfo->channelInfo.pidViewer == 0)
			{
				
				channelInfo->channelInfo.type = -1;

				// rollback rw if necessary
				RollbackIfDirty_RW(channelInfo);

				// if no data readable, free memory
				// since reader & writer are all dead, it's safe to read values without lock
				//if (channelInfo->channelRWInfo.readableSize == 0)
				if (true)
				{
					// reset rw info
					Init(&channelInfo->channelRWInfo);
					Init(&channelInfo->channelRWInfoRollback);

					// reset mem info
					Lock_Mem(memRollbackManager);
					if (IsDirty_Mem(memRollbackManager))
					{
						Rollback_Mem(memRollbackManager);
						SetClean_Mem(memRollbackManager);
					}

					PrepareRollback_Mem(memRollbackManager, channelInfo);
					SetDirty_Mem(memRollbackManager);

					int head = channelInfo->channelMemInfo.firstBlockOffset;

					while(head != -1)
					{
						ChannelHead * headPtr = (ChannelHead *)allocator->Ptr(head);
						head = headPtr->nextOffset;
						allocator->Free(headPtr);
						//printf("Free one block\n");
					}

					Init(&channelInfo->channelMemInfo);

					SetClean_Mem(memRollbackManager);

					Unlock_Mem(memRollbackManager);
				}
			}

			SharedChannelTable::PostTableOperation(channelInfo);
		}

		if (bDone)
			break;
	}
}

void PrepareRollback_Table(ChannelTableItem * channelInfo)
{
	channelInfo->channelInfoRollback.pidPlotter = channelInfo->channelInfo.pidPlotter;
	channelInfo->channelInfoRollback.pidViewer = channelInfo->channelInfo.pidViewer;
	channelInfo->channelInfoRollback.type = channelInfo->channelInfo.type;
}

void Rollback_Table(ChannelTableItem * channelInfo)
{
	channelInfo->channelInfo.pidPlotter = channelInfo->channelInfoRollback.pidPlotter;
	channelInfo->channelInfo.pidViewer = channelInfo->channelInfoRollback.pidViewer;
	channelInfo->channelInfo.type = channelInfo->channelInfoRollback.type;
}

static int CheckPid(int pid)
{
	HANDLE processHandle = ::OpenProcess(SYNCHRONIZE, FALSE, pid);
	if (processHandle == NULL)
	{
		return 0;
	}
	else
	{
		::CloseHandle(processHandle);
		return 1;
	}
}
