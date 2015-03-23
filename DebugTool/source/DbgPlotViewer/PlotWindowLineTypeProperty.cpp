// PlotWindowLineTypeProperty.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "PlotWindowLineTypeProperty.h"
#include "BitmapTypeSettings.h"
#include "SettingsPlotWindow.h"

using namespace std;

// PlotWindowLineTypeProperty

IMPLEMENT_DYNAMIC(PlotWindowLineTypeProperty, BaseProperty)

PlotWindowLineTypeProperty::PlotWindowLineTypeProperty(
	const std::wstring typeName,
	const std::wstring name, 
	const std::wstring xLabel, 
	const std::wstring yLabel,
	bool bAutoScale,
	double maxValue,
	double minValue,
	bool bShowGrid,
	bool bLog,
	bool enableDataRange,
	size_t dataRange
	)
{
	_typename = typeName;
	_name = name;
	_xLabel = xLabel;
	_yLabel = yLabel;
	_bAutoScale = bAutoScale;
	_maxValue = maxValue;
	_minValue = minValue;
	_bShowGrid = bShowGrid;
	_bLog = bLog;
	_enableDataRange = enableDataRange;
	_dataRange = dataRange;
}

PlotWindowLineTypeProperty::~PlotWindowLineTypeProperty()
{
}


BEGIN_MESSAGE_MAP(PlotWindowLineTypeProperty, BaseProperty)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// PlotWindowLineTypeProperty message handlers




int PlotWindowLineTypeProperty::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (BaseProperty::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	CMFCPropertyGridProperty * pProp;

	pProp = new CMFCPropertyGridProperty(_T("Type"), _typename.c_str());
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

	pProp = new CMFCPropertyGridProperty( _T("Min Value"), (_variant_t) _minValue, _T("Specifies the min value"));
	this->AddProperty(pProp);	
	if (_bAutoScale)
		pProp->Enable(FALSE);

	if (_enableDataRange)
	{
		pProp = new CMFCPropertyGridProperty( _T("Data Count"), (_variant_t) _dataRange, _T("Specifies the data count."));
		this->AddProperty(pProp);	
	}

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

	pProp = new CMFCPropertyGridProperty(_T("Coordinate"), strMode.GetBuffer(), _T("Specifies the type of the coordinate system"));
	pProp->AddOption(_T("General"));
	pProp->AddOption(_T("Logarithm"));
	pProp->AllowEdit(FALSE);

	this->AddProperty(pProp);

	return 0;
}


void PlotWindowLineTypeProperty::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
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
		CMFCPropertyGridProperty * propertyMin = FindProperty(L"Min Value");
		BOOL enableEdit = autoScale ? FALSE : TRUE;

		propertyMax->Enable(enableEdit);
		propertyMin->Enable(enableEdit);
	}
	else if (wcscmp(pProp->GetName(), L"Max Value") == 0)
	{
		_variant_t value = pProp->GetValue();
		_maxValue = value.dblVal;
		double maxValidValue = ::SettingGetBitmapRangeMax(_bLog);
		_maxValue = max(_minValue + ::SettingGetBitmapRangeHalfDelta(_bLog), _maxValue);
		_maxValue = min(_maxValue, maxValidValue);

		if (value.dblVal != _maxValue)
		{
			value.dblVal = _maxValue;
			pProp->SetValue(value);
		}

		EventMaxValue.Raise(this, _maxValue);
	}
	else if (wcscmp(pProp->GetName(), L"Min Value") == 0)
	{
		_variant_t value = pProp->GetValue();
		_minValue = value.dblVal;
		double minValidValue = ::SettingGetBitmapRangeMin(_bLog);
		_minValue = min(_maxValue - ::SettingGetBitmapRangeHalfDelta(_bLog), _minValue);
		_minValue = max(_minValue, minValidValue);

		if (value.dblVal != _minValue)
		{
			value.dblVal = _minValue;
			pProp->SetValue(value);
		}

		EventMinValue.Raise(this, _minValue);
	}

	else if (wcscmp(pProp->GetName(), L"Show Grid") == 0)
	{
		_variant_t value = pProp->GetValue();
		bool showGrid = (value.boolVal != 0);
		EventShowGrid.Raise(this, showGrid);
	}
	else if (wcscmp(pProp->GetName(), L"Coordinate") == 0)
	{
		_variant_t value = pProp->GetValue();
		BSTR bstr = value.bstrVal;
		if ( wcscmp(bstr, L"General") == 0 )
		{
			_bLog = false;
		}
		else
		{
			_bLog = true;
		}

		EventLog.Raise(this, _bLog);
	}
	else if (wcscmp(pProp->GetName(), L"Data Count") == 0)
	{
		assert(_enableDataRange);
		_variant_t value = pProp->GetValue();
		int dataCount = value.intVal;
		dataCount = max(1, dataCount);
		dataCount = min((int)::SettingLineTypeMaxDataCount(), dataCount);
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


void PlotWindowLineTypeProperty::OnCloseProperty()
{
	this->EventAutoScale.Reset();
	this->EventLog.Reset();
	this->EventMaxValue.Reset();
	this->EventMinValue.Reset();
	this->EventName.Reset();
	this->EventShowGrid.Reset();
	this->EventXLabel.Reset();
	this->EventYLabel.Reset();
	this->EventDataCount.Reset();
}

void PlotWindowLineTypeProperty::UpdateMaxMin(double max, double min)
{
	this->DoLater([this, max, min](bool bFlush){
		if (bFlush)
			return;

		if (this->m_hWnd == 0)
			return;

		// max
		_maxValue = max;

		CMFCPropertyGridProperty * property = this->FindProperty(L"Max Value");

		_variant_t value = property->GetValue();
		value.dblVal = max;
		property->SetValue(value);

		// min
		_minValue = min;

		property = FindProperty(L"Min Value");

		value.dblVal = min;
		property->SetValue(value);
	});
}

void PlotWindowLineTypeProperty::UpdateAutoScale(bool bAutoScale)
{
	this->DoLater([this, bAutoScale](bool bFlush){
		if (bFlush)
			return;

		if (this->m_hWnd == 0)
			return;

		CMFCPropertyGridProperty * property = this->FindProperty(L"Auto Scale");

		_variant_t value = property->GetValue();
		value.boolVal = bAutoScale;
		property->SetValue(value);

		CMFCPropertyGridProperty * propertyMax = FindProperty(L"Max Value");
		CMFCPropertyGridProperty * propertyMin = FindProperty(L"Min Value");
		BOOL enableEdit = bAutoScale ? FALSE : TRUE;

		propertyMax->Enable(enableEdit);
		propertyMin->Enable(enableEdit);
	});
}

void PlotWindowLineTypeProperty::UpdateDataRange(size_t dataCount)
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
