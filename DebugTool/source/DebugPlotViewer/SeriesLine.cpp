#include "stdafx.h"
//#include "DebugPlotViewerDoc.h"
#include "SeriesLine.h"
#include "PlotWndPropLine.h"
#include "HelperFunc.h"
#include "LineSeriesProperty.h"
#include "AppMessage.h"

/******************************

Line Series

*******************************/
HRESULT LineSeriesProp::CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
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

void LineSeriesProp::Write(const void * ptr, size_t length)
{	
	size_t dummy;
	_writeFilter->Write(ptr, length, dummy);
}

size_t LineSeriesProp::DataSize()
{
	return _ringBuffer->Size();
}

void LineSeriesProp::ClearData()
{
	this->_ringBuffer->Reset();
}

bool LineSeriesProp::GetTimeStamp(size_t index, unsigned long long & out)
{
	this->LockData();
	bool succ = _ringBuffer->GetTimeStampBySample(index, out);
	this->UnlockData();

	return succ;
}

bool LineSeriesProp::GetData(size_t index, int & out)
{
	out = (*_ringBuffer)[index];
	return true;
}

void LineSeriesProp::Draw(Graphics * g, const CRect & rect, size_t start, size_t size)
{
	bool bDrawDot = (size > 0) && (rect.Width() / size >= 5);
	bool bDrawType2 = (size > 0) && (rect.Width() / size >= 1);

	PlotWndPropLine * plotWndProp = (PlotWndPropLine *)(this->GetPlotWndProp());
	int width = rect.Width();

	if (width == 0)
		return;

	Gdiplus::REAL lastDispX, lastDispY;

	Color color;
	color.SetFromCOLORREF(this->color);

	Color colorLine;

	if (bDrawDot)
		colorLine = Color(64, color.GetR(), color.GetG(), color.GetB());
	else
		colorLine = Color(255, color.GetR(), color.GetG(), color.GetB());

	Pen penLine(colorLine);

	if (!bDrawType2)
	{
		bool firstPoint = true;
		for (int i = 0; i < width; i++)
		{
			size_t mappedIdx = start + size - 1 - i * size / width;
			if (mappedIdx >= _ringBuffer->Size())
				continue;

			int y = (*_ringBuffer)[mappedIdx];
			double dataT = ::TransformCoordinate(y, plotWndProp->isLog);
			Gdiplus::REAL dataY = plotWndProp->GetClientY(dataT, rect);
			Gdiplus::REAL dataX = (Gdiplus::REAL)(i + rect.left);

			if (!firstPoint)
				g->DrawLine(&penLine, lastDispX, lastDispY, dataX, dataY);
			else
				firstPoint = false;

			lastDispX = dataX;
			lastDispY = dataY;
		}
	}

	if (bDrawType2)
	{
		bool firstPoint = true;
		Pen penDot(color, 1.5);
		for (size_t i = 0; i < size; i++)
		{
			if (i + start >= _ringBuffer->Size())
				break;

			int y = (*_ringBuffer)[i + start];
			double dataT = ::TransformCoordinate(y, plotWndProp->isLog);
			Gdiplus::REAL dataY = plotWndProp->GetClientY(dataT, rect);
			Gdiplus::REAL dataX = (Gdiplus::REAL)((size - i - 1) * rect.Width() / size + rect.left);

			if (bDrawDot)
				this->DrawDot(g, dataX, dataY, penDot);

			if (firstPoint)
			{
				firstPoint = false;
			}
			else
			{
				g->DrawLine(&penLine, lastDispX, lastDispY, dataX, dataY);
			}
			lastDispX = dataX;
			lastDispY = dataY;
		}
	}
}

void LineSeriesProp::DrawDot(Graphics * g, double x, double y, const Pen & pen)
{
	int xx = (int)x;
	int yy = (int)y;

	g->DrawLine(&pen, xx - 3, yy, xx + 3, yy);
	g->DrawLine(&pen, xx, yy - 3, xx, yy + 3);
}

LineSeriesProp::LineSeriesProp()
{
	//colorIsSet = false;
	color = RGB(0, 255, 0);	// green;

	_ringBuffer = new RingBufferWithTimeStamp<__int32>(1*1024*1024);
	_writeFilter = new SoraDbgPlot::FrameWithSizeInfoWriter<__int32>(_ringBuffer);
}

LineSeriesProp::~LineSeriesProp()
{
	delete _writeFilter;
	delete _ringBuffer;
}

BaseProperty * LineSeriesProp::GetPropertyPage()
{
	BaseProperty * property = new LineSeriesProperty;
	property->SetTarget(this);
	return property;
}

void LineSeriesProp::Close()
{
	CWnd * targetWnd = this->GetTargetWnd();
	if (targetWnd)
	{
		targetWnd->PostMessage(WM_APP, CMD_DATA_CLEAR, (LPARAM)this);
		targetWnd->InvalidateRgn(NULL, 1);
		this->SetTargetWnd(0);
	}

	SeriesProp::Close();
}

bool LineSeriesProp::GetMaxMinRange(size_t in, size_t & out)
{
	out = in;
	return true;
}

bool LineSeriesProp::Export(const CString & filename, bool bAll)
{
	FILE * fp;
	errno_t ret = _wfopen_s(&fp, filename, L"wb");

	if (ret == 0)
	{
		char * digitBuf = new char[128];

		this->LockData();

		if (1)
		{
			this->_ringBuffer->Export([fp, digitBuf](const int * ptr, size_t length){
				while(length > 0)
				{
					fprintf(fp, "%d\n", *ptr);
					ptr ++;
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

			this->_ringBuffer->ExportRange(start, length, [fp, digitBuf](const int * ptr, size_t length){
				while(length > 0)
				{
					fprintf(fp, "%d\n", *ptr);
					ptr ++;
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
