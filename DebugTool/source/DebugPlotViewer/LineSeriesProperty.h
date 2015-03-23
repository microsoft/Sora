#pragma once

#include "BaseProperty.h"

// LineSeriesProperty

class LineSeriesProperty : public BaseProperty
{
	DECLARE_DYNAMIC(LineSeriesProperty)

public:
	LineSeriesProperty();
	virtual ~LineSeriesProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


