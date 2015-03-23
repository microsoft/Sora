#include <Windows.h>
#include "sora.h"
#include "DpViewer.h"
#include "DpViewerExt.h"
#include "HashTable.h"
#include "SpinLock.h"

unsigned long ___HashCodeIntEx(int val)
{
	return val;
}

bool ___EqualIntEx(int val1, int val2)
{
	return val1 == val2;
}

typedef SimpleHashTable<int, void *, 1024, ___HashCodeIntEx, ___EqualIntEx> HashType;

struct Buffer
{
	int length;
	char * buffer;
	SpinLock lock;

	void AllocForSize(int size)
	{
		if (length < size)
		{
			if (buffer == 0)
			{
				delete [] buffer;
			}

			buffer = new char[size];
			length = size;
		}
	}

	void Init()
	{
		length = 0;
		buffer = 0;
	}

	void Release()
	{
		if (buffer != 0)
		{
			delete [] buffer;
			buffer = 0;
		}

		length = 0;
	}
};

struct ViewerLibExtMain
{
	HashType hashTable;
	SpinLock lock;

	ViewerLibExtMain()
	{
	
	}

	~ViewerLibExtMain()
	{
		hashTable.Foreach(&ReleaseFunc);
	}

	Buffer * GetBuffer(int key)
	{
		Buffer * buf = 0;

		lock.Lock();

		HashEntry<int, void *> * entry = hashTable.Find(key);

		if (entry == 0)
		{
			buf = new Buffer();
			buf->Init();
			hashTable.Insert(key, buf);
		}
		else
		{
			buf = (Buffer *)entry->value;
		}

		lock.Unlock();

		return buf;
	}

	static bool ReleaseFunc(int key, void * value)
	{
		Buffer * buf = (Buffer *)value;
		buf->Release();
		delete buf;

		return true;
	}
};

ViewerLibExtMain viewerLibExtMain;

HRESULT __stdcall DpReadLine(CHANNEL_HANDLE hChannel, int * buf, int lenBuf, int * lenRead)
{
	// if type mismatch, 
	int lenReadChar = 0;
	HRESULT hResult = DpReadChannelData(hChannel, (char *)buf, sizeof(int) * lenBuf, &lenReadChar);
	*lenRead = lenReadChar / sizeof(int);
	return hResult;
}

HRESULT __stdcall DpReadDots(CHANNEL_HANDLE hChannel, COMPLEX16 * buf, int lenBuf, int * lenRead)
{
	int lenReadChar = 0;
	HRESULT hResult = DpReadChannelData(hChannel, (char *)buf, sizeof(COMPLEX16) * lenBuf, &lenReadChar);
	*lenRead = lenReadChar / sizeof(int);
	return hResult;
}

HRESULT __stdcall DpReadSpectrum(CHANNEL_HANDLE hChannel, int * buf, int lenBuf, int * lenRead, int * lenRequired)
{
	HRESULT hRet = S_OK;

	int lenReadInBytes = 0;
	int sizeSpectrum;

	// 1. get the size of the spectrum
	HRESULT hResult = DpPeekChannelData(hChannel, (char *)&sizeSpectrum, sizeof(int), &lenReadInBytes);
	
	if (lenReadInBytes < sizeof(int))	// not enough data
	{
		if (hResult == E_CHANNEL_CLOSED)
			hRet = E_CHANNEL_CLOSED;
		goto NOT_ENOUGH_DATA;
	}
	else if (sizeSpectrum > lenBuf)	// buffer is to small
	{
		goto NOT_ENOUGH_BUFFER;
	}

	// 3. do a double copy
	Buffer * bufObj = viewerLibExtMain.GetBuffer((int)hChannel);

	int sizeHead = sizeof(int);
	int sizeBody = sizeSpectrum * sizeof(int);
	int sizeAll = sizeHead + sizeBody;

	bufObj->lock.Lock();

	bufObj->AllocForSize(sizeAll);
	hResult = DpReadChannelData(hChannel, bufObj->buffer, sizeAll, &lenReadInBytes);

	assert(lenReadInBytes == sizeAll);

	memcpy(buf, bufObj->buffer + sizeHead, lenReadInBytes - sizeHead);

	int sizeOut = (lenReadInBytes - sizeHead) / sizeof(int);

	if (lenRead != 0)
		*lenRead = sizeOut;

	if (lenRequired != 0)
		*lenRequired = sizeOut;

	bufObj->lock.Unlock();

	hRet = hResult;
	return hRet;

NOT_ENOUGH_DATA:
	if (lenRead != 0)
		*lenRead = 0;
	if (lenRequired != 0)
		*lenRequired = 0;
	return hRet;

NOT_ENOUGH_BUFFER:
	if (lenRead != 0)
		*lenRead = 0;
	if (lenRequired != 0)
		*lenRequired = sizeSpectrum;
	return E_NOT_ENOUGH_BUFFER;
}

HRESULT __stdcall DpReadText(CHANNEL_HANDLE hChannel, char * buf, int lenBuf, int * lenRead, int * lenRequired)
{
	HRESULT hRet = S_OK;

	int lenReadInBytes = 0;
	int sizeText;

	// 1. get the size of the spectrum
	HRESULT hResult = DpPeekChannelData(hChannel, (char *)&sizeText, sizeof(int), &lenReadInBytes);
	
	if (lenReadInBytes < sizeof(int))	// not enough data
	{
		if (hResult == E_CHANNEL_CLOSED)
			hRet = E_CHANNEL_CLOSED;
		goto NOT_ENOUGH_DATA;
	}
	else if (sizeText > lenBuf)	// buffer is to small
	{
		goto NOT_ENOUGH_BUFFER;
	}

	// 3. do a double copy
	Buffer * bufObj = viewerLibExtMain.GetBuffer((int)hChannel);

	int sizeHead = sizeof(int);
	int sizeBody = sizeText * sizeof(char);
	int sizeAll = sizeHead + sizeBody;

	bufObj->lock.Lock();

	bufObj->AllocForSize(sizeAll);
	hResult = DpReadChannelData(hChannel, bufObj->buffer, sizeAll, &lenReadInBytes);

	assert(lenReadInBytes == sizeAll);

	memcpy(buf, bufObj->buffer + sizeHead, lenReadInBytes - sizeHead);

	int sizeOut = lenReadInBytes - sizeHead;

	if (lenRequired != 0)
		*lenRequired = sizeOut;
	if (lenRead != 0)
		*lenRead = sizeOut;

	bufObj->lock.Unlock();

	hRet = hResult;
	return hRet;

NOT_ENOUGH_DATA:
	if (lenRead != 0)
		*lenRead = 0;
	if (lenRequired != 0)
		*lenRequired = 0;
	return hRet;

NOT_ENOUGH_BUFFER:
	if (lenRead != 0)
		*lenRead = 0;
	if (lenRequired != 0)
		*lenRequired = sizeText;
	return E_NOT_ENOUGH_BUFFER;
}
