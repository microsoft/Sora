// TextSeriesProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "TextSeriesProperty.h"
#include "SeriesText.h"

// TextSeriesProperty

IMPLEMENT_DYNAMIC(TextSeriesProperty, BaseProperty)

TextSeriesProperty::TextSeriesProperty()
{

}

TextSeriesProperty::~TextSeriesProperty()
{
}


BEGIN_MESSAGE_MAP(TextSeriesProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// TextSeriesProperty message handlers




int TextSeriesProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	TextSeriesProp * seriesProp = (TextSeriesProp *)this->target;

	ASSERT(seriesProp != 0);

	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Text Channel"));

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

	//if (!seriesProp->isLog)
	//{
	//	CString strMode;
	//	if (seriesProp->replaceMode)
	//		strMode.SetString(L"Replace");
	//	else
	//		strMode.SetString(L"Append");

	//	pProp = new CMFCPropertyGridProperty(_T("Display Mode"), strMode.GetBuffer());
	//	pProp->AddOption(_T("Replace"));
	//	pProp->AddOption(_T("Append"));
	//	pProp->AllowEdit(FALSE);
	//	this->AddProperty(pProp);
	//}

	return 0;
}


void TextSeriesProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class
	TextSeriesProp * seriesProp = (TextSeriesProp *)this->target;

	if (wcscmp(pProp->GetName(), L"Color") == 0)
	{
		COLORREF color = ((CMFCPropertyGridColorProperty *)pProp)->GetColor();
		seriesProp->color = color;
		//seriesProp->colorIsSet = true;
	}
	//else if (wcscmp(pProp->GetName(), L"Display Mode") == 0)
	//{
	//	_variant_t value = pProp->GetValue();
	//	if (wcscmp(value.bstrVal, L"Replace") == 0)
	//	{
	//		seriesProp->replaceMode = true;
	//	}
	//	else
	//	{
	//		seriesProp->replaceMode = false;
	//	}
	//	//if (seriesProp->smSeriesInfo)
	//	//{
	//	//	SharedSeriesInfo * sharedSeriesInfo = 
	//	//		(SharedSeriesInfo *)seriesProp->smSeriesInfo->GetAddress();
	//	//	sharedSeriesInfo->replace = seriesProp->replaceMode;
	//	//}
	//}
	else 
	{
		ASSERT(false);	// impossible
	}

	
	seriesProp->ForceUpdateGraph();

	return BaseProperty::OnPropertyChanged(pProp);
}
