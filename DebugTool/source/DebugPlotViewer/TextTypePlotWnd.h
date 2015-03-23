#pragma once

#include "PlotWnd.h"
#include "TargetTestable.h"
#include "SubTextTypePlotWnd.h"

class TextTypePlotWnd : public CPlotWnd
{
public:
	~TextTypePlotWnd();

private:
	static const int ID_BUTTON_PLAY = 1;
	SubTextTypePlotWnd * _subTextWnd;
	CButton * _buttonPlay;

	enum PlayPauseState
	{
		STATE_PLAY,
		STATE_PAUSE,
	};

	PlayPauseState _playPauseState;
	

public:
	struct PlayPauseEvent { bool IsPlay; };
	//struct SpeedChangeEvent { bool IsUp; };
	//struct SeekEvent { double Pos; };
	//struct WheelEvent { bool IsUp; };
	struct ResizeEvent { int x; int y; int cx; int cy; };

	SoraDbgPlot::Event::Event<PlayPauseEvent> EventPlayClicked;
	//SoraDbgPlot::Event::Event<SpeedChangeEvent> EventSpeedClicked;
	//SoraDbgPlot::Event::Event<SeekEvent> EventSeek;
	//SoraDbgPlot::Event::Event<WheelEvent> EventWheel;
	SoraDbgPlot::Event::Event<ResizeEvent> EventResize;

	SoraDbgPlot::Strategy::Strategy<size_t, char *> StrategyGetText;

	//void SetBitmap(Bitmap * bitmap);
	void SetButtonStatePlay();
	void SetButtonStatePause();
	//void SetTrackBarViewWndRange(double right, double range);
	void UpdateView(size_t itemCount);
	void EnableUpdate(bool);
public:
	void SetColor(COLORREF color);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPlayButtonClicked();
	DECLARE_MESSAGE_MAP()

};
