// SubBitmapPlotWnd.cpp : implementation file
//

#include "stdafx.h"
#include <limits.h>
#include "DebugPlotViewer.h"
#include "SubBitmapPlotWnd.h"
#include "HelperFunc.h"
#include "AppMessage.h"

// SubBitmapPlotWnd

IMPLEMENT_DYNAMIC(SubBitmapPlotWnd, CWnd)

SubBitmapPlotWnd::SubBitmapPlotWnd()
{
	bmp = 0;
}

SubBitmapPlotWnd::~SubBitmapPlotWnd()
{
	if (bmp)
	{
		delete bmp;
	}
}

BEGIN_MESSAGE_MAP(SubBitmapPlotWnd, CWnd)
	ON_WM_PAINT()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_CREATE()
	ON_WM_MOUSEHWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// SeriesGraph message handlers

void SubBitmapPlotWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);

	if (bmp)
		graphics.DrawImage(bmp,rect.left,rect.top,rect.right,rect.bottom);
}

LRESULT SubBitmapPlotWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_QUERY_TYPE:
		{
		this->GetParent()->SendMessage(WM_APP, wParam, lParam);
		return 0;
		}
	case CMD_UPDATE_BITMAP:
		{
			if (bmp)
			{
				delete bmp;
			}
			MsgUpdateBmp * msg = (MsgUpdateBmp *)lParam;
			bmp = msg->bmp;
			delete msg;
			this->InvalidateRgn(NULL, 0);
		}
		return 0;
	default:
		return 0;
	}
}

BOOL SubBitmapPlotWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);
}

int SubBitmapPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_REGISTER_WND, (LPARAM)this);

	return 0;
}

void SubBitmapPlotWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_UNREGISTER_WND, (LPARAM)this);
	//delete this;

	CWnd::PostNcDestroy();
}

void SubBitmapPlotWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	SetFocus();

	CWnd::OnMouseMove(nFlags, point);
}


BOOL SubBitmapPlotWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	WheelEvent e;
	e.IsUp = zDelta > 0;
	EventWheel.Raise(this, e);

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void SubBitmapPlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);

	ClickEvent e;
	e.x = point.x;
	e.y = point.y;
	EventClicked.Raise(this, e);

	CWnd::OnLButtonDown(nFlags, point);
}

void SubBitmapPlotWnd::SetBitmap(Bitmap * bitmap)
{
	if (bmp)
		delete bmp;

	bmp = bitmap;
}

bool SubBitmapPlotWnd::TestTarget(void * obj)
{
	TestTargetParam p;
	p.obj = obj;
	bool ret;
	bool succ = StrategyTestTarget.Call(this, p, ret);
	if (succ && ret)
		return true;
	else
		return false;
}

void SubBitmapPlotWnd::AddToTarget(void * obj)
{
	AddTargetEvent e;
	e.obj = obj;
	EventAddTarget.Raise(this, e);
}

void SubBitmapPlotWnd::HighLight(bool highlight)
{
	HighLightEvent e;
	e.highlight = highlight;
	EventHighLight.Raise(this, e);
}
