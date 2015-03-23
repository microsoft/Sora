#include "stdafx.h"
#include "TextTypePlotWnd.h"
#include "SubTextTypePlotWnd.h"
#include "SeriesObj.h"

BEGIN_MESSAGE_MAP(TextTypePlotWnd, CPlotWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(TextTypePlotWnd::ID_BUTTON_PLAY, OnPlayButtonClicked)
END_MESSAGE_MAP()


int TextTypePlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPlotWnd::OnCreate(lpCreateStruct) == -1 )
		return -1;
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	_subTextWnd = new SubTextTypePlotWnd;

	_subTextWnd->StrategyGetText.Set([this](const void * sender, size_t index, char * & text){
		bool succ = this->StrategyGetText.Call(sender, index, text);
	});
	
	_subTextWnd->Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL, rectDummy, this, 0);

	_playPauseState = STATE_PLAY;
	_buttonPlay = new CButton;
	_buttonPlay->Create(L"||", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		rectDummy, this, ID_BUTTON_PLAY);

	SetButtonStatePause();

	return 0;
}

void TextTypePlotWnd::OnSize(UINT nType, int cx, int cy)
{
	CRect clientRect;
	this->CalcClientRect(&clientRect);

	CRect controlRect = clientRect;
	controlRect.top = controlRect.bottom - 17;
	controlRect.left = controlRect.right - 17;

	_buttonPlay->MoveWindow(&controlRect, 1);

	if (controlRect.top > 0) // now the graphic window can be visible
	{
		CRect graphRect = clientRect;
		graphRect.bottom = controlRect.top;
		_subTextWnd->MoveWindow(&graphRect, 1);
		ResizeEvent e;
		e.cx = graphRect.Width();
		e.cy = graphRect.Height();
		this->EventResize.Raise(this, e);
	}

	CPlotWnd::OnSize(nType, cx, cy);
}

void TextTypePlotWnd::SetButtonStatePlay()
{
	_playPauseState = STATE_PLAY;
	_buttonPlay->SetWindowTextW(L">");
}

void TextTypePlotWnd::SetButtonStatePause()
{
	_playPauseState = STATE_PAUSE;
	_buttonPlay->SetWindowTextW(L"||");
}

void TextTypePlotWnd::UpdateView(size_t itemCount)
{
	_subTextWnd->SetItemCount(itemCount);
	_subTextWnd->EnsureVisible(_subTextWnd->GetItemCount() - 1, FALSE);
}

void TextTypePlotWnd::EnableUpdate(bool enable)
{
	this->_subTextWnd->SetRedraw(enable == true);
}

TextTypePlotWnd::~TextTypePlotWnd()
{
	if (_subTextWnd)
		delete _subTextWnd;
	if (_buttonPlay)
		delete _buttonPlay;
}

void TextTypePlotWnd::OnPlayButtonClicked()
{
	PlayPauseEvent e;
	e.IsPlay = _playPauseState == STATE_PLAY ? true : false;
	EventPlayClicked.Raise(this, e);
}

void TextTypePlotWnd::SetColor(COLORREF color)
{
	_subTextWnd->SetColor(color);
}
