// DotsPlotWndProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "PlotWndPropDots.h"
#include "DotsPlotWndProperty.h"
#include "AppMessage.h"

// DotsPlotWndProperty

IMPLEMENT_DYNAMIC(DotsPlotWndProperty, BaseProperty)

DotsPlotWndProperty::DotsPlotWndProperty()
{

}

DotsPlotWndProperty::~DotsPlotWndProperty()
{
}


BEGIN_MESSAGE_MAP(DotsPlotWndProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// DotsPlotWndProperty message handlers




int DotsPlotWndProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	PlotWndPropDots * objProp = (PlotWndPropDots *)this->target;

	CMFCPropertyGridProperty * pProp;
	
	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Dots Window"));
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), objProp->GetName());
	this->AddProperty(pProp);
	
	pProp = new CMFCPropertyGridProperty(_T("X Label"), objProp->GetXLabel());
	this->AddProperty(pProp);
	
	pProp = new CMFCPropertyGridProperty(_T("Y Label"), objProp->GetYLabel());
	this->AddProperty(pProp);

	VARIANT_BOOL boolAutoScale;
	if (objProp->autoScale)
		boolAutoScale = VARIANT_TRUE;
	else
		boolAutoScale = VARIANT_FALSE;

	COleVariant varAutoScale((short)boolAutoScale, VT_BOOL);
	pProp = new CMFCPropertyGridProperty(_T("Auto Scale"), varAutoScale, _T("Enable/disable auto scale function for the plot window"));
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Max Value"), (_variant_t) objProp->GetMaxValue(), _T("Specifies the max value."));
	this->AddProperty(pProp);
	if (objProp->autoScale)
		pProp->Enable(FALSE);
	
	VARIANT_BOOL boolShowGrid;
	if (objProp->showGrid)
		boolShowGrid = VARIANT_TRUE;
	else
		boolShowGrid = VARIANT_FALSE;

	pProp = new CMFCPropertyGridProperty( _T("Dot Count"), (_variant_t) objProp->dotShowCount, _T("Specifies the number of dots to show on the plot window"));
	this->AddProperty(pProp);

	COleVariant varShowGrid((short)boolShowGrid, VT_BOOL);
	pProp = new CMFCPropertyGridProperty(_T("Show Grid"), varShowGrid, _T("Enable/disable grid on plot window"));
	this->AddProperty(pProp);

	VARIANT_BOOL boolLuminescence;
	if (objProp->luminescence)
		boolLuminescence = VARIANT_TRUE;
	else
		boolLuminescence = VARIANT_FALSE;

	COleVariant varLuminescence((short)boolLuminescence, VT_BOOL);
	pProp = new CMFCPropertyGridProperty(_T("Luminescence"), varLuminescence, _T("Enable/disable the luminescence effect for dots"));
	this->AddProperty(pProp);
	return 0;
}


void DotsPlotWndProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class
	PlotWndPropDots * objProp = (PlotWndPropDots *)this->target;

	if (wcscmp(pProp->GetName(), L"Name") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->SetName(value.bstrVal);
		objProp->nameIsSet = true;
	}
	else if (wcscmp(pProp->GetName(), L"Y Label") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->YLabel(value.bstrVal);
	}
	else if (wcscmp(pProp->GetName(), L"X Label") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->XLabel(value.bstrVal);
	}
	else if (wcscmp(pProp->GetName(), L"Auto Scale") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->autoScale = (value.boolVal != 0);
		if (objProp->autoScale)
			objProp->autoScaleMaxValue = 0;

		if (objProp->autoScale)
		{
			CMFCPropertyGridProperty * property = FindProperty(L"Max Value");
			property->Enable(FALSE);
		}
		else
		{
			CMFCPropertyGridProperty * property = FindProperty(L"Max Value");
			property->Enable(TRUE);			
		}
	}
	else if (wcscmp(pProp->GetName(), L"Max Value") == 0)
	{
		_variant_t value = pProp->GetValue();
		if (value.intVal < 1)
		{
			value.intVal = 1;
			pProp->SetValue(value);
		}
		objProp->SetMaxValue(value.intVal);
	}
	else if (wcscmp(pProp->GetName(), L"Dot Count") == 0)
	{
		_variant_t value = pProp->GetValue();
		if (value.intVal < 1)
		{
			value.intVal = 1;
			pProp->SetValue(value);
		}
		else if (value.intVal > PlotWndPropDots::MAX_DOTS_SHOWN)
		{
			value.intVal = PlotWndPropDots::MAX_DOTS_SHOWN;
			pProp->SetValue(value);
		}

		objProp->dotShowCount = value.intVal;
	}
	else if (wcscmp(pProp->GetName(), L"Show Grid") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->showGrid = (value.boolVal != 0);
	}
	else if (wcscmp(pProp->GetName(), L"Luminescence") == 0)
	{
		_variant_t value = pProp->GetValue();
		objProp->luminescence = (value.boolVal != 0);	
	}

	objProp->ForceUpdateGraph();
	return BaseProperty::OnPropertyChanged(pProp);
}

void DotsPlotWndProperty::UpdatePropertyValue(const wchar_t * propertyName)
{

	CMFCPropertyGridProperty * property = NULL;
	for (int i = 0; i < this->GetPropertyCount(); i++)
	{
		 CMFCPropertyGridProperty * aProperty = this->GetProperty(i);
		 if (wcscmp(aProperty->GetName(), propertyName) == 0)
		 {
			 property = aProperty;
			 break;
		 }
	}

	if (property == NULL)
		return;

	_variant_t value = property->GetValue();
	
	PlotWndPropDots * objProp = (PlotWndPropDots *)this->target;
	if (wcscmp(propertyName, L"Max Value") == 0)
	{
		_variant_t value = property->GetValue();
		value.intVal = (int)objProp->GetMaxValue();
		property->SetValue(value);
	}
	//else if (wcscmp(propertyName, L"Frame Count") == 0)
	//{
	//	_variant_t value = property->GetValue();
	//	value.intVal = (int)objProp->frameCount;
	//	property->SetValue(value);
	//}
}
