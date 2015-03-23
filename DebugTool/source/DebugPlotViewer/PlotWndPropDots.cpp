#include "stdafx.h"
//#include "DebugPlotViewerDoc.h"
#include "SeriesDots.h"
#include "PlotWndPropDots.h"
#include "PlotWnd.h"
#include "HelperFunc.h"
#include "BaseProperty.h"
#include "DotsPlotWndProperty.h"
#include "SeriesDots.h"

/***********************************

Dots Plot Wnd

************************************/

// configration
const size_t PlotWndPropDots::MAX_RANGE = 1024;

HRESULT PlotWndPropDots::CreateElementPlot(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
{
	HRESULT hr=S_OK;
	IXMLDOMElement *pSeries=NULL;
	IXMLDOMElement *psub=NULL;
	CreateAndAddElementNode(pDom, L"WndName", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->GetName());
	CreateAndAddElementNode(pDom, L"WndNameIsSet", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->nameIsSet);
	//psub->Release();
	CreateAndAddElementNode(pDom, L"WndType", L"\n\t\t", pe, &psub);
	psub->put_text(_bstr_t(typeid(*this).name()));
	//psub->Release();
	CreateAndAddElementNode(pDom, L"WndautoScale", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->autoScale);
	//psub->Release();
	CreateAndAddElementNode(pDom, L"WndmaxValue", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->maxValue);
	//psub->Release();
	CreateAndAddElementNode(pDom, L"Wndluminescence", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->luminescence);
	//psub->Release();
	CreateAndAddElementNode(pDom, L"WndshowGrid", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->showGrid);
	//psub->Release();
	CreateAndAddElementNode(pDom, L"DotCount", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->dotShowCount);

	CreateAndAddElementNode(pDom, L"WndRectTLX", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->rect.TopLeft().x);

	CreateAndAddElementNode(pDom, L"WndRectTLY", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->rect.TopLeft().y);

	CreateAndAddElementNode(pDom, L"WndRectBRX", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->rect.BottomRight().x);

	CreateAndAddElementNode(pDom, L"WndRectBRY", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->rect.BottomRight().y);

	std::vector<SeriesProp *>::iterator iterSeries;
	for (iterSeries = this->seriesInPlotWndProp.begin();
		iterSeries !=this->seriesInPlotWndProp.end();
		iterSeries++)
	{
		CreateAndAddElementNode(pDom, L"Series", L"\n\t\t", pe, &pSeries);
		(*iterSeries)->CreateElementSeries(pDom,pSeries);
		pSeries->Release();
	}

	return hr;
}

PlotWndPropDots::PlotWndPropDots()
{
	autoScale = true;
	maxValue = 100;
	autoScaleMaxValue = maxValue;
	showGrid = true;
	luminescence = true;
	dotShowCount = 64;
	frameCount = DebuggingProcessProp::REPLAY_BUF_LEN;
}

BaseProperty * PlotWndPropDots::GetPropertyPage()
{
	BaseProperty * property = new DotsPlotWndProperty;
	property->SetTarget(this);
	return property;
}

Bitmap * PlotWndPropDots::CreateBitmap()
{
	CRect rect(0, 0, this->ClientWidth(), this->ClientHeight());
	Bitmap * bmp = new Bitmap(rect.right,rect.bottom);

	Graphics* memGraph = Graphics::FromImage(bmp);

	CRect clientRect;
	this->DrawBackground(memGraph);
	this->DrawLabel(memGraph);
	this->GetCanvasRect(clientRect);

	bool taken = false;
	double maxValue = 0;

	if (clientRect.right > clientRect.left && clientRect.bottom > clientRect.top)
	{

		if (autoScale)
		{
			std::vector<SeriesProp *>::iterator iter;
			for (iter = seriesInPlotWndProp.begin();
				iter != seriesInPlotWndProp.end();
				iter++)
			{
				SeriesProp * seriesProp = *iter;
				if (typeid(*seriesProp) == typeid(DotsSeriesProp))
				{
					DotsSeriesProp * seriesProp = (DotsSeriesProp *)(*iter);

					double max;
					bool succ = seriesProp->CalcMax(clientRect, this->LatestIdx(), this->RangeSize(), max);
					if (succ)
					{
						if (!taken)
						{
							taken = true;
							maxValue = max;
						}
						else
						{
							maxValue = max(maxValue, max);
						}
					}
				}
			}

			if (taken)
			{
				int lastDispMaxValue = this->GetDispMaxValue();
				int targetMaxValue = (int)(maxValue * 1.2);
				if (lastDispMaxValue < targetMaxValue || lastDispMaxValue > targetMaxValue * 1.5)
				{
					this->SetDispMaxValue((int)(maxValue * 1.2));			
				}
			}
		}


		if (showGrid)
		{
			this->DrawGrid(memGraph, clientRect);
		}

		std::vector<SeriesProp *>::iterator iter;
		for (iter = seriesInPlotWndProp.begin();
			iter != seriesInPlotWndProp.end();
			iter++)
		{
			SeriesProp * seriesProp = *iter;
			if (typeid(*seriesProp) == typeid(DotsSeriesProp))
			{
				DotsSeriesProp * seriesProp = (DotsSeriesProp *)(*iter);
				seriesProp->Draw(memGraph, clientRect, this->LatestIdx(), this->RangeSize(), this->luminescence, this->GetDispMaxValue());
			}
		}
	}
	delete memGraph;
	return bmp;
}

size_t PlotWndPropDots::MaxDataSize()
{
	size_t maxSize = 0;
	std::for_each(
		this->seriesInPlotWndProp.begin(),
		this->seriesInPlotWndProp.end(), 
		[&maxSize](SeriesProp * series){
			DotsSeriesProp * lSeries = dynamic_cast<DotsSeriesProp *>(series);
			if (lSeries != 0)
			{
				size_t size = lSeries->DataSize();
				if (maxSize < size)
					maxSize = size;
			}
	});

	return maxSize;
}

void PlotWndPropDots::DrawGrid(Graphics * g, const CRect & clientRect)
{
	int ycenter = (clientRect.top + clientRect.bottom) / 2;
	int xcenter = (clientRect.left + clientRect.right) / 2;
	int xleft = clientRect.left;
	int xright = clientRect.right;
	int ytop = clientRect.top;
	int ybottom = clientRect.bottom;

	Pen pen(Color(255, 50, 50, 50));
	pen.SetDashStyle(DashStyleDash);

	// horizantal line
	g->DrawLine(&pen, xleft, ycenter, xright, ycenter);

	// vertical line
	g->DrawLine(&pen, xcenter, ytop, xcenter, ybottom);

	// outer ellipse
	g->DrawEllipse(
		&pen, 
		xleft, 
		ytop, 
		clientRect.Width() - 1, 
		clientRect.Height() - 1);

	// innner ellipse

	if ( min(clientRect.Width(), clientRect.Height() ) > 100 )
	{
		g->DrawEllipse(
			&pen,
			clientRect.right / 4 + xleft,
			clientRect.bottom / 4 + ytop,
			clientRect.Width() / 2 - 1,
			clientRect.Height() / 2 - 1);

		// draw text
		SolidBrush fontBrush(Color(255, 150, 150, 150));
		StringFormat format;
		format.SetAlignment(StringAlignmentNear);
		CString strValue;
		::FormatCommaString(strValue, this->GetDispMaxValue());
		CString strDisp;
		strDisp.Format(L"r = %s", strValue);
		Gdiplus::Font gridFont(L"Arial", 10);
		PointF pointF((Gdiplus::REAL)(xleft + 5), (Gdiplus::REAL)(ybottom - 16));
		g->DrawString(strDisp, -1, &gridFont, pointF, &format, &fontBrush);
	}

	this->DrawXAxis(g, clientRect);
}

void PlotWndPropDots::DrawBackground(Graphics * g)
{
	CRect clientRect(0, 0, this->ClientWidth(), this->ClientHeight());

	SolidBrush brush(Color(255, 0, 0, 0));
	g->FillRectangle(&brush, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
}


void PlotWndPropDots::DrawDot(Graphics * g, COMPLEX16 data, Color& color)
{
	SolidBrush brush(color);
	g->FillEllipse(&brush, data.re-2, data.im-2, 5, 5);
}


void PlotWndPropDots::ModifyDataRange(size_t & range)
{
	if (range > MAX_RANGE)
		range = MAX_RANGE;
}

bool PlotWndPropDots::Accept(SeriesProp * series)
{
	if ( (this->seriesInPlotWndProp.size() > 0) &&
		series->GetProcess() != this->seriesInPlotWndProp[0]->GetProcess())
		return false;

	vector<SeriesProp *>::iterator iter;
	for (iter = this->seriesInPlotWndProp.begin();
		iter != this->seriesInPlotWndProp.end();
		iter++)
	{
		if (*iter == series)
			return false;
	}

	if ( (typeid(*series) == typeid(DotsSeriesProp)) || (typeid(*series) == typeid(TextSeriesProp)))
		return true;
	else
		return false;
}


