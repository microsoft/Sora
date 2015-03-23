// TextPlayControlWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "TextPlayControlWnd.h"


// TextPlayControlWnd

IMPLEMENT_DYNAMIC(TextPlayControlWnd, CWnd)

TextPlayControlWnd::TextPlayControlWnd()
{
}

TextPlayControlWnd::~TextPlayControlWnd()
{
}


BEGIN_MESSAGE_MAP(TextPlayControlWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_COMMAND(TextPlayControlWnd::ID_BUTTON_PLAY, OnPlayButtonClicked)
END_MESSAGE_MAP()

// TextPlayControlWnd message handlers

int TextPlayControlWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	
	_playPauseState = STATE_PAUSE;
	this->_buttonPlay.Create(L"||", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
        rectDummy, this, ID_BUTTON_PLAY);

	return 0;
}

void TextPlayControlWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect clientRect;
	this->GetClientRect(&clientRect);

	const int BUTTON_SIZE = 17;
	int buttonX = clientRect.right - BUTTON_SIZE;
	int buttonY = clientRect.top;

	_buttonPlay.MoveWindow(buttonX, buttonY, BUTTON_SIZE, BUTTON_SIZE, 1);
}

void TextPlayControlWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(&rect);

	CBrush backBrush(RGB(0, 0, 0));

	dc.FillRect(&rect, &backBrush);
}

void TextPlayControlWnd::OnPlayButtonClicked()
{
	PlayPauseEvent e;
	e.IsPlay = _playPauseState == STATE_PLAY ? true : false;
	EventPlayClicked.Raise(this, e);
}

void TextPlayControlWnd::SetButtonStatePlay()
{
	if (_playPauseState == STATE_PAUSE)
		_buttonPlay.SetWindowTextW(L">");
	_playPauseState = STATE_PLAY;
}

void TextPlayControlWnd::SetButtonStatePause()
{
	if (_playPauseState == STATE_PLAY)
		_buttonPlay.SetWindowTextW(L"||");
	_playPauseState = STATE_PAUSE;
}
