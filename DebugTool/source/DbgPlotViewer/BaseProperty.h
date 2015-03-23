#pragma once

#include <memory>
#include <string>
#include "Event.h"
#include "CSRecursiveLock.h"
#include "PassiveTaskQueue.h"

// BaseProperty
#define WMME_EXE_TASKQUEUE (WM_APP+1)

class BaseProperty : public CMFCPropertyGridCtrl
{
	DECLARE_DYNAMIC(BaseProperty)

public:
	BaseProperty();
	virtual ~BaseProperty();
	void CloseProperty();
	SoraDbgPlot::Event::Event<bool> EventClosed;
	void DoLater(std::function<void(bool)>);

protected:
	virtual void OnCloseProperty() = 0;

private:
	PassiveTaskQueue _pvTaskQueue;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnExeTaskQueue(WPARAM, LPARAM);

protected:
	CMFCPropertyGridProperty * FindProperty(const wchar_t * name) const;
	virtual void PostNcDestroy();
};
