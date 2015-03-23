#pragma once

#include "PlotWnd.h"
#include "Targetable.h"
#include "SubTextTypePlotWnd.h"
#include "TextPlayControlWnd.h"

class TextTypePlotWnd : public CPlotWnd
{
public:
	TextTypePlotWnd(void * userData);
	~TextTypePlotWnd();

private:
	static const int ID_BUTTON_PLAY = 1;
	SubTextTypePlotWnd * _subTextWnd;
	CWnd * _canvasWnd;
	TextPlayControlWnd * _controlWnd;

	
	int _lastItemCount;

public:
	struct PlayPauseEvent { bool IsPlay; };

	struct ResizeEvent { int x; int y; int cx; int cy; };

	SoraDbgPlot::Event::Event<PlayPauseEvent> EventPlayClicked;

	SoraDbgPlot::Event::Event<ResizeEvent> EventResize;

	SoraDbgPlot::Strategy::Strategy<size_t, char *> StrategyGetText;

	void SetButtonStatePlay();
	void SetButtonStatePause();
	void UpdateView(size_t itemCount);
	void EnableUpdate(bool);
public:
	void SetColor(COLORREF color);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

};
