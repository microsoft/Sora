#pragma once

#include "HashTable.h"
#include "ChannelTableStruct.h"
#include "DpFixedSizeSharedMemAllocator.h"

inline unsigned long ___HashCodeInt(const int key)
{
	return key;
}

inline bool ___EqualInt(const int a, const int b)
{
	return a == b;
}

typedef SimpleHashTable<int, int, 1024, ___HashCodeInt, ___EqualInt> HashType;

struct GcSession
{
	HashType hashTable;
};

struct MemRollbackManager
{
	SharedChannelTable::RollbackStruct * RollBackStruct;
	SharedChannelTable::ChannelTableItem * tableBasePtr;
};

//

// mem
void Lock_Mem(MemRollbackManager * manager);
void Unlock_Mem(MemRollbackManager * manager);

bool IsDirty_Mem(MemRollbackManager * manager);
void Rollback_Mem(MemRollbackManager * manager);
void PrepareRollback_Mem(MemRollbackManager * manager, SharedChannelTable::ChannelTableItem * sharedChannelInfo);
void SetDirty_Mem(MemRollbackManager * manager);
void SetClean_Mem(MemRollbackManager * manager);

void RollbackIfDirty(MemRollbackManager * manager);

// r & w

void Lock_RW(SharedChannelTable::ChannelTableItem * sharedChannelInfo);
void Unlock_RW(SharedChannelTable::ChannelTableItem * sharedChannelInfo);

bool IsDirty_RW(SharedChannelTable::ChannelTableItem * sharedChannelInfo);
void Rollback_RW(SharedChannelTable::ChannelTableItem * sharedChannelInfo);
void PrepareRollback_RW(SharedChannelTable::ChannelTableItem * sharedChannelData);
void SetDirty_RW(SharedChannelTable::ChannelTableItem * sharedChannelInfo);
void SetCleen_RW(SharedChannelTable::ChannelTableItem * sharedChannelInfo);

void RollbackIfDirty_RW(SharedChannelTable::ChannelTableItem * sharedChannelInfo);

struct GC_RollbackManager
{
	SharedChannelTable::ChannelTableItem * channelBase;
	int tableSize;
};

void GC(GC_RollbackManager * manager);
void GC_Channel(SharedChannelTable::ChannelTableItem * channelInfo, MemRollbackManager * memRollbackManager, DpFixedSizeSharedMemAllocator * allocator, GcSession *gcSession);
void PrepareRollback_Table(SharedChannelTable::ChannelTableItem * channelInfo);
void Rollback_Table(SharedChannelTable::ChannelTableItem * channelInfo);


