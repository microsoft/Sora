#include "stdafx.h"
//#include "DebugPlotViewerDoc.h"
#include "SeriesSpectrum.h"
#include "PlotWndPropSpectrum.h"
#include "PlotWnd.h"
#include "SubBitmapPlotWnd.h"
#include "HelperFunc.h"
#include "BaseProperty.h"
#include "SpectrumPlotWndProperty.h"
#include "SeriesSpectrum.h"

/***********************************

Line Plot Wnd

************************************/
PlotWndPropSpectrum::PlotWndPropSpectrum()
{
}

HRESULT PlotWndPropSpectrum::CreateElementPlot(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
{
	HRESULT hr;
	hr=S_OK;
	IXMLDOMElement *pSeries=NULL;
	IXMLDOMElement *psub=NULL;

	CreateAndAddElementNode(pDom, L"Logarithm", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->isLog);

	CreateAndAddElementNode(pDom, L"WndName", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->GetName());

	CreateAndAddElementNode(pDom, L"WndNameIsSet", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->nameIsSet);
	//psub->Release();
	//no release
	CreateAndAddElementNode(pDom, L"WndType", L"\n\t\t", pe, &psub);
	psub->put_text(_bstr_t(typeid(*this).name()));
	//psub->Release();
	CreateAndAddElementNode(pDom, L"WndautoScale", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->autoScale);
	//psub->Release();
	CreateAndAddElementNode(pDom, L"WndmaxValue", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->maxValue);
	//psub->Release();
	CreateAndAddElementNode(pDom, L"WndminValue", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->minValue);
	psub->Release();
	CreateAndAddElementNode(pDom, L"WndshowGrid", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->showGrid);
	//psub->Release();
	CreateAndAddElementNode(pDom, L"WndFrameCount", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->frameCount);
	//psub->Release();
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

BaseProperty * PlotWndPropSpectrum::GetPropertyPage()
{
	BaseProperty * property = new SpectrumPlotWndProperty;
	property->SetTarget(this);
	return property;
}

bool PlotWndPropSpectrum::Accept(SeriesProp * series)
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

	if ( (typeid(*series) == typeid(SpectrumSeriesProp)) || (typeid(*series) == typeid(TextSeriesProp)))
		return true;
	else
		return false;
}

void PlotWndPropSpectrum::ModifyDataRange(size_t & range)
{
	//range = 1;
}

