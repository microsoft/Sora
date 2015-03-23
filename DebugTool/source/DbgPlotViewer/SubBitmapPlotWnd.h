#pragma once

#include <map>

//#include "DebugPlotViewerDoc.h"
//#include "AppType.h"
#include "Event.h"
#include "Strategy.h"
#include "Targetable.h"

// SubBitmapPlotWnd

class SubBitmapPlotWnd : public CWnd, public Targetable
{
	DECLARE_DYNAMIC(SubBitmapPlotWnd)

public:
	SubBitmapPlotWnd(void * userData);
	virtual ~SubBitmapPlotWnd();

	struct WheelEvent { bool IsUp; };
	SoraDbgPlot::Event::Event<WheelEvent> EventWheel;

	struct ClickEvent { int x; int y; };
	SoraDbgPlot::Event::Event<ClickEvent> EventClicked;


protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();

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
	void * UserData();
private:
	void * _userData;
};


