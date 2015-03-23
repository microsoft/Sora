#include "sora.h"
#include "Receive.h"
#include "UmxCommonOps.h"

HRESULT ReceiveLoop(SoraThread * thread, IDataProcessor * processor, Radio * radio);

UMXReceiveThread::UMXReceiveThread(Radio * radio, IThreadControl * threadCallback)
{
	this->processor = 0;
	SetCallback(threadCallback);
}

void UMXReceiveThread::SetProcessor(IDataProcessor * processor)
{
	this->processor = processor;
}

void UMXReceiveThread::Threadfunc()
{
	//RX_BLOCK block;
	//while (!CheckStatus())
	//{
	//	if (processor)
	//		processor->ProcessData(&block, 1);	
	//}

	//return;
	//while(!CheckStatus())
	//{
	//	RX_BLOCK block;

	//	if (processor)
	//		processor->ProcessData(&block, 1);
	//}

	HRESULT hResult;
	int ret = 0;

	hResult = ::SoraUmxInit(Radio::Current());
	if (FAILED(hResult))
	{
		return;
	}

	Radio::Current()->SoraUmxConfigRadio();

	UmxReceiver * receiver = new UmxReceiver(Radio::Current());
	if (!receiver)
	{
		Logger::GetLogger(L"umx")->Log(LOG_ERROR, L"Cannot new UmxReceiver object.\n");
		goto EXIT;
	}

	hResult = receiver->Init();
	if (FAILED(hResult))
		goto EXIT_DEINIT;

	Logger * logger = Logger::GetLogger(L"umx");

	bool rxLogEnabled = true;

	logger->Log(LOG_INFO, L"Start receiving signal...\n");

	while(!CheckStatus())
	{
		RX_BLOCK block;

		hResult = receiver->ReceiveBlock(&block, rxLogEnabled);

		if (rxLogEnabled)
		{
			if (hResult == S_OK)
				logger->Log(LOG_INFO, L"Repeated calls to reception-related functions will not be logged.\n");		
			rxLogEnabled = false;
		}

		if (FAILED(hResult))
			break;

		if (processor)
			processor->ProcessData(&block, 1);
	}

	logger->Log(LOG_INFO, L"End receiving signal.\n");

EXIT_DEINIT:
	receiver->Deinit();
EXIT:

	::SoraUmxDeinit(Radio::Current());
	if (receiver)
		delete receiver;

	return;
	//::ReceiveLoop(this, processor, radio);
}

UmxReceiver::UmxReceiver(Radio * radio)
{
	logger = Logger::GetLogger(L"umx");
	rxBuf = 0;
	rxBufSize = 0;
	sampleStreamAllocated = false;
	rxBufferMapped = false;
}

HRESULT UmxReceiver::Init()
{
	HRESULT hr = -1;

	// 1. map receive buffer
	hr = SoraURadioMapRxSampleBuf(
		Radio::Current()->GetRadioNum(),
		&rxBuf,
		&rxBufSize
		);

	logger->Log(LOG_FUNC_CALL, L"SoraURadioMapRxSampleBuf called for radio %d\r\n", Radio::Current()->GetRadioNum());
	if(FAILED(hr))
	{
		logger->Log(LOG_ERROR, L"Failed. ret = %x\r\n", hr);
		return -1;
	}
	else
	{
		logger->Log(LOG_SUCCESS, L"success\r\n");
		rxBufferMapped = true;
	}

	hr = SoraURadioAllocRxStream(
		&SampleStream,
		Radio::Current()->GetRadioNum(),
		(PUCHAR)rxBuf,
		rxBufSize
		);

	logger->Log(LOG_FUNC_CALL, L"SoraURadioAllocRxStream called for radio %d\r\n", Radio::Current()->GetRadioNum());

	if (SUCCEEDED(hr))
	{
		sampleStreamAllocated = true;
		logger->Log(LOG_SUCCESS, L"success\n");
	}
	else
	{
		logger->Log(LOG_ERROR, L"Failed. ret = %x", hr);
	}

	
	return S_OK;
}

HRESULT UmxReceiver::ReceiveBlock(PRX_BLOCK block, bool log)
{
	if (!sampleStreamAllocated)
		return -1;

	HRESULT ret = S_OK;

	PRX_BLOCK pbScanPoint = SoraRadioGetRxStreamPos(&SampleStream);

	if (log)
	{
		logger->Log(LOG_FUNC_CALL, L"SoraRadioGetRxStreamPos called\n");
	}

	FLAG bTouched = 0;

	HRESULT hr = 
		SoraCheckSignalBlock(pbScanPoint, SoraGetStreamVStreamMask(&SampleStream), SORA_MAX_RX_SPIN_WAIT * 8, &bTouched);

	if (log)
	{
		logger->Log(LOG_FUNC_CALL, L"SoraGetStreamVStreamMask called\n");
		logger->Log(LOG_FUNC_CALL, L"SoraCheckSignalBlock called\n");
	}


	if (FAILED(hr)) 
	{
		if ( hr == E_FETCH_SIGNAL_HW_TIMEOUT )
		{
			logger->Log(LOG_ERROR, L"SoraCheckSignalBlock timeout.\n");	
			logger->Log(LOG_ERROR, L"Please disable and enable HwTest driver to solve the problem.\n");

			ret = hr;
			goto EXIT;
		}
		else
		{
			logger->Log(LOG_ERROR, L"SoraCheckSignalBlock failed. ret = 0x%x\r\n", hr);
			ret = -1;
			goto EXIT;
		}
	}
	else
	{
		if (log)
		{
			logger->Log(LOG_SUCCESS, L"success\n");
		}
	}

	*block = *(PRX_BLOCK)pbScanPoint;

	pbScanPoint = __SoraRadioIncRxStreamPointer(&SampleStream, pbScanPoint);

	if (log)
	{
		logger->Log(LOG_FUNC_CALL, L"__SoraRadioGetRxStreamNextPos called\n");
	}

	SoraRadioSetRxStreamPos(&SampleStream, pbScanPoint);

	if (log)
	{
		logger->Log(LOG_FUNC_CALL, L"__SoraRadioSetRxStreamPos called\n");
	}

EXIT:
	return ret;
}

HRESULT UmxReceiver::Deinit()
{
	HRESULT hr;

	if (sampleStreamAllocated)
	{
		SoraURadioReleaseRxStream(&SampleStream, Radio::Current()->GetRadioNum());
		logger->Log(LOG_FUNC_CALL, L"SoraURadioReleaseRxStream called for radio %d\r\n", Radio::Current()->GetRadioNum());
		sampleStreamAllocated = false;
	}

	if (rxBuf)
	{
		hr = SoraURadioUnmapRxSampleBuf(Radio::Current()->GetRadioNum(), rxBuf);
		rxBuf = 0;
		rxBufSize = 0;
		rxBufferMapped = false;
		logger->Log(LOG_FUNC_CALL, L"SoraURadioUnmapRxSampleBuf called for radio %d\r\n", Radio::Current()->GetRadioNum());
		if (FAILED(hr))
		{
			logger->Log(LOG_ERROR, L"Failed. ret = %x\r\n", hr);		
		}
		else
		{
			logger->Log(LOG_SUCCESS, L"success\r\n");
		}
	}

	return S_OK;
}

