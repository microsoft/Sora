#pragma once

#include "BaseProperty.h"
#include "Event.h"

// PlotWindowTextTypeProperty

class PlotWindowTextTypeProperty : public BaseProperty
{
	DECLARE_DYNAMIC(PlotWindowTextTypeProperty)

public:
	PlotWindowTextTypeProperty(const std::wstring & typeName, const std::wstring & name);
	virtual ~PlotWindowTextTypeProperty();

	SoraDbgPlot::Event::Event<std::wstring> EventName;
	virtual void OnCloseProperty();

private:
	std::wstring _typename;
	std::wstring _name;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


