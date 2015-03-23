#pragma once

#include "BaseProperty.h"

// DotsSeriesProperty

class DotsSeriesProperty : public BaseProperty
{
	DECLARE_DYNAMIC(DotsSeriesProperty)

public:
	DotsSeriesProperty();
	virtual ~DotsSeriesProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


