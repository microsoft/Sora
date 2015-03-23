#include "stdafx.h"
//#include "DebugPlotViewerDoc.h"
#include "SeriesDots.h"
#include "PlotWndPropDots.h"
#include "HelperFunc.h"
#include "DotsSeriesProperty.h"
#include "AppMessage.h"

/***********************************

Dots Series

***********************************/

HRESULT DotsSeriesProp::CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
{
	//MSXML2::IXMLDOMElementPtr pSeries=NULL;
	IXMLDOMElement *pssub=NULL;
	HRESULT  hr=S_OK;

	CreateAndAddElementNode(pDom, L"SName", L"\n\t\t\t", pe, &pssub);
	pssub->put_text((_bstr_t)this->name);
	//pssub->Release();
	CreateAndAddElementNode(pDom, L"SType", L"\n\t\t\t", pe, &pssub);
	pssub->put_text(_bstr_t(typeid(*this).name()));
	//pssub->Release();
	//CreateAndAddElementNode(pDom, L"colorIsSet", L"\n\t\t\t", pe, &pssub);
	//pssub->put_text((_bstr_t)this->colorIsSet);
	//pssub->Release();
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

DotsSeriesProp::DotsSeriesProp()
{
	//colorIsSet = false;
	color = RGB(0, 255, 0);	// green;

	_ringBuffer = new RingBufferWithTimeStamp<COMPLEX16>(1*1024*1024);
	_writeFilter = new SoraDbgPlot::FrameWithSizeInfoWriter<COMPLEX16>(_ringBuffer);
}

DotsSeriesProp::~DotsSeriesProp()
{
	delete _writeFilter;
	delete _ringBuffer;	
}

BaseProperty * DotsSeriesProp::GetPropertyPage()
{
	BaseProperty * property = new DotsSeriesProperty;
	property->SetTarget(this);
	return property;
}

void DotsSeriesProp::Close()
{
	CWnd * targetWnd = this->GetTargetWnd();
	if (targetWnd)
	{
		targetWnd->PostMessage(WM_APP, CMD_DATA_CLEAR, (LPARAM)this);
		targetWnd->InvalidateRgn(NULL, 1);
		SetTargetWnd(0);
	}

	SeriesProp::Close();
}

void DotsSeriesProp::Draw(Graphics * g, const CRect & rect, size_t start, size_t size, bool luminescence, double dispMax)
{
	CRect rectClient = rect;

	double oldRange = dispMax * 2;
	int newRangeX = rectClient.Width();
	int newRangeY = rectClient.Height();

	Color color;
	color.SetFromCOLORREF(this->color);

	//if (this->colorIsSet)
	//	color.SetFromCOLORREF(this->color);
	//else
	//	color.SetFromCOLORREF(RGB(0, 255,0));

	int redColor = color.GetRed();
	int greenColor = color.GetGreen();
	int blueColor = color.GetBlue();

	for (size_t i = 0; i < size; i++)
	{
		size_t mappedIdx = start + i;
		if (mappedIdx >= _ringBuffer->Size() || mappedIdx < 0)
			continue;
		COMPLEX16 y = (*_ringBuffer)[mappedIdx];
		int alphaDot;
		if (luminescence)
			alphaDot = 255 * (size - i) / size;
		else
			alphaDot = 255;

		Color colorFrame(alphaDot, redColor, greenColor, blueColor);

		Pen pen(colorFrame);

		Gdiplus::REAL re = float((y.re - (-dispMax)) * newRangeX / oldRange + 0);
		Gdiplus::REAL im = float((rectClient.bottom - (y.im - (-dispMax)) * newRangeY / oldRange));

		g->DrawLine(&pen, re-1.0f, im, re+1.0f, im);
		g->DrawLine(&pen, re, im-1.0f, re, im+1.0f);
	}
}

bool DotsSeriesProp::CalcMax(const CRect & rect, size_t start, size_t size, double & max)
{
	double maxValue = 0;

	bool initialized = false;

	for (size_t i = 0; i < size; i++)
	{
		size_t mappedIdx = start + i;
		if (mappedIdx >= _ringBuffer->Size() || mappedIdx < 0)
			continue;

		COMPLEX16 y = (*_ringBuffer)[mappedIdx];
		double value = (double)(y.re * y.re + y.im * y.im);

		if (!initialized)
		{
			maxValue = value;
			initialized = true;
		}
		else
		{
			maxValue = max(maxValue, value);
		}
	}

	if (initialized)
	{
		max = sqrt(maxValue);
	}

	return initialized;
}

void DotsSeriesProp::Write(const void * ptr, size_t length)
{
	size_t dummy;
	_writeFilter->Write(ptr, length, dummy);
}


void DotsSeriesProp::ClearData()
{
	this->_ringBuffer->Reset();
}

size_t DotsSeriesProp::DataSize()
{
	return _ringBuffer->Size();
}

bool DotsSeriesProp::GetTimeStamp(size_t index, unsigned long long & out)
{
	this->LockData();
	bool succ = _ringBuffer->GetTimeStampBySample(index, out);
	this->UnlockData();

	return succ;
}

bool DotsSeriesProp::Export(const CString & filename, bool bAll)
{
	FILE * fp;
	errno_t ret = _wfopen_s(&fp, filename, L"wb");

	if (ret == 0)
	{
		char * digitBuf = new char[128];

		this->LockData();

		if (1)
		{
			this->_ringBuffer->Export([fp, digitBuf](const COMPLEX16 * ptr, size_t length){
				while(length > 0)
				{
					fprintf(fp, "%d %d\n", (*ptr).re, (*ptr).im);
					ptr++;
					length --;
				}
			});
		}
		else
		{
			auto plotWnd = (BitmapTypePlotWndProp *)this->GetPlotWndProp();
			size_t start = plotWnd->LatestIdx();
			size_t length = plotWnd->RangeSize();
			length = min(length, _ringBuffer->Size());

			this->_ringBuffer->ExportRange(start, length, [fp, digitBuf](const COMPLEX16 * ptr, size_t length){
				while(length > 0)
				{
					fprintf(fp, "%d %d\n", (*ptr).re, (*ptr).im);
					ptr++;
					length --;
				}
			});		
		}

		this->UnlockData();

		delete [] digitBuf;

		fclose(fp);

		return true;
	}

	return false;
}
