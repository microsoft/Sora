// PlotWndContainer.cpp : implementation file
//

#include "stdafx.h"
#include <memory>
#include "MainFrm.h"
#include "DbgPlotViewer.h"
#include "PlotWndContainer.h"
#include "ChannelOpened.h"
#include "ChannelOpenedLine.h"
#include "PlotWindowOpened.h"
#include "PlotWindowOpenedLine.h"
#include "ViewerAppConst.h"

using namespace std;
// PlotWndContainer

IMPLEMENT_DYNAMIC(PlotWndContainer, Invokable)

PlotWndContainer::PlotWndContainer(void * userData)
{
	_bDragHighlight = false;
	_userData = userData;
	_dropRect = CRect(0, 0, ViewerAppConst::PlotWindowSize(), ViewerAppConst::PlotWindowSize());
}

PlotWndContainer::~PlotWndContainer()
{
}

void PlotWndContainer::HighLight(bool highlight)
{
	_bDragHighlight = highlight;
	this->Invalidate(1);
}

void PlotWndContainer::AddPlotWindow(CPlotWnd * wnd, const CRect & rect)
{
	this->Invoke([this, wnd, rect](bool close){
		if (close)
			return;

		if (m_hWnd)
		{
			wnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, rect, this, 0, this);
		}
	});
}

void PlotWndContainer::AddPlotWindow(CPlotWnd * wnd)
{
	this->Invoke([this, wnd](bool close){
		if (close)
			return;

		if (m_hWnd)
		{
			wnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, _dropRect, this, 0, this);
		}
	});
}

void PlotWndContainer::SetDropPoint(CPoint point)
{
	_dropPoint = point;
	_dropRect.left = point.x;
	_dropRect.right = point.x + ViewerAppConst::PlotWindowSize();
	_dropRect.top = point.y;
	_dropRect.bottom = point.y + ViewerAppConst::PlotWindowSize();

}

BEGIN_MESSAGE_MAP(PlotWndContainer, Invokable)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// PlotWndContainer message handlers


void PlotWndContainer::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	CRect rectClient;
	GetClientRect(rectClient);

	CBrush greyBrush(RGB(50, 50, 50));
	dc.FillRect(rectClient, &greyBrush);

	if (_bDragHighlight)
	{
		CBrush greenBrush(RGB(0, 50, 0));
		dc.FillRect(_dropRect, &greenBrush);
	}
}

void PlotWndContainer::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	EventSize.Reset();
	EventClosed.Raise(this, true);

	__super::PostNcDestroy();
}


void PlotWndContainer::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	EventSize.Raise(this, CRect(0, 0, cx, cy));
}


BOOL PlotWndContainer::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= WS_CLIPCHILDREN;

	return __super::PreCreateWindow(cs);
}

void * PlotWndContainer::UserData()
{
	return _userData;
}


void PlotWndContainer::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

#ifdef _DEBUG
	CRect rect;
	GetClientRect(&rect);

	auto file = L"C:\\Users\\mingl\\Desktop\\Untitled.xml";

	auto theMainFrame = (CMainFrame *)::AfxGetMainWnd();

	if (point.x > rect.Width() / 2)
	{
		theMainFrame->OpenDocument(file);
	}
	else
	{
		theMainFrame->SaveDocument(file);
	}

#endif

	__super::OnLButtonDblClk(nFlags, point);
}
