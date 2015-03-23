#pragma once

#include "BaseProperty.h"

// SpectrumPlotWndProperty

class SpectrumPlotWndProperty : public BaseProperty
{
	DECLARE_DYNAMIC(SpectrumPlotWndProperty)

public:
	SpectrumPlotWndProperty();
	virtual ~SpectrumPlotWndProperty();
	virtual void UpdatePropertyValue(const wchar_t * propertyName);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


