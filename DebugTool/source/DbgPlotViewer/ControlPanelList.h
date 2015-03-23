#pragma once

#include <functional>
#include <vector>
#include "Invokable.h"
#include "ControlPanelWnd.h"
#include "Event.h"

// ControlPanelList

class ControlPanelList : public Invokable
{
	DECLARE_DYNAMIC(ControlPanelList)

public:
	ControlPanelList();
	virtual ~ControlPanelList();

	void AddControlPanelWnd(std::shared_ptr<ControlPanelWnd>);
	void RemoveControlPanelWnd(std::shared_ptr<ControlPanelWnd>);

private:
	void AdjustLayout();

private:
	std::list<std::shared_ptr<ControlPanelWnd> > _controlPanelList;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
};


