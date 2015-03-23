#pragma once

#include "BaseProperty.h"

// SpectrumSeriesProperty

class SpectrumSeriesProperty : public BaseProperty
{
	DECLARE_DYNAMIC(SpectrumSeriesProperty)

public:
	SpectrumSeriesProperty();
	virtual ~SpectrumSeriesProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


