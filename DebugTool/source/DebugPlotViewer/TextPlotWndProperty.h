#pragma once

#include "BaseProperty.h"

// TextPlotWndProperty

class TextPlotWndProperty : public BaseProperty
{
	DECLARE_DYNAMIC(TextPlotWndProperty)

public:
	TextPlotWndProperty();
	virtual ~TextPlotWndProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


