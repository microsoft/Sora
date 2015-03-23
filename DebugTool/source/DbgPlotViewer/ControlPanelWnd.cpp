// ControlPanelWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "ControlPanelWnd.h"


// ControlPanelWnd

IMPLEMENT_DYNAMIC(ControlPanelWnd, Invokable)

ControlPanelWnd::ControlPanelWnd(const CString & caption)
{
	_captionWidth = 100;
	_readPos = 0.4f;
	_writePos = 0.55f;
	_caption = caption;
	_buttonIsPlayState = false;
	_bitmap = 0;
	_buttonEnabled = true;
}

ControlPanelWnd::~ControlPanelWnd()
{
	if (_bitmap)
		delete _bitmap;
}

int ControlPanelWnd::Height()
{
	return 30;
}

int ControlPanelWnd::ButtonWidth()
{
	return 28;
}


void ControlPanelWnd::DrawCaption(Graphics * g)
{

	SolidBrush fontBrush(Color(255, 150, 150, 150));
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	//format.SetFormatFlags(StringFormatFlagsNoWrap);
	//format.SetTrimming(StringTrimmingEllipsisCharacter);
	Gdiplus::Font captionFont(L"Arial", 10);
	PointF pointF(5, 2);

	CRect rectCaption;
	GetClientRect(&rectCaption);
	int top = rectCaption.top;
	int bottom = rectCaption.bottom;
	int middle = (top + bottom) / 2;

	rectCaption.left = this->_captionLeft;
	rectCaption.right = this->_captionRight;

	SolidBrush brushBk(Color(30, 30, 30));
	g->FillRectangle(&brushBk, rectCaption.left, rectCaption.top, rectCaption.right,rectCaption.bottom);

	RectF rectName(
		(Gdiplus::REAL)this->_captionLeft,
		(Gdiplus::REAL)top,
		(Gdiplus::REAL)this->_captionRight,
		(Gdiplus::REAL)bottom );
	g->DrawString(_caption, -1, &captionFont, rectName, &format, &fontBrush);
}

void ControlPanelWnd::DrawTrackBar(Graphics * g)
{
	CRect rect;
	GetClientRect(&rect);
	if (_bitmap)
	{
		g->DrawImage(_bitmap, _trackBarLeft, rect.top, _trackBarRight - _trackBarLeft, rect.bottom - rect.top);
	}

	//CRect rect;
	//GetClientRect(&rect);
	//rect.left = this->_trackBarLeft;
	//rect.right = this->_trackBarRight;

	//float readRatio = this->_readPos;
	//float readPos = (rect.right - rect.left) * readRatio + rect.left;
	//Color colorReadDataPart(28,48,69);
	//SolidBrush brushReadDataPart(colorReadDataPart);
	//g->FillRectangle(
	//	&brushReadDataPart, 
	//	float(rect.left),
	//	float(rect.top), 
	//	readPos, 
	//	float(rect.bottom));

	//float emptyStartPos = readPos;

	//if (this->_readPos < this->_writePos)
	//{
	//	float writeRatio = (float)this->_writePos;
	//	float writePos = float((rect.right - rect.left) * writeRatio + rect.left);
	//	Color colorWriteDataPart(100, 100, 100);
	//	SolidBrush brushWriteDataPart(colorWriteDataPart);
	//	g->FillRectangle(
	//		&brushWriteDataPart,
	//		readPos,
	//		float(rect.top), 
	//		writePos, 
	//		float(rect.bottom));
	//	emptyStartPos = writePos;
	//}

	//SolidBrush brush(Color::Black);
	//g->FillRectangle(
	//	&brush,
	//	emptyStartPos,
	//	float(rect.top),
	//	float(rect.right),
	//	float(rect.bottom));
}

void ControlPanelWnd::AdjustLayout()
{
	// calculate one by one
	/*
	|caption|trace bar|play button|single step button|
	*/

	CRect rect;
	this->GetClientRect(&rect);
	int width = rect.Width();
	int height = rect.Height();

	const int MARGIN = 1;
	const int BUTTON_WIDTH = ControlPanelWnd::ButtonWidth();

	_captionLeft = 1;
	_captionRight = _captionLeft + _captionWidth;
	
	int buttonWidthRequired = MARGIN * 3 + BUTTON_WIDTH * 2;
	if (_captionRight + buttonWidthRequired >= width) // layout buttons from left
	{
		_trackBarLeft = _trackBarRight = _captionRight;
		_playButtonLeft = _captionRight + MARGIN;
		_playButtonRight = _playButtonLeft + BUTTON_WIDTH;
		_singleStepButtonLeft = _playButtonRight + MARGIN;
		_singleStepButtonRight = _singleStepButtonLeft + BUTTON_WIDTH;
	}
	else // layout buttons from right
	{
		_singleStepButtonRight = width - MARGIN;
		_singleStepButtonLeft = _singleStepButtonRight - BUTTON_WIDTH;
		_playButtonRight = _singleStepButtonLeft - MARGIN;
		_playButtonLeft = _playButtonRight - BUTTON_WIDTH;
		_trackBarLeft = _captionRight + MARGIN;
		_trackBarRight = _playButtonLeft - MARGIN;
		assert(_trackBarRight >= _trackBarLeft);
	}

	_playButton.MoveWindow(_playButtonLeft, 0, BUTTON_WIDTH, height, 1);
	_singleStepButton.MoveWindow(_singleStepButtonLeft, 0, BUTTON_WIDTH, height, 1);
}

void ControlPanelWnd::SetCaptionWidth(int width)
{
	this->Invoke([this, width](bool close){
		if (close) return;

		this->_captionWidth = width;
	});
}

void ControlPanelWnd::SetCaption(const CString & caption)
{
	this->Invoke([this, caption](bool close){
		if (close) return;

		this->_caption = caption;
	});
}

void ControlPanelWnd::SetReadWritePos(float readPos, float writePos)
{
	this->Invoke([this, readPos, writePos](bool close){
		if (close) return;

		this->_readPos = readPos;
		this->_writePos = writePos;
	});
}

void ControlPanelWnd::SetPlayPauseButtonState(bool bPlay)
{
	this->Invoke([this, bPlay](bool close){
		if (close) return;

		this->_buttonIsPlayState = bPlay;
		if (_playButton.m_hWnd)
		{
			if (bPlay)
			{
				_playButton.SetImage(IDB_BITMAP_CONTROL_PLAY, IDB_BITMAP_CONTROL_PLAY);
			}
			else
			{
				_playButton.SetImage(IDB_BITMAP_CONTROL_PAUSE, IDB_BITMAP_CONTROL_PAUSE);
			}

			_playButton.Invalidate(TRUE);
		}
	});
}

void ControlPanelWnd::SetBitmap(Bitmap * bitmap)
{
	this->Invoke([this, bitmap](bool close){
		if (close)
		{
			delete bitmap;
			return;
		}

		if (_bitmap)
			delete _bitmap;
		if (this->m_hWnd)
		{
			_bitmap = bitmap;
			this->Invalidate();
		}
		else
		{
			delete bitmap;
		}
	});
}

void ControlPanelWnd::EnableButton(bool bEnable)
{
	this->Invoke([this, bEnable](bool close){
		if (close) return;

		if (_playButton.m_hWnd != 0)
		{
			if (_buttonEnabled != bEnable)
			{
				this->_playButton.EnableWindow(bEnable ? TRUE : FALSE);
			}
		}

		if (_singleStepButton.m_hWnd != 0)
		{
			if (_buttonEnabled != bEnable)
			{
				this->_singleStepButton.EnableWindow(bEnable ? TRUE : FALSE);
			}
		}

		_buttonEnabled = bEnable;
	});
}

BEGIN_MESSAGE_MAP(ControlPanelWnd, Invokable)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_COMMAND(ControlPanelWnd::ID_PLAY_BUTTON, &ControlPanelWnd::OnPlayPauseButtonClicked)
	ON_COMMAND(ControlPanelWnd::ID_SINGLE_STEP_BUTTON, &ControlPanelWnd::OnSingleStepButtonClicked)
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// ControlPanelWnd message handlers




void ControlPanelWnd::OnSize(UINT nType, int cx, int cy)
{
	Invokable::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	this->Invoke([this](bool close){
		if (close) return;

		if (this->m_hWnd == 0)
			return;

		this->AdjustLayout();
		CRect rect;
		this->GetClientRect(&rect);
		rect.left = _trackBarLeft;
		rect.right = _trackBarRight;
		EventTraceBarSizeChanged.Raise(this, rect);
	});
}


int ControlPanelWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (Invokable::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	//_playButton.m_bTransparent = TRUE;
	_playButton.SetImage(IDB_BITMAP_CONTROL_PAUSE, IDB_BITMAP_CONTROL_PAUSE);

	_playButton.Create(L"", BS_PUSHBUTTON | BS_OWNERDRAW | WS_VISIBLE | WS_CHILD,
        rectDummy, this, ID_PLAY_BUTTON);

	_singleStepButton.SetImage(IDB_BITMAP_SINGLESTEP, IDB_BITMAP_SINGLESTEP);

	_singleStepButton.Create(L"", BS_PUSHBUTTON | BS_OWNERDRAW | WS_VISIBLE | WS_CHILD,
		rectDummy, this, ID_SINGLE_STEP_BUTTON);

	return 0;
}

void ControlPanelWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call Invokable::OnPaint() for painting messages

	//CBrush brush(RGB(0, 0, 0));
	CRect rect;
	GetClientRect(&rect);
	//dc.FillRect(rect,&brush);

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


	this->DrawCaption(memGraph);
	this->DrawTrackBar(memGraph);

	Graphics graphics(dc.m_hDC);
	graphics.DrawImage(&bmp,rect.left,rect.top,rect.right,rect.bottom);
	
	delete memGraph;
}


BOOL ControlPanelWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	
	cs.style |= WS_CLIPCHILDREN;

	return Invokable::PreCreateWindow(cs);
}

void ControlPanelWnd::OnPlayPauseButtonClicked()
{
	EventPlayPause.Raise(this, _buttonIsPlayState);

	//_playButton.SetWindowText(_buttonIsPlayState ? L"||" : L">");
	if (_playButton.m_hWnd)
	{
		if (_buttonIsPlayState)
		{
			_playButton.SetImage(IDB_BITMAP_CONTROL_PAUSE, IDB_BITMAP_CONTROL_PAUSE);
		}
		else
		{
			_playButton.SetImage(IDB_BITMAP_CONTROL_PLAY, IDB_BITMAP_CONTROL_PLAY);	
		}

		_playButton.Invalidate(TRUE);

		_buttonIsPlayState = !_buttonIsPlayState;
	}
}

void ControlPanelWnd::OnSingleStepButtonClicked()
{
	EventSingleStep.Raise(this, true);
	this->SetPlayPauseButtonState(true);
}

void ControlPanelWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	EventClosed.Raise(this, true);
	EventPlayPause.Reset();
	EventSingleStep.Reset();
	EventTraceBarSizeChanged.Reset();
	EventTraceBarWheel.Reset();
	EventTraceBarSeek.Reset();
	EventClosed.Reset();

	Invokable::PostNcDestroy();
}


BOOL ControlPanelWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default

	EventTraceBarWheel.Raise(this, zDelta > 0);

	return Invokable::OnMouseWheel(nFlags, zDelta, pt);
}


void ControlPanelWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	int width = _trackBarRight - _trackBarLeft;
	if (width > 0)
	{
		if (point.x >= _trackBarLeft &&
			point.y < _trackBarRight)
		{
			double pos = ((double)(point.x - _trackBarLeft)) / width;
			EventTraceBarSeek.Raise(this, pos);
		}
	}

	Invokable::OnLButtonDown(nFlags, point);
}


void ControlPanelWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetFocus();

	Invokable::OnMouseMove(nFlags, point);
}
