#pragma once

#include "Radio.h"
#include "SoraThread.h"
#include "sora.h"

class IDataReader
{
public:
	virtual int ReadData(char *, int) = 0;
};

class UMXSendThread : public SoraThread
{
public:
	UMXSendThread(Radio * radio, IThreadControl * callback);
	void SetReader(IDataReader * reader);
	void Threadfunc();

private:
	void ReadData(PRX_BLOCK buf, int size);

private:
	IDataReader * reader;
};

class UmxSender
{
public:
	UmxSender(Radio * radio);
	//~UmxSender();
	HRESULT Init();
	HRESULT Transfer(char * buf, int len, int retryCount);
	HRESULT Transmit(bool log);
	HRESULT Deinit();

private:
	PVOID sampleBuf;
	ULONG sampleBufSize;
	bool initialized;
	ULONG txID;
	Logger * logger;
	bool transferAllocated;
};

