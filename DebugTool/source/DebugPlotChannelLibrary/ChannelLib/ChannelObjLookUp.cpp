#include <Windows.h>
#include <stdio.h>
#include "HashTable.h"
#include "ChannelObjLookuUp.h"
#include "Channel.h"
#include "ShareChannelManager.h"

struct ChannelKey
{
	const char * name;
	int type;
};

unsigned long ___HashCodeString(const char * str)
{
	int hashVal = 4967;
	int c;

	while (c = *str++)
		hashVal = ((hashVal << 4) + hashVal) + c;

	return hashVal;
}

bool ___EqualString(const char * s1, const char * s2)
{
	return strcmp(s1, s2) == 0;
}

bool ___EqualChannel(const ChannelKey *key1, const ChannelKey *key2)
{
	return ___EqualString(key1->name, key2->name) && (key1->type == key2->type);
}

unsigned long ___HashCodeChannel(const ChannelKey * key)
{
	return ___HashCodeString(key->name) + key->type;
}

typedef SimpleHashTable<const ChannelKey *, Channel *, 1024, ___HashCodeChannel, ___EqualChannel> HashType;

struct ChannelObjLookUpImpl
{
	HashType * pObjHash;
	SoraDbgPlot::Lock::RWLock rwLock;
};

ChannelObjLookUp::ChannelObjLookUp()
{
	_impl = new ChannelObjLookUpImpl();
	_impl->pObjHash = new HashType;
}

ChannelObjLookUp::~ChannelObjLookUp()
{
	delete _impl->pObjHash;
	delete _impl;
}

ChannelOpenResult ChannelObjLookUp::GetChannel(const char * channelName, int type, Channel ** pChannel)
{
	Channel * ch = 0;

	_impl->rwLock.LockRead();

	ChannelKey key;
	key.name = channelName;
	key.type = type;

	HashEntry<const ChannelKey *, Channel *> * entry = _impl->pObjHash->Find(&key);

	if (!entry)
	{
		_impl->rwLock.UnlockRead();
		_impl->rwLock.LockWrite();

		entry = _impl->pObjHash->Find(&key);
		if (!entry)
		{
			ch = ShareChannelManager::Instance()->OpenForWrite(channelName, type);

			if (ch != 0)
			{
				ChannelKey * keyDup = new ChannelKey();
				keyDup->name = _strdup(channelName);
				keyDup->type = type;

				_impl->pObjHash->Insert(keyDup, ch);
			}
		}
		else
		{
			ch = entry->value;
		}
		_impl->rwLock.UnlockWrite();
		_impl->rwLock.LockRead();
	}
	else
	{
		ch = entry->value;
	}

	_impl->rwLock.UnlockRead();

	if (pChannel != 0)
		*pChannel = ch;

	if (ch == 0)
		return ALLOCATION_FAIL;
	
	return OK;
}

static bool CleanTableFunc(const ChannelKey * key, Channel * channel)
{
	delete channel;

	delete [] key->name;
	delete key;

	return true;
}

void ChannelObjLookUp::Clean()
{
	_impl->rwLock.LockWrite();

	_impl->pObjHash->Foreach(CleanTableFunc);

	_impl->rwLock.UnlockWrite();
}
