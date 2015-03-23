#pragma once

#include "SeriesObj.h"

// BaseProperty

class BaseProperty : public CMFCPropertyGridCtrl
{
	DECLARE_DYNAMIC(BaseProperty)

public:
	BaseProperty();
	virtual ~BaseProperty();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void UpdatePropertyValue(const wchar_t * propertyName);

	void SetTarget(void * target);
	void * GetTarget();
	CMFCPropertyGridProperty * FindProperty(const wchar_t * name) const;

protected:
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);
	void * target;
};



