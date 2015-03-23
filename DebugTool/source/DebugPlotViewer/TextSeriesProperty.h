#pragma once

#include "BaseProperty.h"

// TextSeriesProperty

class TextSeriesProperty : public BaseProperty
{
	DECLARE_DYNAMIC(TextSeriesProperty)

public:
	TextSeriesProperty();
	virtual ~TextSeriesProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


