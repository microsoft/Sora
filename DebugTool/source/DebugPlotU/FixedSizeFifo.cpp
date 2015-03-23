#include <Windows.h>
#include "FixedSizeFifo.h"

FixedSizeFifo * FixedSizeFifo::Create(void * ptr, _int32 size)
{
	if (ptr == 0)
		return 0;

	if (size <= 0)
		return 0;

	FixedSizeFifo * freeFifo = new FixedSizeFifo();
	freeFifo->_smPtr = (FixeSizeFifoSharedStruct *)ptr;
	_int32 offset = sizeof(FixeSizeFifoSharedStruct);
	freeFifo->_ptr = (_int32 *)((char *)ptr + offset);
	freeFifo->_size = (size - offset)/sizeof(_int32);

	return freeFifo;
}

void FixedSizeFifo::InitShareMem(void * ptr, _int32 size)
{
	FixeSizeFifoSharedStruct * smPtr = (FixeSizeFifoSharedStruct *)ptr;
	smPtr->_rPos = 0;
	smPtr->_wPos = 0;
	//smPtr->_dataSize = 0;

	_int32 offset = sizeof(FixeSizeFifoSharedStruct);
	_int32 numItem = (size - offset) / sizeof(_int32);
	_int32 * ptrItem = (_int32 *)((char *)ptr + offset);
	for (int i = 0; i < numItem; i++)
	{
		ptrItem[i] = -1;
	}
}

bool FixedSizeFifo::Put(_int32 value)
{
	while(1)
	{
		_int32 wOld = _smPtr->_wPos;
		_int32 rOld = _smPtr->_rPos;

		_int32 diff = rOld - wOld;
		if (diff < 0)
			diff += _size;
		if ( diff == 1 )
			return false;

		unsigned long res = ::InterlockedCompareExchange((volatile unsigned long *)(_ptr + wOld), value, -1);

		if (res == -1)
		{
			_smPtr->_wPos = (wOld + 1) % _size;

			//::InterlockedIncrement(&_smPtr->_dataSize);

			return true;
		}
	}
}

bool FixedSizeFifo::Get(_int32& value)
{
	while(1)
	{
		_int32 wOld = _smPtr->_wPos;
		_int32 rOld = _smPtr->_rPos;

		_int32 diff = wOld - rOld;
		if ( diff == 0)
			return false;

		__int32 oldValue = *((volatile _int32 *)(_ptr + rOld));
		if (oldValue == -1)
			continue;

		unsigned long res = ::InterlockedCompareExchange((volatile unsigned long *)(_ptr + rOld), -1, oldValue);

		if (res == oldValue)
		{
			_smPtr->_rPos = (rOld + 1) % _size;
			value = res;

			//::InterlockedDecrement(&_smPtr->_dataSize);

			return true;
		}
	}
	return false;
}

FixedSizeFifo::FixedSizeFifo() :
	_ptr(0)
{}

bool FixedSizeFifo::InterlockedAddOne(volatile unsigned long * addr)
{
	while(1)
	{
		unsigned long oldValue = *addr;
		unsigned long targetValue = (oldValue + 1) % _size;
		unsigned long res = ::InterlockedCompareExchange(addr, targetValue, oldValue);
		if (res == oldValue)
			return true;
	}
}

_int32 FixedSizeFifo::CalcSharedBufferSize(_int32 numItem)
{
	return (numItem + 1) * sizeof(_int32) + sizeof(FixeSizeFifoSharedStruct);
}

void FixedSizeFifo::FillBitmap(char * bm, int maxCount)
{
	for (int count = 0; count < maxCount; count++)
	{
		for (int i = 0; i < _size; i++)
		{
			int num = _ptr[i];
			if (num != -1)
				bm[num] = 1;
		}
	}
}

//int FixedSizeFifo::DataSize()
//{
//	return _smPtr->_dataSize;
//}
