#include "stdafx.h"
#include "BitmapPlotWnd.h"

BEGIN_MESSAGE_MAP(BitmapPlotWnd, CPlotWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


BitmapPlotWnd::BitmapPlotWnd(void * userData) :
	CPlotWnd(userData)
{
	_userData = userData;
}

int BitmapPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPlotWnd::OnCreate(lpCreateStruct) == -1 )
		return -1;
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	_canvasWnd = new CWnd();
	_canvasWnd->Create(NULL, L"control", WS_CHILD | WS_VISIBLE, rectDummy, this, 0);

	_controlWnd = new PlayControlWnd;
	_controlWnd->EventPlayClicked.Subscribe([this](const void * sender, const PlayControlWnd::PlayPauseEvent e){
		PlayPauseEvent ee;
		ee.IsPlay = e.IsPlay;
		this->EventPlayClicked.Raise(this, ee);
	});

	_controlWnd->EventSpeedClicked.Subscribe([this](const void * sender, const PlayControlWnd::SpeedChangeEvent e){
		SpeedChangeEvent ee;
		ee.IsUp = e.IsUp;
		this->EventSpeedClicked.Raise(this, ee);
	});

	_controlWnd->EventTrackBarSeeked.Subscribe([this](const void * sender, const PlayControlWnd::TrackBarSeekEvent & e) {
		SeekEvent ee;
		ee.Pos = e.Pos;
		this->EventSeek.Raise(this, ee);
	});

	_controlWnd->Create(NULL, L"control", WS_CHILD | WS_VISIBLE, rectDummy, _canvasWnd, 0);

	_subBitmapWnd = new SubBitmapPlotWnd(_userData);

	_subBitmapWnd->EventWheel.Subscribe([this](const void * sender, const SubBitmapPlotWnd::WheelEvent & e) {
		WheelEvent ee;
		ee.IsUp = e.IsUp;
		this->EventWheel.Raise(this, ee);
	});

	_subBitmapWnd->EventClicked.Subscribe([this](const void * sender, const SubBitmapPlotWnd::ClickEvent & e) {
		this->BringWindowToTop();
		CPlotWnd::BringToFrontEvent ee;
		this->EventBringToFront.Raise(this, ee);
	});

	_subBitmapWnd->Create(NULL, L"bitmap", WS_CHILD | WS_VISIBLE, rectDummy, _canvasWnd, 0);

	return 0;
}

void BitmapPlotWnd::OnSize(UINT nType, int cx, int cy)
{
	const int CTRL_WND_HEIGHT = 17;
	CRect clientRect;
	this->CalcClientRect(&clientRect);

	CRect canvasRect = clientRect;

	_canvasWnd->MoveWindow(&canvasRect);

	CRect controlRect;
	controlRect.SetRectEmpty();
	controlRect.right += clientRect.Width();
	controlRect.bottom += clientRect.Height();
	controlRect.top = controlRect.bottom - CTRL_WND_HEIGHT;

	_controlWnd->MoveWindow(&controlRect, 1);

	if (true)
	{
		CRect graphRect;
		graphRect.SetRectEmpty();
		graphRect.right += clientRect.Width();
		graphRect.bottom = controlRect.top;
		_subBitmapWnd->MoveWindow(&graphRect, 1);
		ResizeEvent e;
		e.cx = graphRect.Width();
		e.cy = graphRect.Height();
		this->EventBitmapResize.Raise(this, e);
	}

	CPlotWnd::OnSize(nType, cx, cy);
}

void BitmapPlotWnd::SetButtonStatePlay()
{
	this->Invoke([this](bool close){
		if (close)
			return;

		if (_controlWnd->m_hWnd)
			_controlWnd->SetButtonStatePlay();
	});
}

void BitmapPlotWnd::SetButtonStatePause()
{
	this->Invoke([this](bool close){
		if (close)
			return;

		if (_controlWnd->m_hWnd)
			_controlWnd->SetButtonStatePause();
	});
}

void BitmapPlotWnd::SetTrackBarViewWndRange(double right, double range)
{
	this->Invoke([this, right, range](bool close){
		if (close)
			return;

		if (_controlWnd->m_hWnd)
			_controlWnd->SetTrackBarWndPos(right, range);
	});
}

void BitmapPlotWnd::SetBitmap(Bitmap * bitmap)
{
	this->Invoke([this, bitmap](bool close){
		if (close)
		{
			delete bitmap;
			return;
		}

		if (_subBitmapWnd->m_hWnd)
			this->_subBitmapWnd->SetBitmap(bitmap);
		else
			delete bitmap;
	});
}

BitmapPlotWnd::~BitmapPlotWnd()
{
	if (_controlWnd)
	{
		delete _controlWnd;
		_controlWnd = 0;
	}
	if (_subBitmapWnd)
	{
		delete _subBitmapWnd;
		_subBitmapWnd = 0;
	}
	if (_canvasWnd)
	{
		delete _canvasWnd;
		_canvasWnd = 0;
	}
}

void BitmapPlotWnd::AddSubPlotWnd(SubPlotWnd * subWnd, const CRect & rect)
{
	this->Invoke([this, subWnd, rect](bool close){
		if (close)
			return;

		if (m_hWnd == 0 || _subBitmapWnd->m_hWnd == 0)
			return;

		CRect rect2 = rect;
		this->_subBitmapWnd->ScreenToClient(rect2);
		subWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, rect2, this->_subBitmapWnd, 0, 0);
	});
}

void BitmapPlotWnd::AddSubPlotWndRelative(SubPlotWnd * subWnd, const CRect & rect)
{	
	this->Invoke([this, subWnd, rect](bool close){
		if (close)
			return;

		if (m_hWnd == 0 || _subBitmapWnd->m_hWnd == 0)
			return;

		CRect rect2 = rect;
		subWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, rect2, this->_subBitmapWnd, 0, 0);
	});
}

void * BitmapPlotWnd::UserData()
{
	return _userData;
}

void BitmapPlotWnd::MyScreenToClient(CRect & rect)
{
	this->_subBitmapWnd->ScreenToClient(&rect);
}

void BitmapPlotWnd::MyScreenToClient(CPoint & point)
{
	this->_subBitmapWnd->ScreenToClient(&point);
}

void BitmapPlotWnd::EnableSpeedButtons(bool bEnable)
{
	this->Invoke([this, bEnable](bool close){
		if (close) return;

		this->_controlWnd->EnableSpeedButtons(bEnable);
	});
}
