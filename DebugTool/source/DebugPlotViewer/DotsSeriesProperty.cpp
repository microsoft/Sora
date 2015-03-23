// DotsSeriesProperty.cpp : implementation file
//

#include "stdafx.h"
#include "SeriesDots.h"
#include "DebugPlotViewer.h"
#include "DotsSeriesProperty.h"


// DotsSeriesProperty

IMPLEMENT_DYNAMIC(DotsSeriesProperty, BaseProperty)

DotsSeriesProperty::DotsSeriesProperty()
{

}

DotsSeriesProperty::~DotsSeriesProperty()
{
}


BEGIN_MESSAGE_MAP(DotsSeriesProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// DotsSeriesProperty message handlers




int DotsSeriesProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	DotsSeriesProp * seriesProp = (DotsSeriesProp *)this->target;
	ASSERT(seriesProp != 0);

	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Dots Channel"));
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


void DotsSeriesProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class
	DotsSeriesProp * seriesProp = (DotsSeriesProp *)this->target;

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
