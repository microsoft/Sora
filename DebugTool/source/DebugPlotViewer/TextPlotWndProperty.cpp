// TextPlotWndProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "TextPlotWndProperty.h"
#include "AppMessage.h"
#include "SeriesText.h"
#include "PlotWndPropText.h"

// TextPlotWndProperty

IMPLEMENT_DYNAMIC(TextPlotWndProperty, BaseProperty)

TextPlotWndProperty::TextPlotWndProperty()
{

}

TextPlotWndProperty::~TextPlotWndProperty()
{
}


BEGIN_MESSAGE_MAP(TextPlotWndProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// TextPlotWndProperty message handlers




int TextPlotWndProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	PlotWndPropText * objProp = (PlotWndPropText *)this->target;

	CMFCPropertyGridProperty * pProp;
	
	bool isLog = false;
	if (objProp->seriesInPlotWndProp.size() > 0)
	{
		TextSeriesProp * textSeriesProp = dynamic_cast<TextSeriesProp *>(objProp->seriesInPlotWndProp[0]);
		if (textSeriesProp && textSeriesProp->isLog)
			isLog = true;
	}

	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Text Window"));
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), objProp->GetName());
	this->AddProperty(pProp);

	return 0;
}


void TextPlotWndProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class
	PlotWndPropText * objProp = (PlotWndPropText *)this->target;

	if (wcscmp(pProp->GetName(), L"Name") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->SetName(value.bstrVal);
		objProp->nameIsSet = true;
	}
	/*else if (wcscmp(pProp->GetName(), L"Frame Count") == 0)
	{
		_variant_t value = pProp->GetValue();
		if (value.intVal < 1)
		{
			value.intVal = 1;
			pProp->SetValue(value);
		}
		else if (value.intVal > objProp->seriesInPlotWndProp[0]->GetProcess()->replayCapacity)
		{
			value.intVal = objProp->seriesInPlotWndProp[0]->GetProcess()->replayCapacity;
			pProp->SetValue(value);
		}

		objProp->frameCount = value.intVal;
	}*/

	objProp->ForceUpdateGraph();

	return BaseProperty::OnPropertyChanged(pProp);
}
