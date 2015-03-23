// LineSeriesProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SpectrumSeriesProperty.h"
#include "SeriesSpectrum.h"

// LineSeriesProperty

IMPLEMENT_DYNAMIC(SpectrumSeriesProperty, BaseProperty)

SpectrumSeriesProperty::SpectrumSeriesProperty()
{

}

SpectrumSeriesProperty::~SpectrumSeriesProperty()
{
}


BEGIN_MESSAGE_MAP(SpectrumSeriesProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// SpectrumSeriesProperty message handlers




int SpectrumSeriesProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	SpectrumSeriesProp * seriesProp = (SpectrumSeriesProp *)this->target;
	ASSERT(seriesProp != 0);

	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Spectrum Channel"));
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), seriesProp->name);
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Color"), seriesProp->color, NULL);
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	this->AddProperty(pColorProp);

	return 0;
}

void SpectrumSeriesProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class

	SpectrumSeriesProp * seriesProp = (SpectrumSeriesProp *)this->target;

	if (wcscmp(pProp->GetName(), L"Color") == 0)
	{
		COLORREF color = ((CMFCPropertyGridColorProperty *)pProp)->GetColor();
		seriesProp->color = color;
		//seriesProp->colorIsSet = true;
	}
	else 
	{
		ASSERT(false);	// impossible
	}
	
	seriesProp->ForceUpdateGraph();

	return BaseProperty::OnPropertyChanged(pProp);
}
