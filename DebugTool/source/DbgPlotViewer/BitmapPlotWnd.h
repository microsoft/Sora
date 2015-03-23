#pragma once

#include "PlotWnd.h"
#include "PlayControlWnd.h"
#include "SubBitmapPlotWnd.h"
#include "Targetable.h"

class BitmapPlotWnd : public CPlotWnd
{
public:
	BitmapPlotWnd(void * userData);
	~BitmapPlotWnd();

public:
	PlayControlWnd * _controlWnd;
	SubBitmapPlotWnd * _subBitmapWnd;
	CWnd * _canvasWnd;

	struct PlayPauseEvent { bool IsPlay; };
	struct SpeedChangeEvent { bool IsUp; };
	struct SeekEvent { double Pos; };
	struct WheelEvent { bool IsUp; };
	struct ResizeEvent { int x; int y; int cx; int cy; };

	SoraDbgPlot::Event::Event<PlayPauseEvent> EventPlayClicked;
	SoraDbgPlot::Event::Event<SpeedChangeEvent> EventSpeedClicked;
	SoraDbgPlot::Event::Event<SeekEvent> EventSeek;
	SoraDbgPlot::Event::Event<WheelEvent> EventWheel;
	SoraDbgPlot::Event::Event<ResizeEvent> EventBitmapResize;
	
	void SetBitmap(Bitmap * bitmap);
	void SetButtonStatePlay();
	void SetButtonStatePause();
	void SetTrackBarViewWndRange(double right, double range);
	void MyScreenToClient(CRect &);
	void MyScreenToClient(CPoint &);
	void EnableSpeedButtons(bool bEnable);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

public:
	void AddSubPlotWnd(SubPlotWnd * subWnd, const CRect & rect);
	void AddSubPlotWndRelative(SubPlotWnd * subWnd, const CRect & rect);

public:
	void * UserData();
private:
	void * _userData;
};
