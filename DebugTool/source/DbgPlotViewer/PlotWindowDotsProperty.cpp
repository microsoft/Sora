// PlotWindowDotsProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "PlotWindowDotsProperty.h"
#include "BitmapTypeSettings.h"
#include "SettingsPlotWindow.h"

using namespace std;

// PlotWindowDotsProperty

IMPLEMENT_DYNAMIC(PlotWindowDotsProperty, BaseProperty)

PlotWindowDotsProperty::PlotWindowDotsProperty(
	const std::wstring name, 
	const std::wstring xLabel, 
	const std::wstring yLabel,
	bool bAutoScale,
	double maxValue,
	size_t dataCount,
	bool bShowGrid,
	bool bLog,
	bool bLuminescence
	)
{
	_name = name;
	_xLabel = xLabel;
	_yLabel = yLabel;
	_bAutoScale = bAutoScale;
	_maxValue = maxValue;
	_dataCount = dataCount;
	_bShowGrid = bShowGrid;
	_bLog = bLog;
	_bLuminescence = bLuminescence;
}

PlotWindowDotsProperty::~PlotWindowDotsProperty()
{
}


BEGIN_MESSAGE_MAP(PlotWindowDotsProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// PlotWindowDotsProperty message handlers




int PlotWindowDotsProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	CMFCPropertyGridProperty * pProp;
	
	pProp = new CMFCPropertyGridProperty(_T("Type"), _T("Dots Window"));
	pProp->Enable(FALSE);
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Name"), _name.c_str());
	this->AddProperty(pProp);
	
	pProp = new CMFCPropertyGridProperty(_T("X Label"), _xLabel.c_str());
	this->AddProperty(pProp);
	
	pProp = new CMFCPropertyGridProperty(_T("Y Label"), _yLabel.c_str());
	this->AddProperty(pProp);

	VARIANT_BOOL boolAutoScale;
	if (_bAutoScale)
		boolAutoScale = VARIANT_TRUE;
	else
		boolAutoScale = VARIANT_FALSE;

	COleVariant varAutoScale((short)boolAutoScale, VT_BOOL);
	pProp = new CMFCPropertyGridProperty(_T("Auto Scale"), varAutoScale, _T("Enable/disable auto scale function for the plot window"));
	this->AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Max Value"), (_variant_t) _maxValue, _T("Specifies the max value."));
	this->AddProperty(pProp);
	if (_bAutoScale)
		pProp->Enable(FALSE);

	pProp = new CMFCPropertyGridProperty( _T("Data Count"), (_variant_t) _dataCount, _T("Specifies the data count."));
	this->AddProperty(pProp);

	VARIANT_BOOL boolShowGrid;
	if (_bShowGrid)
		boolShowGrid = VARIANT_TRUE;
	else
		boolShowGrid = VARIANT_FALSE;

	COleVariant varShowGrid((short)boolShowGrid, VT_BOOL);
	pProp = new CMFCPropertyGridProperty(_T("Show Grid"), varShowGrid, _T("Enable/disable grid on plot window"));
	this->AddProperty(pProp);

	CString strMode;
	if (_bLog)
		strMode.SetString(L"Logarithm");
	else
		strMode.SetString(L"General");

	VARIANT_BOOL boolLuminescence;
	if (_bLuminescence)
		boolLuminescence = VARIANT_TRUE;
	else
		boolLuminescence = VARIANT_FALSE;

	COleVariant varLuminescence((short)boolLuminescence, VT_BOOL);
	pProp = new CMFCPropertyGridProperty(_T("Luminescence"), varLuminescence, _T("Enable/disable the luminescence effect for dots"));
	this->AddProperty(pProp);

	return 0;
}


void PlotWindowDotsProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	// TODO: Add your specialized code here and/or call the base class

	if (wcscmp(pProp->GetName(), L"Name") == 0)
	{
		_variant_t value = pProp->GetValue();
		wstring name = value.bstrVal;
		EventName.Raise(this, name);
	}
	else if (wcscmp(pProp->GetName(), L"Y Label") == 0)
	{
		_variant_t value = pProp->GetValue();
		wstring yLabel = value.bstrVal;
		EventYLabel.Raise(this, yLabel);
	}
	else if (wcscmp(pProp->GetName(), L"X Label") == 0)
	{
		_variant_t value = pProp->GetValue();
		wstring xLabel = value.bstrVal;
		EventXLabel.Raise(this, xLabel);
	}
	else if (wcscmp(pProp->GetName(), L"Auto Scale") == 0)
	{
		_variant_t value = pProp->GetValue();
		bool autoScale = (value.boolVal != 0);
		EventAutoScale.Raise(this, autoScale);

		CMFCPropertyGridProperty * propertyMax = FindProperty(L"Max Value");
		BOOL enableEdit = autoScale ? FALSE : TRUE;

		propertyMax->Enable(enableEdit);
	}
	else if (wcscmp(pProp->GetName(), L"Max Value") == 0)
	{
		_variant_t value = pProp->GetValue();
		double maxValue = value.dblVal;
		maxValue = max(maxValue, 2.0 * ::SettingGetBitmapRangeHalfDelta(false));
		maxValue = min(maxValue, ::SettingGetBitmapRangeMax(false));

		if (maxValue != value.dblVal)
		{
			value.dblVal = maxValue;
			pProp->SetValue(value);
		}

		EventMaxValue.Raise(this, maxValue);
	}
	else if (wcscmp(pProp->GetName(), L"Show Grid") == 0)
	{
		_variant_t value = pProp->GetValue();
		bool showGrid = (value.boolVal != 0);
		EventShowGrid.Raise(this, showGrid);
	}
	else if (wcscmp(pProp->GetName(), L"Luminescence") == 0)
	{
		_variant_t value = pProp->GetValue();
		bool bLuminescence = (value.boolVal != 0);
		EventLuminescence.Raise(this, bLuminescence);
	}
	else if (wcscmp(pProp->GetName(), L"Data Count") == 0)
	{
		_variant_t value = pProp->GetValue();
		int dataCount = value.intVal;
		dataCount = max(1, dataCount);
		dataCount = min((int)::SettingDotMaxDataCount(), dataCount);
		if (dataCount != value.intVal)
		{
			value.intVal = dataCount;
			pProp->SetValue(value);
		}

		size_t eValue = (size_t)dataCount;
		EventDataCount.Raise(this, eValue);
	}

	return BaseProperty::OnPropertyChanged(pProp);
}


void PlotWindowDotsProperty::OnCloseProperty()
{
	this->EventAutoScale.Reset();
	this->EventLog.Reset();
	this->EventMaxValue.Reset();
	this->EventName.Reset();
	this->EventShowGrid.Reset();
	this->EventXLabel.Reset();
	this->EventYLabel.Reset();
	this->EventLuminescence.Reset();
	this->EventDataCount.Reset();
}

void PlotWindowDotsProperty::UpdateMax(double max)
{
	this->DoLater([this, max](bool bFlush){
		if (bFlush)
			return;

		if (this->m_hWnd == 0)
			return;

		CMFCPropertyGridProperty * property = this->FindProperty(L"Max Value");

		_variant_t value = property->GetValue();
		value.dblVal = max;
		property->SetValue(value);
	});
}

void PlotWindowDotsProperty::UpdateDataRange(size_t dataCount)
{
	this->DoLater([this, dataCount](bool bFlush){
		if (bFlush)
			return;

		if (this->m_hWnd == 0)
			return;

		CMFCPropertyGridProperty * property = this->FindProperty(L"Data Count");

		_variant_t value = property->GetValue();
		value.intVal = dataCount;
		property->SetValue(value);
	});
}
