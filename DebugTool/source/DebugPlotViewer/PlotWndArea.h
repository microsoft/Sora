#pragma once

#include <vector>
#include "PlotWnd.h"
#include "AppMessage.h"
#include "TargetTestable.h"
#include "PlotWnd.h"
#include "PlotWndProp.h"

class CDebugPlotViewerDoc;

class PlotWndArea : public CWnd , public TargetTestable
{
	DECLARE_DYNAMIC(PlotWndArea)

public:
	PlotWndArea();
	virtual ~PlotWndArea();

private:
	bool dragHighlight;

	CPlotWnd * CreatePlotWnd(PlotWndProp * plotWndProp);

public:
	void AutoLayout(int pos);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg LRESULT OnApp(WPARAM, LPARAM);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

private:
	CDebugPlotViewerDoc * doc;
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

	PlotWndAreaProp * plotWndAreaProp;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	PlotWndAreaProp * GetProp();

	virtual bool TestTarget(void * obj);
	virtual void AddToTarget(void * obj);
	virtual void HighLight(bool highlight);
};



