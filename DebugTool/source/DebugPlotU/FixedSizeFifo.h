#pragma once

class FixedSizeFifo
{
public:
	static FixedSizeFifo * Create(void * ptr, _int32 size);
	static void InitShareMem(void * ptr, _int32 size);

	static _int32 CalcSharedBufferSize(_int32 numItem);
	bool Put(_int32 value);
	bool Get(_int32& value);
	_int32 ElementSize();

	void FillBitmap(char * bm, int maxCount);
	//int DataSize();

private:
	struct FixeSizeFifoSharedStruct
	{
		volatile _int32 _wPos;
		volatile _int32 _rPos;
		//volatile long _dataSize;
	};

private:
	FixedSizeFifo();
	bool InterlockedAddOne(volatile unsigned long * addr);

private:
	volatile _int32 * _ptr;
	_int32 _size;
	FixeSizeFifoSharedStruct * _smPtr;
};
