#pragma once

//#include "DebugPlotViewerDoc.h"
#include "SeriesText.h"
#include "Event.h"
#include "Strategy.h"

// SubPlotWnd

class SubPlotWnd : public CWnd
{
	DECLARE_DYNAMIC(SubPlotWnd)

public:
	SubPlotWnd();
	virtual ~SubPlotWnd();

	SoraDbgPlot::Event::Event<CRect> EventMoveWindow;

	SoraDbgPlot::Strategy::Strategy<int, COLORREF> StrategyGetColor;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);

	afx_msg void OnNcPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

public:
	//TextSeriesProp * seriesProp;

	bool isDraging;
	CRect rectBeforeDrag;
	CPoint cursorBeforeDrag;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCaptureChanged(CWnd *pWnd);

	void PlotText(CString & string);
	//virtual void SetSeriesObj(TextSeriesProp * prop);
	//virtual void SetPlotWndProp(PlotWndProp * prop);
private:
	CString string;
	void DrawString(Graphics * g);
	void DrawFrame(Graphics * g);
	bool isStatic;
	virtual void PostNcDestroy();
	//void SetData(StringData * dataInfo);
	//void RemoveData(void * sender);
public:
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);

public:
	//PlotWndProp * plotWndProp;
};
