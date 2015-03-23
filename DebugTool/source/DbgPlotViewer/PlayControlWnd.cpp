// PlayControlWnd.cpp : implementation file
//

#include "stdafx.h"
#include "PlayControlWnd.h"


// PlayControlWnd

IMPLEMENT_DYNAMIC(PlayControlWnd, CWnd)

PlayControlWnd::PlayControlWnd()
{
	_bSpeedButtonEnabled = true;
}

PlayControlWnd::~PlayControlWnd()
{
}


BEGIN_MESSAGE_MAP(PlayControlWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(PlayControlWnd::ID_BUTTON_PLAY, OnPlayButtonClicked)
	ON_COMMAND(PlayControlWnd::ID_BUTTON_SPEED_UP, OnSpeedUpButtonClicked)
	ON_COMMAND(PlayControlWnd::ID_BUTTON_SPEED_DOWN, OnSpeedDownButtonClicked)
END_MESSAGE_MAP()


// PlayControlWnd message handlers

void PlayControlWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);
	Bitmap bmp(rect.right,rect.bottom);

	Graphics* memGraph = Graphics::FromImage(&bmp);

	SolidBrush brushBg(Color::Black);
	memGraph->FillRectangle(
		&brushBg,
		rect.left,
		rect.top,
		rect.Width(),
		rect.Height()
		);

	graphics.DrawImage(&bmp,rect.left,rect.top,rect.right,rect.bottom);
	
	delete memGraph;
}

int PlayControlWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	this->_buttonPlay.Create(L"||", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
        rectDummy, this, ID_BUTTON_PLAY);

	SetButtonStatePause();

	this->_buttonSpeedUp.Create(L"+", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		rectDummy, this, ID_BUTTON_SPEED_UP);

	this->_buttonSpeedDown.Create(L"-", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		rectDummy, this, ID_BUTTON_SPEED_DOWN);

	_trackBar.EventSeek.Subscribe([this](const void * sender, const ReplayBufferTrackBar::SeekEvent & e){
		TrackBarSeekEvent ee;
		ee.Pos = e.pos;
		this->EventTrackBarSeeked.Raise(this, ee);
	});

	this->_trackBar.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rectDummy, this, ID_TRACKBAR, 0);

	return 0;
}

void PlayControlWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect clientRect;
	this->GetClientRect(&clientRect);

	const int BUTTON_SIZE = 17;
	int buttonX = clientRect.right - BUTTON_SIZE * 3;
	int buttonY = clientRect.top;

	_buttonSpeedUp.MoveWindow(buttonX, buttonY, BUTTON_SIZE, BUTTON_SIZE, 1);
	buttonX += BUTTON_SIZE;

	_buttonSpeedDown.MoveWindow(buttonX, buttonY, BUTTON_SIZE, BUTTON_SIZE, 1);
	buttonX += BUTTON_SIZE;

	_buttonPlay.MoveWindow(buttonX, buttonY, BUTTON_SIZE, BUTTON_SIZE, 1);
	buttonX += BUTTON_SIZE;

	CRect rectTrackBar = clientRect;
	rectTrackBar.right = clientRect.right - BUTTON_SIZE * 3;
	rectTrackBar.left = min(rectTrackBar.left, rectTrackBar.left);
	_trackBar.MoveWindow(rectTrackBar.left, rectTrackBar.top, rectTrackBar.Width(), rectTrackBar.Height(), 1);
}

void PlayControlWnd::OnPlayButtonClicked()
{
	PlayPauseEvent e;
	e.IsPlay = _playPauseState == STATE_PLAY ? true : false;
	EventPlayClicked.Raise(this, e);
}

void PlayControlWnd::OnSpeedUpButtonClicked()
{
	SpeedChangeEvent e;
	e.IsUp = true;
	EventSpeedClicked.Raise(this, e);
}

void PlayControlWnd::OnSpeedDownButtonClicked()
{
	SpeedChangeEvent e;
	e.IsUp = false;
	EventSpeedClicked.Raise(this, e);
}

void PlayControlWnd::SetButtonStatePlay()
{
	_playPauseState = STATE_PLAY;
	_buttonPlay.SetWindowTextW(L">");
}

void PlayControlWnd::SetButtonStatePause()
{
	_playPauseState = STATE_PAUSE;
	_buttonPlay.SetWindowTextW(L"||");
}

void PlayControlWnd::SetTrackBarWndPos(double right, double range)
{
	_trackBar.SetViewWnd(right, range);
}

void PlayControlWnd::EnableSpeedButtons(bool bEnable)
{
	if (bEnable != _bSpeedButtonEnabled)
	{
		if (_buttonSpeedDown.m_hWnd)
			_buttonSpeedDown.EnableWindow(bEnable);
		if (_buttonSpeedUp.m_hWnd)
			_buttonSpeedUp.EnableWindow(bEnable);

		_bSpeedButtonEnabled = bEnable;
	}
}

