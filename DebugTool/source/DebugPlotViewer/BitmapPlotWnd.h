#pragma once

#include "PlotWnd.h"
#include "TargetTestable.h"

class BitmapPlotWnd : public CPlotWnd
{
public:
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
	SoraDbgPlot::Event::Event<ResizeEvent> EventResize;

	void SetBitmap(Bitmap * bitmap);
	void SetButtonStatePlay();
	void SetButtonStatePause();
	void SetTrackBarViewWndRange(double right, double range);
	void UpdateView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

public:
	void AddSubPlotWnd(SubPlotWnd * subWnd, CRect & rect, void * userData);
	void AddSubPlotWndRelative(SubPlotWnd * subWnd, CRect & rect, void * userData);
};
