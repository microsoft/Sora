#include "GlobalSingleton.h"
#include "SpinLock.h"

static HashType * g_pObjHash = 0;
SpinLock g_SpinLockHashTable;
HashType * SingletonGetHashTable()
{
	if (!g_pObjHash)
	{
		g_SpinLockHashTable.Lock();
		if (!g_pObjHash)
			g_pObjHash = new HashType;
		g_SpinLockHashTable.Unlock();
	}
	return g_pObjHash;
}

void SingletonReleaseHashTable()
{
	g_SpinLockHashTable.Lock();
	if (g_pObjHash)
	{
		delete g_pObjHash;
		g_pObjHash = 0;
	}
	g_SpinLockHashTable.Unlock();
}

static TextBufVecType * g_pTextVec = 0;
SpinLock g_SpinLockTextVec;
TextBufVecType * SingletonGetTextVector()
{
	if (!g_pTextVec)
	{
		g_SpinLockTextVec.Lock();
		if (!g_pTextVec)
			g_pTextVec = new TextBufVecType;
		g_SpinLockTextVec.Unlock();
	}
	return g_pTextVec;
}

void SingletonReleaseTextVector()
{
	g_SpinLockTextVec.Lock();
	if (g_pTextVec)
	{
		delete g_pTextVec;
		g_pTextVec = 0;
	}
	g_SpinLockTextVec.Unlock();
}
