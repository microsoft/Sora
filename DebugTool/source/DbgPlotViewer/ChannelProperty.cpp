// ChannelProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "ChannelProperty.h"

using namespace std;

// ChannelProperty

IMPLEMENT_DYNAMIC(ChannelProperty, BaseProperty)

ChannelProperty::ChannelProperty(const wstring & typeName, const wstring & name, COLORREF color)
{
	_typename = typeName;
	_name = name;
	_color = color;
}

ChannelProperty::~ChannelProperty()
{
}


BEGIN_MESSAGE_MAP(ChannelProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// ChannelProperty message handlers


int ChannelProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Type"), _typename.c_str());
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), _name.c_str());
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Color"), _color, NULL);
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	this->AddProperty(pColorProp);

	return 0;
}


void ChannelProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class

	if (wcscmp(pProp->GetName(), L"Color") == 0)
	{
		COLORREF color = ((CMFCPropertyGridColorProperty *)pProp)->GetColor();
		EventColor.Raise(0, color);
	}
	else 
	{
		ASSERT(false);	// impossible
	}

	return BaseProperty::OnPropertyChanged(pProp);
}

void ChannelProperty::OnCloseProperty()
{
	EventColor.Reset();
}
