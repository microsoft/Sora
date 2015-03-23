#pragma once

#include <map>

//#include "DebugPlotViewerDoc.h"
#include "AppType.h"
#include "Event.h"
#include "Strategy.h"
#include "TargetTestable.h"

// SubBitmapPlotWnd

class SubBitmapPlotWnd : public CWnd , public TargetTestable
{
	DECLARE_DYNAMIC(SubBitmapPlotWnd)

public:
	struct WheelEvent { bool IsUp; };
	SoraDbgPlot::Event::Event<WheelEvent> EventWheel;

	struct ClickEvent { int x; int y; };
	SoraDbgPlot::Event::Event<ClickEvent> EventClicked;

	struct AddTargetEvent { void * obj; };
	SoraDbgPlot::Event::Event<AddTargetEvent> EventAddTarget;

	struct TestTargetParam { void * obj; };
	SoraDbgPlot::Strategy::Strategy<TestTargetParam, bool> StrategyTestTarget;

	struct HighLightEvent { bool highlight; };
	SoraDbgPlot::Event::Event<HighLightEvent> EventHighLight;

	SubBitmapPlotWnd();
	virtual ~SubBitmapPlotWnd();

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	void SetBitmap(Bitmap * bitmap);

private:
	Bitmap * bmp;

public:
	virtual bool TestTarget(void * obj);
	virtual void AddToTarget(void * obj);
	virtual void HighLight(bool highlight);
};


