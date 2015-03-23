#include "stdafx.h"
#include "BitmapPlotWnd.h"
#include "SeriesLine.h"
#include "SeriesDots.h"
#include "SeriesObj.h"

BEGIN_MESSAGE_MAP(BitmapPlotWnd, CPlotWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

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

	_subBitmapWnd = new SubBitmapPlotWnd;

	_subBitmapWnd->EventWheel.Subscribe([this](const void * sender, const SubBitmapPlotWnd::WheelEvent & e) {
		WheelEvent ee;
		ee.IsUp = e.IsUp;
		this->EventWheel.Raise(this, ee);
	});

	_subBitmapWnd->EventClicked.Subscribe([this](const void * sender, const SubBitmapPlotWnd::ClickEvent & e) {
		this->BringWindowToTop();
	});

	_subBitmapWnd->StrategyTestTarget.Set([this](const void * sender, const SubBitmapPlotWnd::TestTargetParam & p, bool & ret) {
		void * obj = p.obj;
		ret = this->TestTarget(obj);
	});

	_subBitmapWnd->EventAddTarget.Subscribe([this](const void * sender, const SubBitmapPlotWnd::AddTargetEvent & e) {
		this->AddToTarget(e.obj);
	});

	_subBitmapWnd->EventHighLight.Subscribe([this](const void * sender, const SubBitmapPlotWnd::HighLightEvent & e) {
		this->HighLight(e.highlight);
	});

	_subBitmapWnd->Create(NULL, L"bitmap", WS_CHILD | WS_VISIBLE, rectDummy, _canvasWnd, 0);

	return 0;
}

void BitmapPlotWnd::OnSize(UINT nType, int cx, int cy)
{
	CRect clientRect;
	this->CalcClientRect(&clientRect);

	_canvasWnd->MoveWindow(&clientRect);

	CRect controlRect;
	controlRect.SetRectEmpty();
	controlRect.right += clientRect.Width();
	controlRect.bottom += clientRect.Height();
	controlRect.top = controlRect.bottom - 17;

	_controlWnd->MoveWindow(&controlRect, 1);

	if (controlRect.top > 0) // now the graphic window can be visible
	{
		CRect graphRect;
		graphRect.SetRectEmpty();
		graphRect.right += clientRect.Width();
		graphRect.bottom = controlRect.top;
		_subBitmapWnd->MoveWindow(&graphRect, 1);
		ResizeEvent e;
		e.cx = graphRect.Width();
		e.cy = graphRect.Height();
		this->EventResize.Raise(this, e);
	}

	CPlotWnd::OnSize(nType, cx, cy);
}

void BitmapPlotWnd::SetButtonStatePlay()
{
	_controlWnd->SetButtonStatePlay();
}

void BitmapPlotWnd::SetButtonStatePause()
{
	_controlWnd->SetButtonStatePause();
}

void BitmapPlotWnd::SetTrackBarViewWndRange(double right, double range)
{
	_controlWnd->SetTrackBarWndPos(right, range);
}

void BitmapPlotWnd::UpdateView()
{
	_subBitmapWnd->Invalidate(1);
}


void BitmapPlotWnd::SetBitmap(Bitmap * bitmap)
{
	_subBitmapWnd->SetBitmap(bitmap);
}

BitmapPlotWnd::~BitmapPlotWnd()
{
	if (_controlWnd)
		delete _controlWnd;
	if (_subBitmapWnd)
		delete _subBitmapWnd;
	if (_canvasWnd)
		delete _canvasWnd;
}

void BitmapPlotWnd::AddSubPlotWnd(SubPlotWnd * subWnd, CRect & rect, void * userData)
{
	this->_subBitmapWnd->ScreenToClient(rect);
	
	subWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, rect, this->_subBitmapWnd, 0, userData);
}

void BitmapPlotWnd::AddSubPlotWndRelative(SubPlotWnd * subWnd, CRect & rect, void * userData)
{	
	subWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, rect, this->_subBitmapWnd, 0, userData);
}

