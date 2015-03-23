// PlotWndAreaProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "PlotWndAreaProperty.h"
#include "AppMessage.h"

// PlotWndAreaProperty

IMPLEMENT_DYNAMIC(PlotWndAreaProperty, BaseProperty)

PlotWndAreaProperty::PlotWndAreaProperty()
{

}

PlotWndAreaProperty::~PlotWndAreaProperty()
{
}


BEGIN_MESSAGE_MAP(PlotWndAreaProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// PlotWndAreaProperty message handlers




int PlotWndAreaProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	PlotWndAreaProp * objProp = (PlotWndAreaProp *)this->target;
	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty( _T("Grid Size"), (_variant_t) objProp->gridSize, _T("Specifies the snapping grid size"));
	this->AddProperty(pProp);

	return 0;
}

void PlotWndAreaProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class
	PlotWndAreaProp * objProp = (PlotWndAreaProp *)this->target;	

	if (wcscmp(pProp->GetName(), L"Grid Size") == 0)
	{
		_variant_t value = pProp->GetValue();
		if (value.intVal < 1)
		{
			value.intVal = 1;
			pProp->SetValue(value);
		}
		else if (value.intVal > 40)
		{
			value.intVal = 40;
			pProp->SetValue(value);
		}
		objProp->gridSize = value.intVal;
	}

	return BaseProperty::OnPropertyChanged(pProp);
}
