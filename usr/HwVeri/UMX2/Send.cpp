#include "sora.h"
#include "Send.h"
#include "UmxCommonOps.h"
#include "ShortSleep.h"

HRESULT SendLoop(SoraThread * thread, IDataReader * reader, Radio * radio);

UMXSendThread::UMXSendThread(Radio * radio, IThreadControl * threadCallback)
{
	this->reader = 0;
	SetCallback(threadCallback);
}

void UMXSendThread::SetReader(IDataReader * reader)
{
	this->reader = reader;
}

void UMXSendThread::Threadfunc()
{
	HRESULT hResult;
	int ret = 0;
	UmxSender * sender = 0;

	hResult = ::SoraUmxInit(Radio::Current());
	if (FAILED(hResult))
	{
		goto EXIT;
	}

	Radio::Current()->SoraUmxConfigRadio();

	sender = new UmxSender(Radio::Current());
	
	hResult = sender->Init();
	if (FAILED(hResult))
		goto EXIT;

	const int SIZE_2M = 2*1024*1024;
	char * buf = new char[SIZE_2M];
	if (buf == 0)
	{
		Logger::GetLogger(L"umx")->Log(LOG_ERROR, L"Cannot alloc buffer for data reading.\n");
		goto EXIT_DEINIT;
	}

	int readSize;
	if (reader)
		readSize = reader->ReadData(buf, SIZE_2M);

	if (readSize <= 0)
		goto EXIT_DEINIT;

	hResult = sender->Transfer(buf, readSize, 10);
	if (FAILED(hResult))
		goto EXIT_DEINIT;

	Logger * logger = Logger::GetLogger(L"umx");
	
	bool txLogEnabled = true;

	logger->Log(LOG_INFO, L"Start transmitting signal...\n");

	while(!CheckStatus())
	{
		::ShortSleep(100);
		
		hResult = sender->Transmit(txLogEnabled);

		if (txLogEnabled)
		{
			if (hResult == S_OK)
				logger->Log(LOG_INFO, L"Repeated transmission-related function calls will not be logged.\n");
			txLogEnabled = false;
		}

		if (FAILED(hResult))
			break;
	}

	logger->Log(LOG_INFO, L"End transmitting signal.\n");

EXIT_DEINIT:
	if (buf)
		delete buf;
	sender->Deinit();

EXIT:

	::SoraUmxDeinit(Radio::Current());
	if (sender)
		delete sender;
	return;
	//::SendLoop(this, reader, radio);
}

UmxSender::UmxSender(Radio * radio)
{
	//this->radio = radio;
	initialized = false;
	sampleBuf = 0;
	sampleBufSize = 0;
	logger = Logger::GetLogger(L"umx");
	txID = -1;
	transferAllocated = false;
}

HRESULT UmxSender::Init()
{
	if (initialized)
		return S_OK;

	HRESULT hr = -1;

	hr = SoraURadioMapTxSampleBuf(
		Radio::Current()->GetRadioNum(),
		&sampleBuf,
		&sampleBufSize
		);

	logger->Log(LOG_FUNC_CALL, L"SoraURadioMapTxSampleBuf called for radio %d\r\n", Radio::Current()->GetRadioNum());
	if (FAILED(hr))
	{
		logger->Log(LOG_ERROR, L"Failed. ret = %x\r\n", hr);
		return -1;
	}
	else
	{
		logger->Log(LOG_SUCCESS, L"success\r\n");
	}

	initialized = true;

	return S_OK;
}

HRESULT UmxSender::Transfer(char * transferBuf, int transferBufSize, int retryCount)
{
	if (!initialized)
		return -1;

	HRESULT hr = -1;
	ULONG nSigLen;

	if (transferBufSize <= 0)
		return -1;
	if (transferBuf == 0)
		return -1;

	retryCount = min(1, retryCount);

	nSigLen = (ULONG)min(transferBufSize, (long)sampleBufSize);

	memcpy(sampleBuf, transferBuf, nSigLen);

	// align buffer
	if ( (nSigLen & RCB_BUFFER_ALIGN_MASK) != 0 )
	{
		nSigLen = ((nSigLen) & (~RCB_BUFFER_ALIGN_MASK)) + RCB_BUFFER_ALIGN;
	}

	transferAllocated = false;
	int i;
	for (i = 0; i < retryCount; i++)
	{
		hr = SoraURadioTransfer(Radio::Current()->GetRadioNum(), nSigLen, &txID);
		logger->Log(LOG_FUNC_CALL, L"SoraURadioTransfer called for radio %d. SampleSize = %d\r\n", Radio::Current()->GetRadioNum(), nSigLen);

		if (SUCCEEDED(hr))
		{
			transferAllocated = true;
			logger->Log(LOG_SUCCESS, L"success\r\n");
			logger->Log(LOG_INFO, L"TxID: %d\n", txID);
			break;
		}
		else
		{
			logger->Log(LOG_ERROR, L"Failed. ret = %x\r\n", hr);
		}
		::Sleep(2000);
	}

	if (i == retryCount)
		return -1;
	else
	{
		return S_OK;
	}
}

HRESULT UmxSender::Transmit(bool log)
{
	if (!transferAllocated)
		return -1;

	HRESULT hr = -1;

	hr = SoraURadioTx(Radio::Current()->GetRadioNum(), txID);

	if (log)
	{
		logger->Log(LOG_FUNC_CALL, L"SoraURadioTx called for radio %d\n", Radio::Current()->GetRadioNum());
	}

	if (FAILED(hr))
	{
		logger->Log(LOG_ERROR, L"SoraURadioTx failed. ret = %x\r\n", hr);
		return -1;
	}
	else
	{
		if (log)
		{
			logger->Log(LOG_SUCCESS, L"success\n");		
		}
		return S_OK;
	}
}

HRESULT UmxSender::Deinit()
{
	if (!initialized)
		return -1;

	HRESULT hr = -1;
	HRESULT ret = S_OK;

	if (transferAllocated)
	{
		hr = SoraURadioTxFree(Radio::Current()->GetRadioNum(), txID);
		logger->Log(LOG_FUNC_CALL, L"SoraURadioTxFree called for radio %d\r\n", Radio::Current()->GetRadioNum());
		if (FAILED(hr))
		{
			logger->Log(LOG_ERROR, L"Failed. ret = %x\r\n", hr);
			ret = -1;
		}
		else
		{
			logger->Log(LOG_SUCCESS, L"success\r\n");
			txID = -1;
		}
		transferAllocated = false;
	}

	if (sampleBuf)
	{
		hr = SoraURadioUnmapTxSampleBuf(Radio::Current()->GetRadioNum(), (PVOID)sampleBuf);
		logger->Log(LOG_FUNC_CALL, L"SoraURadioUnmapTxSampleBuf called for radio %d\r\n", Radio::Current()->GetRadioNum());
		if (FAILED(hr))
		{
			logger->Log(LOG_ERROR, L"Failed. ret = %x\r\n", hr);		
			ret = -1;
		}
		else
		{
			logger->Log(LOG_SUCCESS, L"success\r\n");
			sampleBuf = 0;
		}
	}

	initialized = false;

	return ret;
}
