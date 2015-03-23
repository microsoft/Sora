#pragma once

#include <memory>
#include <vector>
#include "Strategy.h"

// DockableWndContainer

class DockableWndContainer : public CDockablePane
{
	DECLARE_DYNAMIC(DockableWndContainer)

public:
	DockableWndContainer();
	virtual ~DockableWndContainer();

	void SetBgColor(COLORREF);

	void AddChild(std::shared_ptr<CWnd>);
	void RemoveAllChilds();
	void UpdateLayout();
	void DockableWndContainer::Clear();

	struct LayoutParam
	{
		int _cx;
		int _cy;
		std::vector<std::shared_ptr<CWnd> > * _childs;
	};

	SoraDbgPlot::Strategy::Strategy<LayoutParam, bool> StrategyLayout;

private:
	void AutoLayout(int cx, int cy);
	std::vector<std::shared_ptr<CWnd> > _childs;
	COLORREF _color;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};


