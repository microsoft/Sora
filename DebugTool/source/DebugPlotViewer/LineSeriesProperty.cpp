// LineSeriesProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "LineSeriesProperty.h"
#include "SeriesObj.h"
#include "SeriesLine.h"

// LineSeriesProperty

IMPLEMENT_DYNAMIC(LineSeriesProperty, BaseProperty)

LineSeriesProperty::LineSeriesProperty()
{

}

LineSeriesProperty::~LineSeriesProperty()
{
}


BEGIN_MESSAGE_MAP(LineSeriesProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// LineSeriesProperty message handlers




int LineSeriesProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	LineSeriesProp * seriesProp = (LineSeriesProp *)this->target;
	ASSERT(seriesProp != 0);

	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Line Channel"));
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), seriesProp->name);
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	//pProp = new CMFCPropertyGridProperty( _T("Frame Count"), (_variant_t) seriesProp->frameCount, _T("Number of frames per screen"));
	//this->AddProperty(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Color"), seriesProp->color, NULL);
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	this->AddProperty(pColorProp);

	return 0;
}

void LineSeriesProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class

	LineSeriesProp * seriesProp = (LineSeriesProp *)this->target;

	if (wcscmp(pProp->GetName(), L"Color") == 0)
	{
		COLORREF color = ((CMFCPropertyGridColorProperty *)pProp)->GetColor();
		seriesProp->color = color;
		//seriesProp->colorIsSet = true;
	}
	//else if (wcscmp(pProp->GetName(), L"Frame Count") == 0)
	//{
	//	_variant_t value = pProp->GetValue();
	//	seriesProp->frameCount = value.intVal;
	//}
	else 
	{
		ASSERT(false);	// impossible
	}
	
	seriesProp->ForceUpdateGraph();

	return BaseProperty::OnPropertyChanged(pProp);
}
