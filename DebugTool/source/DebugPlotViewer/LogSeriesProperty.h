#pragma once

#include "BaseProperty.h"

// LogSeriesProperty

class LogSeriesProperty : public BaseProperty
{
	DECLARE_DYNAMIC(LogSeriesProperty)

public:
	LogSeriesProperty();
	virtual ~LogSeriesProperty();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


