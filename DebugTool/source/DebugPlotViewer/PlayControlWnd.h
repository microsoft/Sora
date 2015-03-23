#pragma once

#include <Windows.h>
#include "Event.h"
#include "ReplayBufferTrackBar.h"

// PlayControlWnd

class PlayControlWnd : public CWnd
{
	DECLARE_DYNAMIC(PlayControlWnd)

public:
	PlayControlWnd();
	virtual ~PlayControlWnd();

	struct PlayPauseEvent
	{
		bool IsPlay;
	};
	
	struct SpeedChangeEvent
	{
		bool IsUp;
	};

	struct TrackBarSeekEvent
	{
		double Pos;
	};

	SoraDbgPlot::Event::Event<PlayPauseEvent> EventPlayClicked;
	SoraDbgPlot::Event::Event<SpeedChangeEvent> EventSpeedClicked;
	SoraDbgPlot::Event::Event<TrackBarSeekEvent> EventTrackBarSeeked;

	void SetButtonStatePlay();
	void SetButtonStatePause();
	void SetTrackBarWndPos(double right, double range);

private:
	CButton _buttonPlay;
	CButton _buttonSpeedUp;
	CButton _buttonSpeedDown;
	CRect _rectTrackBar;

	static const int ID_BUTTON_PLAY = 1;
	static const int ID_BUTTON_SPEED_UP = 2;
	static const int ID_BUTTON_SPEED_DOWN = 3;

	enum PlayPauseState
	{
		STATE_PLAY,
		STATE_PAUSE,
	};

	PlayPauseState _playPauseState;

private:
	static const int ID_TRACKBAR = 4;
	ReplayBufferTrackBar _trackBar;

private:
	void DrawTrackBar();

protected:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPlayButtonClicked();
	afx_msg void OnSpeedUpButtonClicked();
	afx_msg void OnSpeedDownButtonClicked();
	DECLARE_MESSAGE_MAP()
};


