#pragma once

#include "BaseProperty.h"

// LogPlotWndProperty

class LogPlotWndProperty : public BaseProperty
{
	DECLARE_DYNAMIC(LogPlotWndProperty)

public:
	LogPlotWndProperty();
	virtual ~LogPlotWndProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


