#include "stdafx.h"
//#include "DebugPlotViewerDoc.h"
#include "SeriesSpectrum.h"
#include "PlotWndPropLine.h"
#include "HelperFunc.h"
#include "SpectrumSeriesProperty.h"
#include "AppMessage.h"

/******************************

Line Series

*******************************/
HRESULT SpectrumSeriesProp::CreateElementSeries(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
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

void SpectrumSeriesProp::Write(const void * ptr, size_t length)
{
	size_t dummy;
	_writeFilter->Write(ptr, length, dummy);
}

size_t SpectrumSeriesProp::DataSize()
{
	return _ringBuffer->Size();
}


bool SpectrumSeriesProp::GetTimeStamp(size_t index, unsigned long long & out)
{
	this->LockData();
	bool succ = _ringBuffer->GetTimeStampBySample(index, out);
	this->UnlockData();

	return succ;
}

bool SpectrumSeriesProp::GetData(size_t index, int & out)
{
	out = (*_ringBuffer)[index];
	return true;	
}

void SpectrumSeriesProp::Draw(Graphics * g, const CRect & rect, size_t start, size_t size)
{
	size_t sizeData = this->GetSpectrumDataWidth();
	size = sizeData;
	start = start / sizeData * sizeData;

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

void SpectrumSeriesProp::DrawDot(Graphics * g, double x, double y, const Pen & pen)
{
	int xx = (int)x;
	int yy = (int)y;

	g->DrawLine(&pen, xx - 3, yy, xx + 3, yy);
	g->DrawLine(&pen, xx, yy - 3, xx, yy + 3);
}

SpectrumSeriesProp::SpectrumSeriesProp()
{
	//colorIsSet = false;
	color = RGB(0, 255, 0);	// green;

	maxValue = 0;
	minValue = 0;
	spectrumDataSize = 0;

	_ringBuffer = new RingBufferWithTimeStamp<__int32>(1*1024*1024);
	_writeFilter = new SoraDbgPlot::FrameWithSizeInfoWriter<__int32>(_ringBuffer);
}

SpectrumSeriesProp::~SpectrumSeriesProp()
{
	delete _writeFilter;
	delete _ringBuffer;
}

BaseProperty * SpectrumSeriesProp::GetPropertyPage()
{
	BaseProperty * property = new SpectrumSeriesProperty;
	property->SetTarget(this);
	return property;
}

void SpectrumSeriesProp::Close()
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

bool SpectrumSeriesProp::GetMaxMinRange(size_t in, size_t & out)
{
	out = this->GetSpectrumDataWidth();
	return true;
}

size_t SpectrumSeriesProp::GetSpectrumDataWidth()
{
	return _dataWidth;
}

void SpectrumSeriesProp::SetSpectrumDataWidth(size_t dataWidth)
{
	_dataWidth = dataWidth;
}

void SpectrumSeriesProp::OnSmInfoSet(SharedSeriesInfo * sharedSeriesInfo)
{
	this->SetSpectrumDataWidth(sharedSeriesInfo->spectrumDataSize);
}

void SpectrumSeriesProp::OnSmInfoRemoved()
{
}

void SpectrumSeriesProp::ClearData()
{
	this->_ringBuffer->Reset();
}

bool SpectrumSeriesProp::Export(const CString & filename, bool bAll)
{
	FILE * fp;
	errno_t ret = _wfopen_s(&fp, filename, L"wb");

	if (ret == 0)
	{
		size_t spectrumSize = this->GetSpectrumDataWidth();

		char * digitBuf = new char[128];
		//int * numBuf = new int[spectrumSize];
		size_t numOutput = 0;

		this->LockData();

		if (1)
		{
			this->_ringBuffer->Export([fp, digitBuf, &numOutput, spectrumSize](const int * ptr, size_t length){
				while(length > 0)
				{
					fprintf(fp, "%d ", *ptr);
					numOutput++;
					if (numOutput == spectrumSize)
					{
						fprintf(fp, "\n");
						numOutput = 0;
					}
					ptr++;
					length --;
				}
			});
		}
		else
		{
			auto plotWnd = (BitmapTypePlotWndProp *)this->GetPlotWndProp();
			size_t start = plotWnd->LatestIdx();
			size_t length = spectrumSize;
			length = min(length, _ringBuffer->Size());

			this->_ringBuffer->ExportRange(start, length, [fp, digitBuf, &numOutput, spectrumSize](const int * ptr, size_t length){
				while(length > 0)
				{
					fprintf(fp, "%d ", *ptr);
					numOutput++;
					if (numOutput == spectrumSize)
					{
						fprintf(fp, "\n");
						numOutput = 0;
					}
					ptr++;
					length --;
				}
			});
		}

		this->UnlockData();

		//delete [] numBuf;
		delete [] digitBuf;

		fclose(fp);

		return true;
	}

	return false;
}
