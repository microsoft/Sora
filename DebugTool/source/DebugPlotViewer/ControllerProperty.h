#pragma once
#include "BaseProperty.h"

// ControllerProperty

class ControllerProperty : public BaseProperty
{
	DECLARE_DYNAMIC(ControllerProperty)

public:
	ControllerProperty();
	virtual ~ControllerProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;
};


