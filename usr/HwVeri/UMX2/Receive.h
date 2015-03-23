#pragma once

#include "Radio.h"
#include "SoraThread.h"

class IDataProcessor
{
public:
	virtual void ProcessData(PRX_BLOCK, int) = 0;
};

class UMXReceiveThread : public SoraThread
{
public:
	UMXReceiveThread(Radio * radio, IThreadControl * callback);
	void SetProcessor(IDataProcessor * processor);
	void Threadfunc();

private:
	void ProcessData(PRX_BLOCK buf, int size);

private:
	IDataProcessor * processor;
};

class UmxReceiver
{
public:
	UmxReceiver(Radio * radio);
	HRESULT Init();
	HRESULT ReceiveBlock(PRX_BLOCK block, bool log);
	HRESULT Deinit();

private:
	Logger * logger;
	PVOID rxBuf;
	ULONG rxBufSize;
	SORA_RADIO_RX_STREAM SampleStream;
	bool sampleStreamAllocated;
	bool rxBufferMapped;
};

