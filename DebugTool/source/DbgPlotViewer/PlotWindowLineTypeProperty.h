#pragma once

#include <string>
#include "BaseProperty.h"
#include "Event.h"

// PlotWindowLineTypeProperty

class PlotWindowLineTypeProperty : public BaseProperty
{
	DECLARE_DYNAMIC(PlotWindowLineTypeProperty)

public:
	PlotWindowLineTypeProperty(
		const std::wstring typeName,
		const std::wstring name, 
		const std::wstring xLabel, 
		const std::wstring,
		bool bAutoScale,
		double maxValue,
		double minValue,
		bool bShowGrid,
		bool bLog,
		bool _enableDataRange,
		size_t dataRange
		);

	virtual ~PlotWindowLineTypeProperty();

	void UpdateMaxMin(double max, double min);
	void UpdateAutoScale(bool bAutoScale);
	void UpdateDataRange(size_t dataCount);

public:
	SoraDbgPlot::Event::Event<std::wstring> EventName;
	SoraDbgPlot::Event::Event<std::wstring> EventXLabel;
	SoraDbgPlot::Event::Event<std::wstring> EventYLabel;
	SoraDbgPlot::Event::Event<bool> EventAutoScale;
	SoraDbgPlot::Event::Event<double> EventMaxValue;
	SoraDbgPlot::Event::Event<double> EventMinValue;
	SoraDbgPlot::Event::Event<bool> EventShowGrid;
	SoraDbgPlot::Event::Event<bool> EventLog;
	SoraDbgPlot::Event::Event<size_t> EventDataCount;

	void OnCloseProperty();

private:
	std::wstring _typename;
	std::wstring _name;
	std::wstring _xLabel;
	std::wstring _yLabel;
	bool _bAutoScale;
	mutable double _maxValue;
	mutable double _minValue;
	bool _bShowGrid;
	mutable bool _bLog;
	bool _enableDataRange;
	size_t _dataRange;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


