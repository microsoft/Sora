#include <Windows.h>
#include "sora.h"
#include "__DebugPlotChannel.h"
#include "DpChannel.h"
#include "DpChannelExt.h"
#include "ChannelLibExtImpl.h"
#include "HashTable.h"
#include "SpinLock.h"

struct ChannelKeyEx
{
	const char * name;
	int type;
};

unsigned long ___HashCodeStringEx(const char * str)
{
	int hashVal = 4967;
	int c;

	while (c = *str++)
		hashVal = ((hashVal << 4) + hashVal) + c;

	return hashVal;
}

bool ___EqualStringEx(const char * s1, const char * s2)
{
	return strcmp(s1, s2) == 0;
}

bool ___EqualChannelEx(const ChannelKeyEx *key1, const ChannelKeyEx *key2)
{
	return ___EqualStringEx(key1->name, key2->name) && (key1->type == key2->type);
}

unsigned long ___HashCodeChannelEx(const ChannelKeyEx * key)
{
	return ___HashCodeStringEx(key->name) + key->type;
}

typedef SimpleHashTable<const ChannelKeyEx *, void *, 1024, ___HashCodeChannelEx, ___EqualChannelEx> HashType;

struct ChannelObj;

typedef void (* ChannelObjInitFunc)(ChannelObj *);
typedef void (* ChannelObjReleaseFunc)(ChannelObj *);

struct ChannelObj
{
	int type;
	void * data;
	ChannelObjReleaseFunc releaseFunc;

	void Release()
	{
		if (releaseFunc != 0)
		{
			releaseFunc(this);
		}
	}
};

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

struct ChannelLibMain
{
	SpinLock lock;
	HashType hashTable;

	ChannelLibMain()
	{
	
	}

	~ChannelLibMain()
	{
		hashTable.Foreach(&CleanUpFunc);
	}

	static bool CleanUpFunc(const ChannelKeyEx * key, void * value)
	{
		ChannelObj * obj = (ChannelObj *)value;
		obj->Release();
		delete obj;

		delete [] key->name;
		delete key;

		return true;
	}

	ChannelObj * GetChannelObj(const char * name, int type, ChannelObjInitFunc initFunc, ChannelObjReleaseFunc releaseFunc)
	{
		ChannelObj * obj = 0;

		lock.Lock();

		ChannelKeyEx key;
		key.name = name;
		key.type = type;

		HashEntry<const ChannelKeyEx *, void *> * keyValue = hashTable.Find(&key);
		if (keyValue != 0)
		{
			obj = (ChannelObj *)keyValue->value;
			if (obj->type != type)	// type mismatch
				obj = 0;
		}
		else
		{
			obj = new ChannelObj();
			obj->type = type;
			obj->data = 0;
			obj->releaseFunc = releaseFunc;

			if (initFunc != 0)
				initFunc(obj);

			ChannelKeyEx * keyDup = new ChannelKeyEx();
			keyDup->name = _strdup(name);
			keyDup->type = type;

			hashTable.Insert(keyDup, obj);
		}

		lock.Unlock();

		return obj;
	}
};


static ChannelLibMain channelLibMain;

HRESULT __stdcall DpPlotLine(const char * name, const int * buf, int length)
{
	//ChannelObj * obj = channelLibMain.GetChannelObj(name, CH_LINE_TYPE, 0, 0);
	//
	//if (obj == 0)
	//	return E_PLOT_TYPE_MISMATCH;

	return ::__PlotData(name, CH_LINE_TYPE, (char *)buf, length * sizeof(int));
}

HRESULT __stdcall DpPlotDots(const char * name, const COMPLEX16 * buf, int length)
{
	//ChannelObj * obj = channelLibMain.GetChannelObj(name, CH_DOTS_TYPE, 0, 0);

	//if (obj == 0)
	//	return E_PLOT_TYPE_MISMATCH;

	return ::__PlotData(name, CH_DOTS_TYPE, (char *)buf, length * sizeof(COMPLEX16));
}

static void BufferInit(ChannelObj * obj)
{
	Buffer * buf = new Buffer();
	buf->Init();

	obj->data = buf;
}

static void BufferRelease(ChannelObj *obj)
{
	Buffer * buf = (Buffer *)obj->data;
	buf->Release();
	delete buf;
	obj->data = 0;
}

HRESULT __stdcall DpPlotSpectrum(const char * name, const int * buf, int length)
{
	// double copy, because
	// we need to write size and data
	// it may fail if we write size first, and then we found that there're not enough space for data, then we fail.
	// but if that happens, we've already written the size, but not the data
	// that causes an inconsistent situation

	ChannelObj * obj = channelLibMain.GetChannelObj(name, CH_SPECTRUM_TYPE, &BufferInit, &BufferRelease);

	if (obj == 0)
		return E_ALLOCATION_FAIL;

	Buffer * bufObj = (Buffer *)obj->data;

	bufObj->lock.Lock();

	int sizeHead = sizeof(int);
	int sizeBody = length * sizeof(int);
	int sizeAll = sizeHead + sizeBody;

	bufObj->AllocForSize(sizeAll);
	memcpy(bufObj->buffer, &length, sizeHead);
	memcpy(bufObj->buffer + sizeHead, buf, sizeBody);

	HRESULT hResult = ::__PlotData(name, CH_SPECTRUM_TYPE, bufObj->buffer, sizeAll);

	bufObj->lock.Unlock();

	return hResult;
}

HRESULT __stdcall DpPlotText(const char * name, const char * format, ...)
{
	ChannelObj * obj = channelLibMain.GetChannelObj(name, CH_TEXT_TYPE, &BufferInit, &BufferRelease);

	if (obj == 0)
		return E_ALLOCATION_FAIL;

	Buffer * bufObj = (Buffer *)obj->data;

	bufObj->lock.Lock();

	const size_t TEXT_BUF_LEN = 4096;

	bufObj->AllocForSize(TEXT_BUF_LEN + sizeof(int));

	va_list ap;
	va_start(ap, format);
	_vsnprintf_s(bufObj->buffer + sizeof(int), TEXT_BUF_LEN, _TRUNCATE, format, ap);
	va_end(ap);	

	int length = strlen(bufObj->buffer + sizeof(int)) + 1;
	int sizeHead = sizeof(int);
	int sizeBody = length;
	int sizeAll = sizeHead + sizeBody;
	
	memcpy(bufObj->buffer, &sizeBody, sizeHead);

	HRESULT hResult = ::__PlotData(name, CH_TEXT_TYPE, bufObj->buffer, sizeAll);

	bufObj->lock.Unlock();

	return hResult;
}
