// PlotWndArea.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewerDoc.h"
#include "DebugPlotViewer.h"
#include "PlotWndArea.h"
#include "PlotWndPropLine.h"
#include "PlotWndPropDots.h"
#include "PlotWndPropSpectrum.h"
#include "PlotWndPropText.h"
#include "PlotWndPropLog.h"
#include "SeriesLine.h"
#include "SeriesDots.h"
#include "SeriesSpectrum.h"
#include "SeriesText.h"
#include "SeriesLog.h"
#include "AppMessage.h"

// PlotWndArea

IMPLEMENT_DYNAMIC(PlotWndArea, CWnd)

PlotWndArea::PlotWndArea()
{
	this->plotWndAreaProp = 0;
}

PlotWndArea::~PlotWndArea()
{
}


BEGIN_MESSAGE_MAP(PlotWndArea, CWnd)
	ON_WM_PAINT()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_CREATE()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// PlotWndArea message handlers


void PlotWndArea::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	CRect rect;
	GetClientRect(rect);

	if (dragHighlight)
	{
		CBrush greenBrush(RGB(0, 50, 0));
		dc.FillRect(rect, &greenBrush);
	}
	else
	{
		CBrush greyBrush(RGB(50, 50, 50));
		dc.FillRect(rect, &greyBrush);
	}

}

LRESULT PlotWndArea::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_QUERY_TYPE:
		{
		TargetWnd * targetWnd = (TargetWnd *)lParam;
		targetWnd->type = WND_TYPE_VIEW;
		targetWnd->wnd = this;
		return 0;
		}

	case CMD_HIGHLIGHT_WND:
		{
		dragHighlight = (lParam != 0);
		return 0;
		}

	case CMD_NEW_DOCUMENT:
		this->doc = (CDebugPlotViewerDoc *)lParam;
		return 0;

	case CMD_AUTO_LAYOUT:
		this->AutoLayout((int)lParam);
		return 0;

	case CMD_NEW_PLOT_WND:
		CreatePlotWnd((PlotWndProp *)lParam);
		return 0;

	default:
		return 0;
	}
}

CPlotWnd * PlotWndArea::CreatePlotWnd(PlotWndProp * plotWndProp)
{
	// snap to grid
	int gridSize = this->plotWndAreaProp->gridSize;
	plotWndProp->rect.left -= plotWndProp->rect.left % gridSize;
	plotWndProp->rect.top -= plotWndProp->rect.top % gridSize;
	plotWndProp->rect.right -= plotWndProp->rect.right % gridSize;
	plotWndProp->rect.bottom -= plotWndProp->rect.bottom % gridSize;

	CPlotWnd * wnd = plotWndProp->GetPlotWnd(this);
	return wnd;
}

void PlotWndArea::AutoLayout(int pos)
{
	CRect clientRect;
	this->GetClientRect(&clientRect);

	int size = this->plotWndAreaProp->autoLayoutSize = pos;
	size = (size - (size % this->plotWndAreaProp->gridSize));
	if (size == 0)
		return;

	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	int xNum = max(clientWidth / size, 1);

	int index = 0;
	std::vector<PlotWndProp *>::iterator iter;
	for (iter = this->plotWndAreaProp->plotWndsInPlotWndAreaProp.begin();
		iter != this->plotWndAreaProp->plotWndsInPlotWndAreaProp.end();
		iter++)
	{
		int xIndex = index % xNum;
		int yIndex = index / xNum;		
		(*iter)->targetWnd->MoveWindow(xIndex * size, yIndex *size, size, size, true);
		index++;
	}
}

int PlotWndArea::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	this->dragHighlight = false;
	return 0;
}

BOOL PlotWndArea::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);
}


void PlotWndArea::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnRButtonDown(nFlags, point);
}


void PlotWndArea::OnContextMenu(CWnd* /*pWnd*/pWnd, CPoint /*point*/point)
{
	// TODO: Add your message handler code here
	//CWnd::OnContextMenu(pWnd, point);
}


void PlotWndArea::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (this->plotWndAreaProp)
	{
		BaseProperty * property = this->plotWndAreaProp->GetPropertyPage();
		::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);
	}
	CWnd::OnLButtonDown(nFlags, point);
}

PlotWndAreaProp * PlotWndArea::GetProp()
{
	return this->plotWndAreaProp;
}

bool PlotWndArea::TestTarget(void * obj)
{
	return true;
}

void PlotWndArea::AddToTarget(void * obj)
{
	auto dragSeriesProp = (SeriesProp *)obj;

	PlotWndProp * plotWndProp = 0;

	if (typeid(*dragSeriesProp) == typeid(LineSeriesProp))
		plotWndProp = new PlotWndPropLine();
	else if (typeid(*dragSeriesProp) == typeid(DotsSeriesProp))
		plotWndProp = new PlotWndPropDots();
	else if (typeid(*dragSeriesProp) == typeid(TextSeriesProp))
		plotWndProp = new PlotWndPropText();
	else if (typeid(*dragSeriesProp) == typeid(LogSeriesProp))
		plotWndProp = new PlotWndPropLog();
	else if (typeid(*dragSeriesProp) == typeid(SpectrumSeriesProp))
		plotWndProp = new PlotWndPropSpectrum();

	ASSERT(plotWndProp);

	// rect
	CPoint screenCursorPoint;
	GetCursorPos(&screenCursorPoint);
	CPoint clientPoint = screenCursorPoint;
	this->ScreenToClient(&clientPoint);
	plotWndProp->rect.left = clientPoint.x;
	plotWndProp->rect.top = clientPoint.y;
	plotWndProp->rect.right = plotWndProp->rect.left + doc->plotWndAreaProp.initialPlotWndWidth;
	plotWndProp->rect.bottom = plotWndProp->rect.top + doc->plotWndAreaProp.initialPlotWndHeight;

	dragSeriesProp->rect.SetRectEmpty();

	// add plot wnd to plot wnd area
	PlotWndAreaProp * plotWndAreaProp = this->plotWndAreaProp;
	plotWndProp->SetPlotWndArea(plotWndAreaProp);											// 1
	plotWndAreaProp->AddPlotWnd(plotWndProp);
	// 2
	// Create CWnd object
	//targetWnd.wnd->SendMessage(WM_APP, CMD_NEW_PLOT_WND, (LPARAM)plotWndProp);				// 3
	this->CreatePlotWnd(plotWndProp);

	// add series to plot wnd
	plotWndProp->AddSeries(dragSeriesProp);													// 7
	dragSeriesProp->SetPlotWndProp(plotWndProp);											// 8

	dragSeriesProp->isOpened = true;
	dragSeriesProp->GetProcess()->openCount++;
	::AfxGetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_CONTROLLER, 0);

	// Create CWnd object
	//plotWndProp->targetWnd->SendMessage(WM_APP, CMD_ADD_SERIES, (LPARAM)dragSeriesProp);	// 9
}

void PlotWndArea::HighLight(bool highlight)
{
	dragHighlight = highlight;
	this->Invalidate(1);
}
