#include "stdafx.h"
#include "TextTypePlotWnd.h"
#include "SubTextTypePlotWnd.h"

BEGIN_MESSAGE_MAP(TextTypePlotWnd, CPlotWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


TextTypePlotWnd::TextTypePlotWnd(void * userData) :
	CPlotWnd(userData)
{
	_lastItemCount = -1;
}

int TextTypePlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPlotWnd::OnCreate(lpCreateStruct) == -1 )
		return -1;
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	_canvasWnd = new CWnd();
	_canvasWnd->Create(NULL, L"control", WS_CHILD | WS_VISIBLE, rectDummy, this, 0);

	_subTextWnd = new SubTextTypePlotWnd;

	_subTextWnd->StrategyGetText.Set([this](const void * sender, size_t index, char * & text){
		bool succ = this->StrategyGetText.Call(sender, index, text);
	});

	_subTextWnd->Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL, rectDummy, _canvasWnd, 0);
	_subTextWnd->SetColumnWidth(0, 2000);

	_controlWnd = new TextPlayControlWnd;
	_controlWnd->Create(NULL, L"control", WS_CHILD | WS_VISIBLE, rectDummy, _canvasWnd, 0);

	_controlWnd->EventPlayClicked.Subscribe([this](const void * sender, const TextPlayControlWnd::PlayPauseEvent & e){
		PlayPauseEvent ee;
		ee.IsPlay = e.IsPlay;
		this->EventPlayClicked.Raise(this, ee);
	});

	return 0;
}

void TextTypePlotWnd::OnSize(UINT nType, int cx, int cy)
{
	const int CTRL_WND_HEIGHT = 17;

	CRect clientRect;
	this->CalcClientRect(&clientRect);

	CRect canvasRect = clientRect;

	_canvasWnd->MoveWindow(&canvasRect);

	CRect controlRect;
	controlRect.right = canvasRect.Width();
	controlRect.bottom = canvasRect.Height();
	controlRect.left = 0;
	controlRect.top = canvasRect.Height() - CTRL_WND_HEIGHT;

	_controlWnd->MoveWindow(&controlRect, 1);

	if (true)
	{
		CRect graphRect;
		graphRect.SetRectEmpty();
		graphRect.right = clientRect.Width();
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
	this->Invoke([this](bool close){
		if (close) return;

		if (this->_controlWnd->m_hWnd)
		{
			this->_controlWnd->SetButtonStatePlay();
		}
	});
}

void TextTypePlotWnd::SetButtonStatePause()
{
	this->Invoke([this](bool close){
		if (close) return;

		if (this->_controlWnd->m_hWnd)
		{
			this->_controlWnd->SetButtonStatePause();
		}
	});
}

void TextTypePlotWnd::UpdateView(size_t itemCount)
{
	this->Invoke([this, itemCount](bool close){

		if (close) return;

		if (this->_subTextWnd->m_hWnd)
		{
			//if (itemCount != this->_lastItemCount)	// this causes a bug
			if (true)
			{
				int sel = this->_subTextWnd->GetSelectionMark();
				this->_subTextWnd->SetItemState(sel, 0, LVIS_SELECTED | LVIS_FOCUSED);

				this->_subTextWnd->SetRedraw(FALSE);
				this->_subTextWnd->SetItemCount(itemCount);
				//this->_subTextWnd->EnsureVisible(0, FALSE);
				this->_subTextWnd->EnsureVisible(itemCount - 1, FALSE);
				this->_subTextWnd->SetRedraw(TRUE);
				this->_lastItemCount = itemCount;
			}
			this->_subTextWnd->UpdateWindow();
		}
	});
}

void TextTypePlotWnd::EnableUpdate(bool enable)
{
	this->_subTextWnd->SetRedraw(enable == true);
}

TextTypePlotWnd::~TextTypePlotWnd()
{
	if (_subTextWnd)
	{
		delete _subTextWnd;
		_subTextWnd = 0;
	}
	if (_controlWnd)
	{
		delete _controlWnd;
		_controlWnd = 0;
	}
	if (_canvasWnd)
	{
		delete _canvasWnd;
		_canvasWnd = 0;
	}
}


void TextTypePlotWnd::SetColor(COLORREF color)
{
	this->Invoke([this, color](bool close){
		if (close) return;

		_subTextWnd->SetColor(color);
	});
}
