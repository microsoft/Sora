#include "stdafx.h"
#include "SeriesText.h"
//#include "DebugPlotViewerDoc.h"
#include "TextSeriesProperty.h"
#include "SubPlotWnd.h"
#include "HelperFunc.h"
#include "AppMessage.h"

/***********************************

Text Series

***********************************/

HRESULT TextSeriesProp::CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
{
	
	IXMLDOMElement *pssub=NULL;
	HRESULT hr=S_OK;

	CreateAndAddElementNode(pDom, L"SName", L"\n\t\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->name);
	//pssub->Release();
	CreateAndAddElementNode(pDom, L"SType", L"\n\t\t\t", pe, &pssub);
	pssub->put_text(_bstr_t(typeid(*this).name()));
	//pssub->Release();
	//CreateAndAddElementNode(pDom, L"colorIsSet", L"\n\t\t\t", pe, &pssub);
	//pssub->put_text((_bstr_t)this->colorIsSet);

	CreateAndAddElementNode(pDom, L"IsLog", L"\n\t\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->isLog);

	//pssub->Release();
	CreateAndAddElementNode(pDom, L"color", L"\n\t\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->color);
	CreateAndAddElementNode(pDom, L"replaceMode", L"\n\t\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->replaceMode);


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

TextSeriesProp::TextSeriesProp()
{
	isLog = false;
	replaceMode = true;
	//colorIsSet = false;
	color = RGB(0, 255, 0);	// green;

	_ringBuffer = new RingBufferWithTimeStamp<char>(1*1024*1024);
	_writeFilter = new SoraDbgPlot::FrameWithSizeInfoWriter<char>(_ringBuffer);

	_latestTimeIdx = 0;
	_dataRange = 1;

	::InitializeCriticalSection(&csLogBuf);
}

TextSeriesProp::~TextSeriesProp()
{
	::DeleteCriticalSection(&csLogBuf);

	delete _writeFilter;
	delete _ringBuffer;
}

BaseProperty * TextSeriesProp::GetPropertyPage()
{
	BaseProperty * property = new TextSeriesProperty;
	property->SetTarget(this);
	return property;
}

void TextSeriesProp::Close()
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

//void TextSeriesProp::UpdateView()
//{
//	if (!isLog)
//		SeriesProp::UpdateView();
//	else
//	{
//		SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)this->smSeriesInfo->GetAddress();
//		if (sharedSeriesInfo->dataElementSize == 0)
//			return;
//
//		char * dataAddr = (char *)this->smSeriesData->GetAddress();
//
//		int wIdx = sharedSeriesInfo->wIdx;
//		int rIdx = sharedSeriesInfo->rIdx;
//
//		int bufLen = sharedSeriesInfo->bufLen;
//		bufLen = bufLen - bufLen % sharedSeriesInfo->dataElementSize;
//
//		if (sharedSeriesInfo->replace)
//			sharedSeriesInfo->rIdx = 0;
//		else
//			sharedSeriesInfo->rIdx = wIdx;
//
//		int dataLen = wIdx - rIdx;
//		if (dataLen < 0)
//		{
//			dataLen += bufLen;
//		}
//
//		if (dataLen >= 0)
//		{
//			char * dataBuf = new char[dataLen+1];
//
//			for (int i = 0; i < dataLen; i++)
//			{
//				dataBuf[i] = dataAddr[(rIdx+i)%bufLen];
//			}
//
//			dataBuf[dataLen] = 0;
//
//			CString newStr(dataBuf);
//
//			::EnterCriticalSection(&csLogBuf);
//			if (logBuf.GetLength() < 32*1024)
//				logBuf.Append(newStr);
//			::LeaveCriticalSection(&csLogBuf);
//
//			delete [] dataBuf;
//		}
//	}
//}

void TextSeriesProp::Write(const void * ptr, size_t length)
{
	size_t dummy;
	_writeFilter->Write(ptr, length, dummy);	
}

size_t TextSeriesProp::DataSize()
{
	return _ringBuffer->RecordCount();
}

void TextSeriesProp::SeekTimeStamp(const std::vector<unsigned long long> & vecTimeStamp)
{
	bool taken = false;
	unsigned long long timestampOldest;
	size_t idxOldest;
	std::for_each(
		vecTimeStamp.begin(), 
		vecTimeStamp.end(), 
		[&taken, &timestampOldest, &idxOldest, this]
		(unsigned long long in) {
			unsigned long long out;
			size_t outIdx;
			bool found = this->_ringBuffer->GetNearestOldTimeStamp(in, out, outIdx);
			if (found)
			{
				if (!taken)
				{
					taken = true;
					timestampOldest = out;
					idxOldest = outIdx;
				}
				else
				{
					if (timestampOldest > out)
					{
						timestampOldest = out;
						idxOldest = outIdx;
					}
				}
			}
	});

	if (taken)
	{
		_latestTimeIdx = idxOldest;
		_dataRange = 1;
		
	}
	else
	{
		_latestTimeIdx = -1;
	}
}

void TextSeriesProp::SeekDataRanage(size_t idx, size_t range)
{
	_latestTimeIdx = idx;
	_dataRange = range;
}

void TextSeriesProp::UpdateSubWnd()
{
	SubPlotWnd * wnd = dynamic_cast<SubPlotWnd *>(this->GetTargetWnd());
	if (wnd)
	{
		if (_latestTimeIdx >= 0)
		{
			size_t sizeOfData;
			bool succ = _ringBuffer->GetDataSizeByTimeStampIdx(_latestTimeIdx, sizeOfData);

			if (succ)
			{
				char * data = new char[sizeOfData + 1];
				size_t readSize;
				succ = _ringBuffer->ReadDataByTimeStampIdx(_latestTimeIdx, data, sizeOfData, readSize);
				if (succ)
				{
					data[readSize] = 0;
					wnd->PlotText(CString(data));
				}
				delete [] data;
			}
		}
		else
		{
			wnd->PlotText(CString(""));
		}
	}
}

SubPlotWnd * TextSeriesProp::CreateSubPlotWnd()
{
	auto subWnd = new SubPlotWnd;
	//subWnd->seriesProp = this;
	this->SetTargetWnd(subWnd);

	subWnd->EventMoveWindow.Subscribe([this](const void * sender, const CRect & rect){
		this->rect = rect;
	});

	subWnd->StrategyGetColor.Set([this](const void * sender, const int & dummy, COLORREF & color){
		color = this->color;
		//if (this->colorIsSet)
		//	color = this->color;
		//else
		//	color = RGB(0, 255, 0);
	});

	return subWnd;
}

char * TextSeriesProp::GetData(size_t index)
{
	size_t dataSize = this->DataSize();

	if (dataSize <= index || index < 0)
		return false;

	size_t sizeOfData;
	bool succ = _ringBuffer->GetDataSizeByTimeStampIdx(index, sizeOfData);

	if (succ)
	{
		char * data = new char[sizeOfData + 1];
		size_t readSize;
		succ = _ringBuffer->ReadDataByTimeStampIdx(index, data, sizeOfData, readSize);
		if (succ)
		{
			data[readSize] = 0;
			return data;
		}
		else
		{
			delete [] data;
			return 0;
		}
	}
	else
		return 0;
}

void TextSeriesProp::ClearData()
{
	this->_ringBuffer->Reset();
}

bool TextSeriesProp::Export(const CString & filename, bool bAll)
{
	FILE * fp;
	errno_t ret = _wfopen_s(&fp, filename, L"wb");

	if (ret == 0)
	{
		this->LockData();

		if (1)
		{
			this->_ringBuffer->Export([fp](const char * ptr, size_t length){
				fwrite(ptr, 1, length, fp);
			});
		}

		this->UnlockData();

		fclose(fp);

		return true;
	}

	return false;
}
