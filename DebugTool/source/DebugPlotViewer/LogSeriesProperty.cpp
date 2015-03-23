// LogSeriesProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "LogSeriesProperty.h"
#include "SeriesLog.h"

// LogSeriesProperty

IMPLEMENT_DYNAMIC(LogSeriesProperty, BaseProperty)

LogSeriesProperty::LogSeriesProperty()
{

}

LogSeriesProperty::~LogSeriesProperty()
{
}


BEGIN_MESSAGE_MAP(LogSeriesProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// LogSeriesProperty message handlers

int LogSeriesProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	LogSeriesProp * seriesProp = (LogSeriesProp *)this->target;

	ASSERT(seriesProp != 0);

	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Log Channel"));
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), seriesProp->name);
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Color"), seriesProp->GetColor(), NULL);
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	this->AddProperty(pColorProp);

	return 0;
}

void LogSeriesProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	LogSeriesProp * seriesProp = (LogSeriesProp *)this->target;

	if (wcscmp(pProp->GetName(), L"Color") == 0)
	{
		COLORREF color = ((CMFCPropertyGridColorProperty *)pProp)->GetColor();
		seriesProp->SetColor(color);
	}
	else 
	{
		ASSERT(false);
	}

	seriesProp->ForceUpdateGraph();

	return BaseProperty::OnPropertyChanged(pProp);
}
