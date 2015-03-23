#pragma once

//#include "DebugPlotViewerDoc.h"
#include "Invokable.h"
#include "Event.h"
#include "Strategy.h"

// SubPlotWnd

class SubPlotWnd : public Invokable
{
	DECLARE_DYNAMIC(SubPlotWnd)

public:
	SubPlotWnd();
	virtual ~SubPlotWnd();

	void SetColor(COLORREF color);

	SoraDbgPlot::Event::Event<CRect> EventMoveWindow;
	SoraDbgPlot::Event::Event<bool> EventClosed;

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

	bool isDraging;
	CRect rectBeforeDrag;
	CPoint cursorBeforeDrag;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCaptureChanged(CWnd *pWnd);

	void PlotText(CString & string);
private:
	CString string;
	COLORREF _color;
	void DrawString(Graphics * g);
	void DrawFrame(Graphics * g);
	bool isStatic;
	virtual void PostNcDestroy();
public:
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);

public:
	//PlotWndProp * plotWndProp;
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
