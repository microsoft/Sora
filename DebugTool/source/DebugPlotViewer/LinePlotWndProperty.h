#pragma once

#include "BaseProperty.h"

// LinePlotWndProperty

class LinePlotWndProperty : public BaseProperty
{
	DECLARE_DYNAMIC(LinePlotWndProperty)

public:
	LinePlotWndProperty();
	virtual ~LinePlotWndProperty();
	virtual void UpdatePropertyValue(const wchar_t * propertyName);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


