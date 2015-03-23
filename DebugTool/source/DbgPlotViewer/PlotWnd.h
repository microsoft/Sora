#pragma once

//#include "DebugPlotViewerDoc.h"
#include "SubPlotWnd.h"
#include <map>
#include "Event.h"
#include "Strategy.h"
#include "Invokable.h"
#include "Targetable.h"

class CPlotWnd : public Invokable, public Targetable
{
public:
	CPlotWnd(void * userData);
	virtual ~CPlotWnd();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	void SetInitCaption(const CString & str);
	void SetCaption(const CString & str);
	void SetSnapGridSize(int size);

	struct MoveEvent { CRect rect; };
	SoraDbgPlot::Event::Event<MoveEvent> EventMove;

	struct BringToFrontEvent {};
	SoraDbgPlot::Event::Event<BringToFrontEvent> EventBringToFront;

	SoraDbgPlot::Event::Event<bool> EventCreated;

	struct NullEvent {};
	SoraDbgPlot::Event::Event<NullEvent> EventClosed;

	struct QueryInfoEvent {
		const enum QueryType {
			TYPE_GRID_SIZE,
		} type;

		union Value
		{
			int intVal;
		} value;
	};

	struct NullParam {};
	SoraDbgPlot::Strategy::Strategy<NullParam, int> StrategyGetGridSize;
	SoraDbgPlot::Strategy::Strategy<NullParam, CString> StrategyGetCaption;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnClose();

	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSetCursor();

	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);

	DECLARE_MESSAGE_MAP()

protected:
	bool dragHighlight;

public:
	afx_msg void OnNcPaint();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);

public:
	void MoveWindowAsync(const CRect & rect);

	void DrawCloseButton(Graphics * g);
	void DrawCaption(Graphics * g);
	void DrawFrame(Graphics * g);

	void CalcCloseButtonRect(CRect * rect);
	void CalcCaptionRect(CRect * rect);
	void CalcClientRect(CRect * rect);

	bool PointInRect(CPoint * point, CRect * rect);

private:
	bool isDraging;
	CRect rectBeforeDrag;
	CPoint cursorBeforeDrag;
	CString _caption;
	int _snapGridSize;

public:
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNcMouseLeave();
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	virtual void PostNcDestroy();

public:
	void HighLight(bool highlight);
public:
	void AddSubPlotWnd(SubPlotWnd * subWnd);

public:
	void * UserData();
private:
	void * _userData;
};
