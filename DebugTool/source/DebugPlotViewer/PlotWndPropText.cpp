#include "stdafx.h"
#include "PlotWndPropText.h"
#include "SeriesText.h"
#include "HelperFunc.h"
#include "TextPlotWndProperty.h"

char * PlotWndPropText::GetText(size_t index)
{
	if (this->seriesInPlotWndProp.size() == 0)
		return false;

	auto seriesText = dynamic_cast<TextSeriesProp *>(this->seriesInPlotWndProp[0]);
	seriesText->LockData();
	size_t size = seriesText->DataSize();
	char * data = seriesText->GetData(size - 1 - index);
	seriesText->UnlockData();
	return data;
}

HRESULT PlotWndPropText::CreateElementPlot(IXMLDOMDocument *pDom, IXMLDOMElement *pe)
{
	HRESULT hr=S_OK;
	IXMLDOMElement *pSeries=NULL;
	IXMLDOMElement *psub=NULL;
	CreateAndAddElementNode(pDom, L"WndName", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->GetName());
	CreateAndAddElementNode(pDom, L"WndNameIsSet", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->nameIsSet);
	//psub->Release();
	//no release
	CreateAndAddElementNode(pDom, L"WndType", L"\n\t\t", pe, &psub);
	psub->put_text(_bstr_t(typeid(*this).name()));
	CreateAndAddElementNode(pDom, L"WndFrameCount", L"\n\t\t", pe, &psub);
	psub->put_text((_bstr_t)this->frameCount);

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

BaseProperty * PlotWndPropText::GetPropertyPage()
{
	BaseProperty * property = new TextPlotWndProperty;
	property->SetTarget(this);
	return property;
}
