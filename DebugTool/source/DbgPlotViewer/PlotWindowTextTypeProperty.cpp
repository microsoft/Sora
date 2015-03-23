// PlotWindowTextTypeProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "PlotWindowTextTypeProperty.h"

using namespace std;

// PlotWindowTextTypeProperty

IMPLEMENT_DYNAMIC(PlotWindowTextTypeProperty, BaseProperty)

PlotWindowTextTypeProperty::PlotWindowTextTypeProperty(const wstring & typeName, const wstring & name)
{
	_typename = typeName;
	_name = name;
}

PlotWindowTextTypeProperty::~PlotWindowTextTypeProperty()
{
}


BEGIN_MESSAGE_MAP(PlotWindowTextTypeProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// PlotWindowTextTypeProperty message handlers




int PlotWindowTextTypeProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Type"), _typename.c_str());
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), _name.c_str());
	this->AddProperty(pProp);

	return 0;
}


void PlotWindowTextTypeProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	if (wcscmp(pProp->GetName(), L"Name") == 0)
	{
		_variant_t value = pProp->GetValue();
		wstring name = value.bstrVal;
		EventName.Raise(this, name);
	}

	return BaseProperty::OnPropertyChanged(pProp);
}

void PlotWindowTextTypeProperty::OnCloseProperty()
{
	this->EventName.Reset();
}
