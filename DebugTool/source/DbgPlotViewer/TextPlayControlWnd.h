#pragma once

#include "Event.h"

// TextPlayControlWnd

class TextPlayControlWnd : public CWnd
{
	DECLARE_DYNAMIC(TextPlayControlWnd)

public:
	TextPlayControlWnd();
	virtual ~TextPlayControlWnd();

public:
	struct PlayPauseEvent
	{
		bool IsPlay;
	};

	SoraDbgPlot::Event::Event<PlayPauseEvent> EventPlayClicked;

	void SetButtonStatePlay();
	void SetButtonStatePause();

private:
	static const int ID_BUTTON_PLAY = 1;
	CButton _buttonPlay;
	
	enum PlayPauseState
	{
		STATE_PLAY,
		STATE_PAUSE,
	};

	PlayPauseState _playPauseState;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnPlayButtonClicked();
};


