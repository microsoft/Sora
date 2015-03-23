#include "stdafx.h"
#include <algorithm>
#include "PlotWnd.h"
#include "Debug.h"
#include "Logger.h"
#include "AppMessage.h"
#include "SubPlotWnd.h"
#include "SubBitmapPlotWnd.h"
#include "SubTextPlotWnd.h"

#include "BitmapPlotWnd.h"

#ifdef DEBUG_USE_VLD
#include "vld.h"
#endif


CPlotWnd::CPlotWnd()// : graph(NULL)
{
}

CPlotWnd::~CPlotWnd()
{
}

BEGIN_MESSAGE_MAP(CPlotWnd, Invokable)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_WINDOWPOSCHANGING()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

BOOL CPlotWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if ( !Invokable::PreCreateWindow(cs) )
		return FALSE;

	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	return TRUE;
}

int CPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( Invokable::OnCreate(lpCreateStruct) == -1 )
		return -1;

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_REGISTER_WND, (LPARAM)this);

	dragHighlight = false;
	isDraging = false;

	this->userData = lpCreateStruct->lpCreateParams;

	return 0;
}

void CPlotWnd::OnPaint()
{
	CPaintDC dc(this);
	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);
	Bitmap bmp(rect.right,rect.bottom);

	Graphics* memGraph = Graphics::FromImage(&bmp);

	this->DrawFrame(memGraph);

	CRect clientRect;
	this->CalcClientRect(&clientRect);
	SolidBrush brush(Color::Black);
	memGraph->FillRectangle(
		&brush,
		clientRect.left,
		clientRect.top,
		clientRect.Width(),
		clientRect.Height()
		);

	this->DrawCaption(memGraph);
	this->DrawCloseButton(memGraph);

	graphics.DrawImage(&bmp,rect.left,rect.top,rect.right,rect.bottom);

	delete memGraph;

	return;
}

LRESULT CPlotWnd::OnNcHitTest(CPoint point)
{
	CRect rect;
	GetClientRect(&rect);
	ClientToScreen(&rect);

	const int MARGIN = 5;

	if (point.y - rect.top < MARGIN)
	{
		if (point.x - rect.left < MARGIN)
			return HTTOPLEFT;
		else if (rect.right - point.x < MARGIN)
			return HTTOPRIGHT;
		else
			return HTTOP;
	}
	else if (rect.bottom - point.y < MARGIN)
	{
		if (point.x - rect.left < MARGIN)
			return HTBOTTOMLEFT;
		else if (rect.right - point.x < MARGIN)
			return HTBOTTOMRIGHT;
		else
			return HTBOTTOM;		
	}
	else
	{
		if (point.x - rect.left < MARGIN)
			return HTLEFT;
		else if (rect.right - point.x < MARGIN)
			return HTRIGHT;
		else
			return HTCLIENT;
	}

	return Invokable::OnNcHitTest(point);
}

void CPlotWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	Invokable::OnWindowPosChanging(lpwndpos);

	int GRID;
	NullParam param;
	bool succ = StrategyGetGridSize.Call(this, param, GRID);
	ASSERT(succ);

	int x2, y2;

	if (lpwndpos->x < 0)
	{
		lpwndpos->x = 0;
		x2 = lpwndpos->x + lpwndpos->cx;
	}
	else
	{
		x2 = lpwndpos->x + lpwndpos->cx;
		lpwndpos->x = lpwndpos->x / GRID * GRID;
	}
	if (lpwndpos->y < 0)
	{
		lpwndpos->y = 0;
		y2 = lpwndpos->y + lpwndpos->cy;
	}
	else
	{
		y2 = lpwndpos->y + lpwndpos->cy;
		lpwndpos->y = lpwndpos->y / GRID * GRID;
	}

	x2 = x2 / GRID * GRID;
	y2 = y2 / GRID * GRID;

	lpwndpos->cx = x2 - lpwndpos->x;
	lpwndpos->cy = y2 - lpwndpos->y;

	lpwndpos->cx = max(lpwndpos->cx, 20);
	lpwndpos->cy = max(lpwndpos->cy, 20);
}

LRESULT CPlotWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_QUERY_TYPE:
		{
		TargetWnd * targetWnd = (TargetWnd *)lParam;
		targetWnd->type = WND_TYPE_PLOT;
		targetWnd->wnd = this;
		return 0;
		}
	case CMD_HIGHLIGHT_WND:
		{
		dragHighlight = (lParam != 0);
		return 0;
		}
	default:
		return 0;
	}
}

void CPlotWnd::CreateSeries(SeriesProp * seriesProp)
{
	CRect clientRect;
	this->CalcClientRect(&clientRect);

	return;
}

void CPlotWnd::DrawCloseButton(Graphics * g)
{
	// draw a simple cross

	CRect rectCloseButton;

	this->CalcCloseButtonRect(&rectCloseButton);

	Color color;
	color.SetFromCOLORREF(RGB(70,110,140));
	Pen pen(color);
	
	g->DrawLine(&pen, 
		rectCloseButton.left, 
		rectCloseButton.top,
		rectCloseButton.right,
		rectCloseButton.bottom);

	g->DrawLine(&pen,
		rectCloseButton.right,
		rectCloseButton.top,
		rectCloseButton.left,
		rectCloseButton.bottom);

}


void CPlotWnd::CalcCloseButtonRect(CRect * rect)
{
	if (!rect)
		return;

	this->CalcCaptionRect(rect);

	const int CLOSE_BUTTON_MARGIN = 5;
	const int CLOSE_BUTTON_SIZE = 10;

	rect->right -= CLOSE_BUTTON_MARGIN;
	rect->left = rect->right - CLOSE_BUTTON_SIZE;
	rect->top += CLOSE_BUTTON_MARGIN;
	rect->bottom = rect->top + CLOSE_BUTTON_SIZE;
}

void CPlotWnd::CalcCaptionRect(CRect * rect)
{
	if (!rect)
		return;

	this->GetClientRect(rect);
	rect->bottom = rect->top + 20;
}

void CPlotWnd::CalcClientRect(CRect * rect)
{
	if (!rect)
		return;

	this->GetClientRect(rect);

	const int MARGIN = 3;

	rect->top += 20;
	rect->left += MARGIN;
	rect->right -= MARGIN;
	rect->bottom -= MARGIN;
}

void CPlotWnd::DrawFrame(Graphics * g)
{
	CRect rect;
	this->GetClientRect(&rect);

	const COLORREF FRAME_HIGHTLIGHT_COLOR = RGB(0, 50, 0);
	const COLORREF FRAME_COLOR = RGB(28,48,69);

	Color color;
	if (this->dragHighlight)
		color.SetFromCOLORREF(FRAME_HIGHTLIGHT_COLOR);
	else
		color.SetFromCOLORREF(FRAME_COLOR);

	SolidBrush brush(color);
	g->FillRectangle(&brush, rect.left, rect.top, rect.right, rect.bottom);
}

void CPlotWnd::DrawCaption(Graphics * g)
{
	SolidBrush fontBrush(Color(255, 150, 150, 150));
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	format.SetFormatFlags(StringFormatFlagsNoWrap);
	format.SetTrimming(StringTrimmingEllipsisCharacter);
	Gdiplus::Font captionFont(L"Arial", 10);
	PointF pointF(5, 2);

	const int MARGIN = 3;
	const int CLOSE_BUTTON_SIZE = 10;

	CRect rectCaption;
	this->CalcCaptionRect(&rectCaption);
	RectF rect(
		(Gdiplus::REAL)rectCaption.left,
		(Gdiplus::REAL)rectCaption.top,
		(Gdiplus::REAL)(rectCaption.right - (MARGIN + CLOSE_BUTTON_SIZE + 10)),
		(Gdiplus::REAL)rectCaption.bottom );

	CString name;
	NullParam param;
	bool succ = this->StrategyGetCaption.Call(this, param, name);
	ASSERT(succ);
	
	g->DrawString(name, -1, &captionFont, rect, &format, &fontBrush);
}

void CPlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	this->BringWindowToTop();

	CRect rectCaption, rectCloseButton;
	this->CalcCaptionRect(&rectCaption);
	this->CalcCloseButtonRect(&rectCloseButton);
	
	if (::PtInRect(&rectCloseButton, point) )
	{
		this->PostMessage(WM_CLOSE, 0, 0);
	}
	else if (::PtInRect(&rectCaption, point) )
	{
		if (!this->isDraging)
		{
			this->GetWindowRect(&this->rectBeforeDrag);
			this->GetParent()->ScreenToClient(&this->rectBeforeDrag);
			::GetCursorPos(&this->cursorBeforeDrag);

			this->SetCapture();
			this->isDraging = true;
		}

		BringToFrontEvent e;
		EventBringToFront.Raise(this, e);
	}
	else
	{
		BringToFrontEvent e;
		EventBringToFront.Raise(this, e);	
	}

	Invokable::OnLButtonDown(nFlags, point);
}


void CPlotWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if (this->isDraging)
	{
		CPoint cursorPoint;
		::GetCursorPos(&cursorPoint);

		CRect rectWindow = this->rectBeforeDrag;
		rectWindow.OffsetRect(cursorPoint - this->cursorBeforeDrag);
		this->MoveWindow(rectWindow.left, rectWindow.top, rectWindow.Width(), rectWindow.Height(), 1);

	}
	else
	{
		CRect rectCloseButton;
		this->CalcCloseButtonRect(&rectCloseButton);
		if ( ::PtInRect(&rectCloseButton, point) )
		{
			::SetCursor(::LoadCursor(NULL, IDC_HAND));			
		}
		else
		{
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));			
		}
	}

	Invokable::OnMouseMove(nFlags, point);
}


void CPlotWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (this->isDraging)
	{
		::ReleaseCapture();
		this->isDraging = false;
	}

	Invokable::OnLButtonUp(nFlags, point);
}


void CPlotWnd::OnCaptureChanged(CWnd *pWnd)
{
	// TODO: Add your message handler code here
	this->isDraging = false;

	CRect rectWindow;
	this->GetWindowRect(&rectWindow);
	this->GetParent()->ScreenToClient(&rectWindow);

	MoveEvent e;
	e.rect = rectWindow;
	this->EventMove.Raise(this, e);

	Invokable::OnCaptureChanged(pWnd);
}


void CPlotWnd::OnSize(UINT nType, int cx, int cy)
{
	Invokable::OnSize(nType, cx, cy);

	CRect rectWindow;
	this->GetWindowRect(&rectWindow);
	this->GetParent()->ScreenToClient(&rectWindow);

	MoveEvent e;
	e.rect = rectWindow;
	this->EventMove.Raise(this, e);

	//ASSERT(this->plotWndProp);

	//// TODO: Add your message handler code here
	//CRect rectWindow;
	//this->GetWindowRect(&rectWindow);
	//this->GetParent()->ScreenToClient(&rectWindow);

	//// update doc data
	//this->plotWndProp->rect.SetRect(
	//	rectWindow.left, 
	//	rectWindow.top, 
	//	rectWindow.right, 
	//	rectWindow.bottom );

	//if (graph)
	//{
	//	CRect clientRect;
	//	this->CalcClientRect(&clientRect);
	//	graph->MoveWindow(&clientRect, 1);
	//}

	//this->plotWndProp->ForceUpdateGraph();
}


BOOL CPlotWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return Invokable::OnEraseBkgnd(pDC);
}



//bool TextPlotWnd::Accept(SeriesProp * prop)
//{
//	if ( (this->plotWndProp->seriesInPlotWndProp.size() > 0) )
//		return false;
//
//	vector<SeriesProp *>::iterator iter;
//	for (iter = plotWndProp->seriesInPlotWndProp.begin();
//		iter != plotWndProp->seriesInPlotWndProp.end();
//		iter++)
//	{
//		if (*iter == prop)
//			return false;
//	}
//
//	if ( typeid(*prop) == typeid(TextSeriesProp) )
//		return true;
//	else
//		return false;
//}

//bool LogPlotWnd::Accept(SeriesProp * prop)
//{
//	if ( (this->plotWndProp->seriesInPlotWndProp.size() > 0) )
//		return false;
//
//	vector<SeriesProp *>::iterator iter;
//	for (iter = plotWndProp->seriesInPlotWndProp.begin();
//		iter != plotWndProp->seriesInPlotWndProp.end();
//		iter++)
//	{
//		if (*iter == prop)
//			return false;
//	}
//
//	if ( typeid(*prop) == typeid(LogSeriesProp) )
//		return true;
//	else
//		return false;
//}

//bool SpectrumPlotWnd::Accept(SeriesProp * prop)
//{
//	if ( (this->plotWndProp->seriesInPlotWndProp.size() > 0) &&
//		prop->GetProcess() != this->plotWndProp->seriesInPlotWndProp[0]->GetProcess())
//		return false;
//
//	vector<SeriesProp *>::iterator iter;
//	for (iter = plotWndProp->seriesInPlotWndProp.begin();
//		iter != plotWndProp->seriesInPlotWndProp.end();
//		iter++)
//	{
//		if (*iter == prop)
//			return false;
//	}
//
//	if ( (typeid(*prop) == typeid(SpectrumSeriesProp)) || (typeid(*prop) == typeid(TextSeriesProp)) && !((TextSeriesProp *)prop)->isLog )
//		return true;
//	else
//		return false;
//}

void CPlotWnd::OnClose()
{
	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_UNREGISTER_WND, (LPARAM)this);

	Invokable::OnClose();
}

void CPlotWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	
	NullEvent e;
	this->EventClosed.Raise(this, e);
	delete this;
/*
	this->plotWndProp->NotifyClosed();

	delete this->plotWndProp;
	delete this;*/

	Invokable::PostNcDestroy();
}

void * CPlotWnd::GetUserData()
{
	return userData;
}

bool CPlotWnd::TestTarget(void * obj)
{
	TestTargetParam p;
	p.obj = obj;
	bool ret;
	bool succ = StragegyTestTarget.Call(this, p, ret);
	if (succ && ret)
		return true;
	else
		return false;
}

BOOL CALLBACK CPlotWnd::UpdateAllChild(HWND hwnd, LPARAM lParam)
{
	::InvalidateRgn(hwnd, NULL, 1);
	return TRUE;
}

void CPlotWnd::AddToTarget(void * obj)
{
	AddTargetEvent e;
	e.obj = obj;
	EventAddTarget.Raise(this, e);
}

void CPlotWnd::HighLight(bool highlight)
{
	this->dragHighlight = highlight;

	this->Invalidate(1);
}

BOOL CPlotWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default

	return Invokable::OnMouseWheel(nFlags, zDelta, pt);
}

//void CPlotWnd::AddSubPlotWnd(SubPlotWnd * subWnd)
//{
//	SubPlotWnd * subWnd = new SubPlotWnd;
//	subWnd->plotWndProp = plotWndProp;
//	subWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, prop->rect, this->graph, 0, prop);
//	prop->SetTargetWnd(subWnd);
//	subWnd->seriesProp = (TextSeriesProp *)prop;
//
//	/*ASSERT(this->plotWndProp);
//
//	if ( (typeid(*prop) == typeid(TextSeriesProp)) &&
//		(typeid(*prop->GetPlotWndProp()) != typeid(PlotWndPropText)))
//	{
//		SubPlotWnd * subWnd = new SubPlotWnd;
//		subWnd->plotWndProp = plotWndProp;
//		subWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, prop->rect, this->graph, 0, prop);
//		prop->SetTargetWnd(subWnd);
//		subWnd->seriesProp = (TextSeriesProp *)prop;
//	}
//	else
//	{
//		prop->SetTargetWnd(this->graph);
//	}
//
//	prop->isOpened = true;
//	prop->GetProcess()->openCount++;*/
//}
