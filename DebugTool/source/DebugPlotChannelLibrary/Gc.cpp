#include "Gc.h"
#include "ChannelTableStruct.h"
#include "RollbackManager.h"
#include "HashTable.h"

unsigned long ___HashCodeInt(const int key)
{
	return key;
}

bool ___EqualInt(const int a, const int b)
{
	return a == b;
}

typedef SimpleHashTable<int, int, 1024, ___HashCodeInt, ___EqualInt> HashType;

using namespace SharedChannelTable;

struct GcSession
{
	HashType hashTable;
};


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

static void RecordPid(int pid, HashType * hashTable)
{
	if (hashTable->Find(pid) == 0)
	{
		hashTable->Insert(pid, ::CheckPid(pid));
	}	
}

void Gc(ChannelTableItem * item, GcSession * session)
{
	int pidPlotter = 0;
	int pidViewer = 0;
	int type = -1;

	while(1)
	{
		bool bDone = true;

		SharedChannelTable::PreTableOperation(item);
		pidPlotter = item->channelInfo.pidPlotter;
		pidViewer = item->channelInfo.pidViewer;
		type = item->channelInfo.type;
		SharedChannelTable::PostTableOperation(item);

		if (type == -1)
			break;

		RecordPid(pidPlotter, &session->hashTable);
		RecordPid(pidViewer, &session->hashTable);

		SharedChannelTable::PreTableOperation(item);
		if (pidPlotter != item->channelInfo.pidPlotter || pidViewer != item->channelInfo.pidViewer)
		{
			bDone = false;
		}
		else
		{
			if (session->hashTable.Find(pidPlotter)->value == 0)
			{
				item->channelInfo.pidPlotter = 0;
			}
			if (session->hashTable.Find(pidViewer)->value == 0)
			{
				item->channelInfo.pidViewer = 0;
			}

			if (item->channelInfo.pidPlotter == 0 && item->channelInfo.pidViewer == 0 && item->channelRWInfo.readableSize == 0)
			{
				// reset rw
				

				// gc mem

				item->channelInfo.type = -1;
			}
			
		}
		SharedChannelTable::PostTableOperation(item);

		if (bDone)
			break;
	}
}
