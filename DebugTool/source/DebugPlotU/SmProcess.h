#pragma once

#include "ShareMemHelper.h"
#include "sora.h"
#include "_share_lock_if.h"

struct SmProcessStruct
{
	int pid;
	int bufLen;
	volatile int rIdx;
	volatile int wIdx;
	volatile unsigned long pauseFlag;
};

class SmProcess
{
public:
	SmProcess(int pid, int bufferSize);
	SmProcess(int pid);
	~SmProcess();
	bool IsValid();
	int Pid();
	void Pause();
	bool TestPauseFlag();

	ShareMem * GetSourceInfo();
	ShareLock * GetEvent();


	int WriteRawData(COMPLEX16 * pData, int inNum);
	int ReadRawData(COMPLEX16 * pData, int inNum);
	void RawDataClear();

	void RawDataPlay();
	void RawDataPause();
	void RawDataSingleStep();
	void PeekRawData(COMPLEX16 * buf, int size, int & start, int width, int & readPos, int & sizeValid);
	void SetReadPos(int pos);
	bool IsRawDataBufferUsed();

private:
	static BOOL SmInit(ShareMem* sm, void* context);
	static BOOL SmInitR(ShareMem* sm, void* context);
	ShareMem * _sm;
	ShareMem * _smSourceInfo;
	ShareLock * _rawdataEvent;
	ShareLock * _syncEvent;
	int _pid;
	int _bufferSize;
	bool _valid;
};

