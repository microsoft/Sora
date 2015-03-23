// LogPlotWndProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "LogPlotWndProperty.h"
#include "AppMessage.h"
#include "PlotWndPropLog.h"

// LogPlotWndProperty

IMPLEMENT_DYNAMIC(LogPlotWndProperty, BaseProperty)

LogPlotWndProperty::LogPlotWndProperty()
{

}

LogPlotWndProperty::~LogPlotWndProperty()
{
}


BEGIN_MESSAGE_MAP(LogPlotWndProperty, CMFCPropertyGridCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// LogPlotWndProperty message handlers




int LogPlotWndProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	PlotWndPropLog * objProp = (PlotWndPropLog *)this->target;

	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Name"), objProp->GetName());
	this->AddProperty(pProp);

	return 0;
}


void LogPlotWndProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class
	PlotWndPropLog * objProp = (PlotWndPropLog *)this->target;

	if (wcscmp(pProp->GetName(), L"Name") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->SetName(value.bstrVal);
		objProp->nameIsSet = true;
	}

	objProp->ForceUpdateGraph();

	return BaseProperty::OnPropertyChanged(pProp);
}
