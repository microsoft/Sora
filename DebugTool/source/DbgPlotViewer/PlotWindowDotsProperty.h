#pragma once

#include <string>
#include "BaseProperty.h"
#include "Event.h"

// PlotWindowDotsProperty

class PlotWindowDotsProperty : public BaseProperty
{
	DECLARE_DYNAMIC(PlotWindowDotsProperty)

public:
	PlotWindowDotsProperty(
		const std::wstring name, 
		const std::wstring xLabel, 
		const std::wstring,
		bool bAutoScale,
		double maxValue,
		size_t dataCount,
		bool bShowGrid,
		bool bLog,
		bool bLuminescence
		);

	virtual ~PlotWindowDotsProperty();

	void UpdateMax(double max);
	void UpdateDataRange(size_t dataCount);

public:
	SoraDbgPlot::Event::Event<std::wstring> EventName;
	SoraDbgPlot::Event::Event<std::wstring> EventXLabel;
	SoraDbgPlot::Event::Event<std::wstring> EventYLabel;
	SoraDbgPlot::Event::Event<bool> EventAutoScale;
	SoraDbgPlot::Event::Event<double> EventMaxValue;
	SoraDbgPlot::Event::Event<bool> EventShowGrid;
	SoraDbgPlot::Event::Event<bool> EventLog;
	SoraDbgPlot::Event::Event<bool> EventLuminescence;
	SoraDbgPlot::Event::Event<size_t> EventDataCount;

	void OnCloseProperty();

private:
	std::wstring _name;
	std::wstring _xLabel;
	std::wstring _yLabel;
	bool _bAutoScale;
	double _maxValue;
	size_t _dataCount;
	bool _bShowGrid;
	bool _bLog;
	bool _bLuminescence;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


