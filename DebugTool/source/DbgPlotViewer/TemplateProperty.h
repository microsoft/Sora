#pragma once

#include "BaseProperty.h"
#include "Event.h"

// TemplateProperty

class TemplateProperty : public BaseProperty
{
	DECLARE_DYNAMIC(TemplateProperty)

public:
	TemplateProperty();
	virtual ~TemplateProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


