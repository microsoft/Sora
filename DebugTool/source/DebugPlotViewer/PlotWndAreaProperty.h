#pragma once

#include "BaseProperty.h"

// PlotWndAreaProperty

class PlotWndAreaProperty : public BaseProperty
{
	DECLARE_DYNAMIC(PlotWndAreaProperty)

public:
	PlotWndAreaProperty();
	virtual ~PlotWndAreaProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


