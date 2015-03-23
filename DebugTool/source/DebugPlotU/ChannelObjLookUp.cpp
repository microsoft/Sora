#include "HashTable.h"
#include "ChannelObjLookuUp.h"

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

typedef SimpleHashTable<const char *, SharedChannelW *, 1024, ___HashCodeString, ___EqualString> HashType;

static HashType * g_pObjHash = 0;
static SoraDbgPlot::Lock::RWLock g_rwLock;

static HashType * SingletonGetHashTable()
{
	if (!g_pObjHash)
	{
		if (!g_pObjHash)
			g_pObjHash = new HashType;
	}
	return g_pObjHash;
}

SharedChannelW * ChannelObjLookUp::GetSharedChannel(const char * channelName, ChannelType chType, int spectrumSize)
{
	SharedChannelW * ch = 0;

	g_rwLock.LockRead();

	HashEntry<const char *, SharedChannelW *> * entry = SingletonGetHashTable()->Find(channelName);

	if (!entry)
	{
		g_rwLock.UnlockRead();
		g_rwLock.LockWrite();

		entry = SingletonGetHashTable()->Find(channelName);
		if (!entry)
		{
			ch = new SharedChannelW(channelName, chType, spectrumSize);
			SingletonGetHashTable()->Insert(_strdup(channelName), ch);
		}
		else
		{
			ch = entry->value;
		}
		g_rwLock.UnlockWrite();
		g_rwLock.LockRead();
	}
	else
	{
		ch = entry->value;
	}

	g_rwLock.UnlockRead();

	return ch;
}

static bool CleanTableFunc(const char * key, SharedChannelW * channel)
{
	delete [] key;
	delete channel;

	return true;
}

void ChannelObjLookUp::Clean()
{
	g_rwLock.LockWrite();

	if (g_pObjHash != 0)
	{
		g_pObjHash->Foreach(CleanTableFunc);
		delete g_pObjHash;
		g_pObjHash = 0;
	}

	g_rwLock.UnlockWrite();
}
