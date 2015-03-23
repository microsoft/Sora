#pragma once

#include "BaseProperty.h"

// DotsPlotWndProperty

class DotsPlotWndProperty : public BaseProperty
{
	DECLARE_DYNAMIC(DotsPlotWndProperty)

public:
	DotsPlotWndProperty();
	virtual ~DotsPlotWndProperty();
	virtual void UpdatePropertyValue(const wchar_t * propertyName);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


