#include <Windows.h>
#include "SharedStruct.h"
#include "SmProcess.h"
#include "ShareMemHelper.h"
#include "SharedNameManagement.h"
#include "AppSettings.h"

SmProcess::SmProcess(int pid, int bufferSize)
{
	_valid = true;
	_pid = pid;
	_bufferSize = bufferSize;

	wchar_t * nameBuf;
	size_t sizeNumBuf = 0;
	SharedNameManager::GenChannelName(pid, 0, sizeNumBuf);

	nameBuf = new wchar_t[sizeNumBuf];
	SharedNameManager::GenChannelName(pid, nameBuf, sizeNumBuf);

	_sm = AllocateSharedMem(
		nameBuf,
		sizeof(SmProcessStruct) + bufferSize, 
		SmInit, this);

	delete [] nameBuf;

	// source info
	size_t bufSize = 0;
	SharedNameManager::GenSourceName(pid, 0, bufSize);
	wchar_t * name = new wchar_t[bufSize];
	SharedNameManager::GenSourceName(pid, name, bufSize);

	_smSourceInfo = AllocateSharedMem(
		name,
		sizeof(SharedSourceInfo) + ::SettingGetSourceBufferSize(),
		SharedSourceInfo::Init);

	delete [] name;

	_smSourceInfo->InitializeNamedSpinlock();


	// event
	name = SharedNameManager::GenEventName();
	_syncEvent = AllocateSharedEvent(TRUE, TRUE, name);
	delete [] name;	

	// raw data event
	name = SharedNameManager::GenRawDataEventName(pid);
	_rawdataEvent = AllocateSharedEvent(TRUE, TRUE, name);
	delete [] name;
}

SmProcess::SmProcess(int pid)
{
	_pid = pid;
	_bufferSize = 4;
	_valid = true;

	wchar_t * nameBuf;
	size_t sizeNumBuf = 0;
	SharedNameManager::GenChannelName(pid, 0, sizeNumBuf);

	nameBuf = new wchar_t[sizeNumBuf];
	SharedNameManager::GenChannelName(pid, nameBuf, sizeNumBuf);

	_sm = AllocateSharedMem(
		nameBuf,
		sizeof(SmProcessStruct) + _bufferSize,
		SmInitR, this);

	delete [] nameBuf;

	// source info
	size_t bufSize = 0;
	SharedNameManager::GenSourceName(pid, 0, bufSize);
	wchar_t * name = new wchar_t[bufSize];
	SharedNameManager::GenSourceName(pid, name, bufSize);

	_smSourceInfo = AllocateSharedMem(
		name,
		sizeof(SharedSourceInfo) + ::SettingGetSourceBufferSize(),
		SharedSourceInfo::Init);

	delete [] name;

	_smSourceInfo->InitializeNamedSpinlock();


	// event
	name = SharedNameManager::GenEventName();
	_syncEvent = AllocateSharedEvent(TRUE, TRUE, name);
	delete [] name;	

	// raw data event
	name = SharedNameManager::GenRawDataEventName(pid);
	_rawdataEvent = AllocateSharedEvent(TRUE, TRUE, name);
	delete [] name;
}

SmProcess::~SmProcess()
{
	ShareMem::FreeShareMem(_sm);
	ShareMem::FreeShareMem(_smSourceInfo);
	ShareLock::FreeShareLock(_syncEvent);
	ShareLock::FreeShareLock(_rawdataEvent);
}

BOOL SmProcess::SmInit(ShareMem* sm, void* context)
{
	SmProcess * smProcess = (SmProcess *)context;
	SmProcessStruct * smProcessStruct = (SmProcessStruct *)sm->GetAddress();

	smProcessStruct->bufLen = smProcess->_bufferSize;
	smProcessStruct->pid = smProcess->_pid;
	smProcessStruct->rIdx = 0;
	smProcessStruct->wIdx = 0;
	smProcessStruct->pauseFlag = 0;

	return true;
}

BOOL SmProcess::SmInitR(ShareMem* sm, void* context)
{
	SmProcess * smProcess = (SmProcess *)context;
	SmProcessStruct * smProcessStruct = (SmProcessStruct *)sm->GetAddress();

	smProcess->_valid = false;

	smProcessStruct->bufLen = smProcess->_bufferSize;
	smProcessStruct->pid = smProcess->_pid;
	smProcessStruct->rIdx = 0;
	smProcessStruct->wIdx = 0;
	smProcessStruct->pauseFlag = 0;

	return true;
}

bool SmProcess::IsValid()
{
	return _valid;
}

int SmProcess::Pid()
{
	return _pid;
}

void SmProcess::Pause()
{
	SmProcessStruct * smProcessStruct = (SmProcessStruct *)_sm->GetAddress();	
	::InterlockedCompareExchange(&smProcessStruct->pauseFlag, 1, 0);
}

bool SmProcess::TestPauseFlag()
{
	SmProcessStruct * smProcessStruct = (SmProcessStruct *)_sm->GetAddress();	
	if (::InterlockedCompareExchange(&smProcessStruct->pauseFlag, 0, 1) == 1)
	{
		return true;
	}

	return false;
}

ShareMem * SmProcess::GetSourceInfo()
{
	return _smSourceInfo;
}

ShareLock * SmProcess::GetEvent()
{
	return _syncEvent;
}


int SmProcess::WriteRawData(COMPLEX16 * pData, int inNum)
{
	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)_smSourceInfo->GetAddress();

	_smSourceInfo->Lock(INFINITE);

	char * baseAddr = (char *)_smSourceInfo->GetAddress() + sizeof(SharedSourceInfo);
	int bufLen = sharedSourceInfo->bufLen - sharedSourceInfo->bufLen % sizeof(COMPLEX16);
	int rIdx = sharedSourceInfo->rIdx;
	int wIdx = sharedSourceInfo->wIdx;
	unsigned int len2End = bufLen - wIdx;
	unsigned int len2Write = inNum*sizeof(COMPLEX16);
	unsigned int writeLen = min(len2End, len2Write);

	if (writeLen > 0)
	{
		memcpy(baseAddr + wIdx, pData, writeLen);
		sharedSourceInfo->wIdx += writeLen;
	}

	int outNum = writeLen/sizeof(COMPLEX16);

	_smSourceInfo->Unlock();	

	return outNum;
}


int SmProcess::ReadRawData(COMPLEX16 * pData, int inNum)
{
	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)_smSourceInfo->GetAddress();

	_rawdataEvent->Lock(INFINITE);

	_smSourceInfo->Lock(INFINITE);

	if (sharedSourceInfo->singleStepFlag)
		_rawdataEvent->ResetShareEvent();

	char * baseAddr = (char *)_smSourceInfo->GetAddress() + sizeof(SharedSourceInfo);
	int rIdx = sharedSourceInfo->rIdx;
	int wIdx = sharedSourceInfo->wIdx;
	unsigned int len2End = wIdx - rIdx;
	unsigned int len2Read = inNum*sizeof(COMPLEX16);
	unsigned int readLen = min(len2End, len2Read);

	if (readLen > 0)
	{
		memcpy(pData, baseAddr + rIdx, readLen);
		sharedSourceInfo->rIdx += readLen;
	}

	int outNum = readLen/sizeof(COMPLEX16);

	_smSourceInfo->Unlock();

	return outNum;
}

void SmProcess::RawDataClear()
{
	ShareMem * smSourceInfo = _smSourceInfo;
	if (!smSourceInfo)
		return;

	smSourceInfo->Lock(INFINITE);

	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)smSourceInfo->GetAddress();
	sharedSourceInfo->wIdx = 0;
	sharedSourceInfo->rIdx = 0;

	smSourceInfo->Unlock();
}

void SmProcess::RawDataPlay()
{
	_smSourceInfo->Lock(INFINITE);
	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)_smSourceInfo->GetAddress();
	sharedSourceInfo->singleStepFlag = false;
	_rawdataEvent->SetShareEvent();
	_smSourceInfo->Unlock();
}

void SmProcess::RawDataPause()
{
	_rawdataEvent->ResetShareEvent();
}

void SmProcess::RawDataSingleStep()
{
	_smSourceInfo->Lock(INFINITE);
	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)_smSourceInfo->GetAddress();
	sharedSourceInfo->singleStepFlag = true;
	_rawdataEvent->SetShareEvent();
	_smSourceInfo->Unlock();
}

void SmProcess::SetReadPos(int pos)
{
	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)_smSourceInfo->GetAddress();
	this->_smSourceInfo->Lock(INFINITE);
	int seekIdx = pos * sizeof(COMPLEX16);
	seekIdx = min(seekIdx, sharedSourceInfo->wIdx);
	sharedSourceInfo->rIdx = seekIdx;
	this->_smSourceInfo->Unlock();
}

void SmProcess::PeekRawData(COMPLEX16 * buf, int size, int & start, int width, int & readPos, int & sizeValid)
{
	assert(size <= width);
	
	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)_smSourceInfo->GetAddress();
	COMPLEX16 * baseAddr = (COMPLEX16 *)_smSourceInfo->GetAddress() + sizeof(SharedSourceInfo);

	_smSourceInfo->Lock(INFINITE);
	
	readPos = sharedSourceInfo->rIdx / sizeof(COMPLEX16);
	
	start = readPos - readPos % width;

	int i;
	for (i = 0; i < size; i++)
	{
		int indexScaled = (int)((__int64)i * width / size + start);
		if (indexScaled >= sharedSourceInfo->wIdx / (int)sizeof(COMPLEX16))
			break;
		if (indexScaled >= ::SettingGetSourceBufferComplex16Count())
			break;
		buf[i] = baseAddr[indexScaled];
	}

	sizeValid = i;


	_smSourceInfo->Unlock();
}

bool SmProcess::IsRawDataBufferUsed()
{
	bool isInUse = false;
	_smSourceInfo->Lock(INFINITE);
	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)_smSourceInfo->GetAddress();
	isInUse = sharedSourceInfo->wIdx != 0;
	_smSourceInfo->Unlock();
	return isInUse;
}
