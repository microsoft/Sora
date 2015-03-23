#include "stdafx.h"
#include <sstream>
#include "SeriesLog.h"
#include "LogSeriesProperty.h"
#include "LogWithFileBackup.h"
#include "HelperFunc.h"
#include "AppMessage.h"

/***********************************

Log Series

***********************************/
int LogSeriesProp::logObjIdx = 0;

LogSeriesProp::LogSeriesProp()
{
	int logObjIdx = LogSeriesProp::logObjIdx++;

	color = RGB(0, 255, 0);	// green;
	char dir[256];
	::GetCurrentDirectoryA(256, dir);
	std::ostringstream os;
	os << dir << "\\logs\\" << this->name << logObjIdx;

	ostringstream osName;
	osName << logObjIdx;
	logObj = LogWithBackUpFile::Make(os.str().c_str(), osName.str().c_str());

	_writeFilter = new SoraDbgPlot::FrameWithSizeInfoWriter<char>();
	
	_writeFilter->EventFlushData.Subscribe([this](const void * sender, const SoraDbgPlot::FrameWithSizeInfoWriter<char>::FlushDataEvent & e){

		size_t dataLen = e.length;
		if (dataLen >= 0)
		{
			char * dataBuf = new char[dataLen+1];
			memcpy(dataBuf, e.ptr, dataLen);
			dataBuf[dataLen] = 0;

			//::EnterCriticalSection(&csLogBuf);
			this->logObj->AddRecord(dataBuf);
			//::LeaveCriticalSection(&csLogBuf);

			delete [] dataBuf;
		}		
	});

	//::InitializeCriticalSection(&csLogBuf);
}

LogSeriesProp::~LogSeriesProp()
{
	//::DeleteCriticalSection(&csLogBuf);
	delete logObj;
	delete _writeFilter;
}

BaseProperty * LogSeriesProp::GetPropertyPage()
{
	BaseProperty * property = new LogSeriesProperty;
	property->SetTarget(this);
	return property;
}

HRESULT LogSeriesProp::CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
{
	IXMLDOMElement *pssub=NULL;
	HRESULT hr=S_OK;

	CreateAndAddElementNode(pDom, L"SName", L"\n\t\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->name);
	//pssub->Release();
	CreateAndAddElementNode(pDom, L"SType", L"\n\t\t\t", pe, &pssub);
	pssub->put_text(_bstr_t(typeid(*this).name()));

	CreateAndAddElementNode(pDom, L"color", L"\n\t\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->color);

	CreateAndAddElementNode(pDom, L"subRectTLX", L"\n\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->rect.TopLeft().x);

	CreateAndAddElementNode(pDom, L"subRectTLY", L"\n\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->rect.TopLeft().y);

	CreateAndAddElementNode(pDom, L"subRectBRX", L"\n\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->rect.BottomRight().x);

	CreateAndAddElementNode(pDom, L"subRectBRY", L"\n\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->rect.BottomRight().y);

	return hr;	
}

//void LogSeriesProp::UpdateView()
//{
//	ASSERT(FALSE);
//	/*SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)this->smSeriesInfo->GetAddress();
//	if (sharedSeriesInfo->dataElementSize == 0)
//		return;
//
//	char * dataAddr = (char *)this->smSeriesData->GetAddress();
//
//	int wIdx = sharedSeriesInfo->wIdx;
//	int rIdx = sharedSeriesInfo->rIdx;
//
//	int bufLen = sharedSeriesInfo->bufLen;
//	bufLen = bufLen - bufLen % sharedSeriesInfo->dataElementSize;
//
//	if (sharedSeriesInfo->replace)
//		sharedSeriesInfo->rIdx = 0;
//	else
//		sharedSeriesInfo->rIdx = wIdx;
//
//	int dataLen = wIdx - rIdx;
//	if (dataLen < 0)
//	{
//		dataLen += bufLen;
//	}
//
//	if (dataLen >= 0)
//	{
//		char * dataBuf = new char[dataLen+1];
//		char * dataPtr = dataBuf;
//
//		::EnterCriticalSection(&csLogBuf);
//		for (int i = 0; i < dataLen; i++)
//		{
//			dataBuf[i] = dataAddr[(rIdx+i)%bufLen];
//			if (dataBuf[i] == '\n')
//			{
//				dataBuf[i+1] = 0;
//				this->logObj->AddRecord(dataPtr);
//				dataPtr = dataBuf + i + 1;
//
//			}
//		}
//		::LeaveCriticalSection(&csLogBuf);
//
//		delete [] dataBuf;
//	}*/
//}

void LogSeriesProp::Close()
{
	CWnd * targetWnd = this->GetTargetWnd();
	if (targetWnd)
	{
		LRESULT destroyed = targetWnd->SendMessage(WM_APP, CMD_SERIES_CLOSED, 0);
		if (! destroyed)
		{
			targetWnd->PostMessage(WM_APP, CMD_DATA_CLEAR, (LPARAM)this);
			targetWnd->InvalidateRgn(NULL, 1);		
		}
		SetTargetWnd(0);
	}

	SeriesProp::Close();
}

COLORREF LogSeriesProp::GetColor()
{
	return color;
}

void LogSeriesProp::SetColor(COLORREF color)
{
	this->color = color;
}

char * LogSeriesProp::Record(int index)
{
	//::EnterCriticalSection(&csLogBuf);
	char * ptr = logObj->Record(index);
	//::LeaveCriticalSection(&csLogBuf);
	return ptr;
}

size_t LogSeriesProp::RecordCount()
{
	//::EnterCriticalSection(&csLogBuf);
	size_t size = logObj->RecordCount();
	//::LeaveCriticalSection(&csLogBuf);
	return size;
}

void LogSeriesProp::Write(const void * ptr, size_t length)
{
	size_t sizeDummy;
	this->_writeFilter->Write(ptr, length, sizeDummy);
	//size_t dataLen = length;
	//if (dataLen >= 0)
	//{
	//	char * dataBuf = new char[dataLen+1];
	//	memcpy(dataBuf, ptr, length);
	//	dataBuf[dataLen] = 0;

	//	::EnterCriticalSection(&csLogBuf);
	//	this->logObj->AddRecord(dataBuf);
	//	::LeaveCriticalSection(&csLogBuf);

	//	delete [] dataBuf;
	//}
}

size_t LogSeriesProp::DataSize()
{
	this->LockData();
	size_t size = logObj->RecordCount();
	this->UnlockData();
	return size;
}


char * LogSeriesProp::GetData(size_t index)
{
	char * str;
	//::EnterCriticalSection(&csLogBuf);
	this->LockData();
	size_t size = logObj->RecordCount();
	str = logObj->Record(size - 1 - index);
	this->UnlockData();
	//::LeaveCriticalSection(&csLogBuf);
	return str;
}

void LogSeriesProp::ClearData()
{
	//this->_ringBuffer->Reset();
}

bool LogSeriesProp::Export(const CString & f, bool bAll)
{
	return this->logObj->Export(f);
}
