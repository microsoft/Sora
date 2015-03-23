#pragma once

#include "PlotWnd.h"
#include "PassiveTaskQueue.h"
#include "Invokable.h"
#include "Targetable.h"

// PlotWndContainer

class PlotWndContainer : public Invokable, public Targetable
{
	DECLARE_DYNAMIC(PlotWndContainer)

public:
	PlotWndContainer(void * userData);
	virtual ~PlotWndContainer();
	void HighLight(bool highlight);
	void SetDropPoint(CPoint point);

	SoraDbgPlot::Event::Event<CRect> EventSize;

	SoraDbgPlot::Event::Event<bool> EventClosed;

	void AddPlotWindow(CPlotWnd * wnd, const CRect & rect);
	void AddPlotWindow(CPlotWnd * wnd);

	virtual void * UserData();

private:
	bool _bDragHighlight;
	void * _userData;
	CPoint _dropPoint;
	CRect _dropRect;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	virtual void PostNcDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};

