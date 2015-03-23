// TemplateProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "TemplateProperty.h"


// TemplateProperty

IMPLEMENT_DYNAMIC(TemplateProperty, BaseProperty)

TemplateProperty::TemplateProperty()
{

}

TemplateProperty::~TemplateProperty()
{
}


BEGIN_MESSAGE_MAP(TemplateProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// TemplateProperty message handlers




int TemplateProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}


void TemplateProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class

	return BaseProperty::OnPropertyChanged(pProp);
}
