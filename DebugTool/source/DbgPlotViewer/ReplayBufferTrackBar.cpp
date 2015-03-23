// ReplayBufferTrackBar.cpp : implementation file
//

#include "stdafx.h"
#include "ReplayBufferTrackBar.h"

// ReplayBufferTrackBar

IMPLEMENT_DYNAMIC(ReplayBufferTrackBar, CWnd)

ReplayBufferTrackBar::ReplayBufferTrackBar()
{
	_range = 0.0;
	_right = 0.0;
}

ReplayBufferTrackBar::~ReplayBufferTrackBar()
{
}


BEGIN_MESSAGE_MAP(ReplayBufferTrackBar, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// ReplayBufferTrackBar message handlers


void ReplayBufferTrackBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);
	Bitmap bmp(rect.right,rect.bottom);

	Graphics* memGraph = Graphics::FromImage(&bmp);

	SolidBrush brushBg(Color(100, 100, 100));
	memGraph->FillRectangle(
		&brushBg,
		rect.left,
		rect.top,
		rect.Width(),
		rect.Height()
		);

	CRect rectViewWnd = rect;
	rectViewWnd.right = (LONG)(rect.Width() * _right);
	rectViewWnd.left = (LONG)(rectViewWnd.right - rect.Width() * _range);

	if (rectViewWnd.right >= rectViewWnd.left)
	{
		// Color(28, 48, 69)
		SolidBrush brushViewWnd(Color(170, 170, 170));
		memGraph->FillRectangle(
			&brushViewWnd,
			rectViewWnd.left,
			rectViewWnd.top,
			rectViewWnd.Width(),
			rectViewWnd.Height()
			);
	}

	graphics.DrawImage(&bmp,rect.left,rect.top,rect.right,rect.bottom);
	
	delete memGraph;
}


void ReplayBufferTrackBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	double pos = 0.0;
	CRect clientRect;
	this->GetClientRect(&clientRect);

	if (clientRect.Width() > 0)
		pos = (double)point.x / clientRect.Width();

	SeekEvent e;
	e.pos = pos;

	EventSeek.Raise(this, e);

	CWnd::OnLButtonDown(nFlags, point);
}

void ReplayBufferTrackBar::SetViewWnd(double right, double range)
{
	_right = right;
	_range = range;
	this->Invalidate(1);
}

