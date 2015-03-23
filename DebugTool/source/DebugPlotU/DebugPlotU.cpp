#include <assert.h>
#include "AppSettings.h"
#include "DebugPlotU.h"
#include "_share_lock_if.h"
#include "SharedChannelW.h"
#include "SmChannel.h"
#include "ChannelManager.h"
#include "SharedNameManagement.h"
#include "SharedSerialNumGenerator.h"
#include "ProcessGlobal.h"
#include "ChannelObjLookuUp.h"


ProcessGlobal * g_callforinitialization = ProcessGlobal::Instance();

// TODO
static 	SoraDbgPlot::Common::SharedSerialNumGenerator generator(
		SharedNameManager::GetSerialNumGeneratorName()
		);

//-------------------------------------------------------------------------

HRESULT __stdcall DebugPlotInit()
{
	return S_OK;
}

void __stdcall DebugPlotDeinit()
{
	ChannelObjLookUp::Clean();

	ProcessGlobal::Release();
}

HRESULT __stdcall PlotText(
	__in const char * channelName,
	__in const char * format,
	...
)
{
	if ( (channelName == 0) ||
		 (format == 0) )
	{
		return E_INVALID_PARAMETER;
	}

	SharedChannelW * ch = ChannelObjLookUp::GetSharedChannel(channelName, CH_TYPE_TEXT, 0);

	if (!ch->IsType(CH_TYPE_TEXT))
		return E_PLOT_TYPE_MISMATCH;

	ch->Lock();

	const size_t TEXT_BUF_LEN = 4096;
	char * textBuffer = ch->GetTextBuffer(TEXT_BUF_LEN);

	va_list ap;
	va_start(ap, format);
	_vsnprintf_s(textBuffer, TEXT_BUF_LEN, _TRUNCATE, format, ap);
	va_end(ap);	

	ch->Write(textBuffer, strlen(textBuffer));

	ch->Unlock();

	return S_OK;
}

HRESULT __stdcall PlotLine(
	__in const char * channelName,
	__in const int * data,
	__in int dataCount
)
{

	if ( (channelName == 0) ||
		 (data == 0)		||
		 (dataCount <= 0) )
	{
		return E_INVALID_PARAMETER;
	}

	SharedChannelW * ch = ChannelObjLookUp::GetSharedChannel(channelName, CH_TYPE_LINE, 0);

	if (!ch->IsType(CH_TYPE_LINE))
		return E_PLOT_TYPE_MISMATCH;

	ch->Lock();

	ch->Write((const char *)data, dataCount * sizeof(int));

	ch->Unlock();

	return S_OK;
}

HRESULT __stdcall PlotSpectrum(
	__in const char * channelName,
	__in const int * data,
	__in int dataCount
)
{
	if ( (channelName == 0) ||
		 (data == 0)		||
		 (dataCount <= 0) )
	{
		return E_INVALID_PARAMETER;
	}

	SharedChannelW * ch = ChannelObjLookUp::GetSharedChannel(channelName, CH_TYPE_SPECTRUM, dataCount);
	
	if (!ch->IsType(CH_TYPE_SPECTRUM))
		return E_PLOT_TYPE_MISMATCH;

	if (ch->SpectrumSize() != dataCount)
		return E_SPECTRUM_SIZE_INVALID;

	ch->Lock();

	ch->Write((const char *)data, dataCount * sizeof(int));

	ch->Unlock();

	return S_OK;
}

HRESULT __stdcall PlotDots(
	__in const char * channelName,
	__in const COMPLEX16 * data,
	__in int dataCount
)
{
	if ( (channelName == 0) ||
		 (data == 0)		||
		 (dataCount <= 0) )
	{
		return E_INVALID_PARAMETER;
	}

	SharedChannelW * ch = ChannelObjLookUp::GetSharedChannel(channelName, CH_TYPE_DOTS, 0);

	if (!ch->IsType(CH_TYPE_DOTS))
		return E_PLOT_TYPE_MISMATCH;

	ch->Lock();

	ch->Write((const char *)data, dataCount * sizeof(COMPLEX16));

	ch->Unlock();

	return S_OK;
}

HRESULT __stdcall Log(
	__in const char * channelName,
	__in const char * format,
	...
)
{
	if ( (channelName == 0) ||
		 (format == 0) )
	{
		return E_INVALID_PARAMETER;
	}

	SharedChannelW * ch = ChannelObjLookUp::GetSharedChannel(channelName, CH_TYPE_LOG, 0);

	if (!ch->IsType(CH_TYPE_LOG))
		return E_PLOT_TYPE_MISMATCH;

	ch->Lock();

	const size_t TEXT_BUF_LEN = 4096;
	char * textBuffer = ch->GetTextBuffer(TEXT_BUF_LEN);

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	_snprintf_s(
		textBuffer, 
		TEXT_BUF_LEN, 
		_TRUNCATE, 
		"[%d:%d:%.3f]: ",
		sysTime.wHour, 
		sysTime.wMinute,
		(double)(sysTime.wSecond*1000 + sysTime.wMilliseconds)/1000);

	size_t size1 = strlen(textBuffer);
	char * ptr2 = textBuffer + size1;

	va_list ap;
	va_start(ap, format);
	_vsnprintf_s(ptr2, TEXT_BUF_LEN - size1, _TRUNCATE, format, ap);
	va_end(ap);	

	ch->Write((const char *)textBuffer, strlen(textBuffer));

	ch->Unlock();

	return S_OK;
}

HRESULT __stdcall TracebufferReadData(
	__in COMPLEX16 * pData,
	__in int inNum,
	__out int * pOutNum
)
{
	HRESULT ret = S_OK;

	if ( (pData == 0) || (inNum <= 0) )
		return E_INVALID_PARAMETER;

	SmProcess * process = ProcessGlobal::Instance()->GetProcess();
	if (!process)
	{
		ret = E_ALLOCATION_FAIL;
		goto RET;
	}

	int numRead = process->ReadRawData(pData, inNum);
	if (numRead  < inNum)
		ret = E_END_OF_BUF;

	if (pOutNum)
	{
		*pOutNum = numRead;
	}

RET:
	return ret;
}


HRESULT __stdcall TracebufferWriteData(
	__in COMPLEX16 * pData,
	__in int inNum,
	__out int * pOutNum
)
{
	HRESULT ret = S_OK;

	if ( (pData == 0) || (inNum <= 0) )
		return E_INVALID_PARAMETER;

	SmProcess * process = ProcessGlobal::Instance()->GetProcess();
	if (!process)
	{
		ret = E_ALLOCATION_FAIL;
		goto RET;
	}

	int numWritten = process->WriteRawData(pData, inNum);
	if (numWritten  < inNum)
		ret = E_END_OF_BUF;

	if (pOutNum)
	{
		*pOutNum = numWritten;
	}

RET:
	return ret;
}

void __stdcall TracebufferClear()
{
	ShareMem * smSourceInfo = ProcessGlobal::Instance()->GetProcess()->GetSourceInfo();
	if (!smSourceInfo)
		return;

	smSourceInfo->Lock(INFINITE);

	SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)smSourceInfo->GetAddress();
	sharedSourceInfo->wIdx = 0;
	sharedSourceInfo->rIdx = 0;

	smSourceInfo->Unlock();
}

DWORD __stdcall WaitForViewer(
	__in DWORD milliseconds
)
{
	ShareLock * event = ProcessGlobal::Instance()->GetProcess()->GetEvent();
	event->ResetShareEvent();
	return event->Lock(milliseconds);	// WaitForSingleObject internally
}

void __stdcall PauseViewer()
{
	ProcessGlobal::Instance()->GetProcess()->Pause();
}
