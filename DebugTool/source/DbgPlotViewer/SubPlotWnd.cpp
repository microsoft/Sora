// SubPlotWnd.cpp : implementation file
//

#include "stdafx.h"
#include "SubPlotWnd.h"

// SubPlotWnd

IMPLEMENT_DYNAMIC(SubPlotWnd, Invokable)

SubPlotWnd::SubPlotWnd()
{
	isStatic = false;
	_color = RGB(0, 255, 0);
}

SubPlotWnd::~SubPlotWnd()
{
}


BEGIN_MESSAGE_MAP(SubPlotWnd, Invokable)
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_CAPTURECHANGED()
	ON_WM_SIZING()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// SubPlotWnd message handlers




LRESULT SubPlotWnd::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if (this->isStatic)
		return HTCLIENT;

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
}


void SubPlotWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);
	Bitmap bmp(rect.right,rect.bottom);

	Graphics* memGraph = Graphics::FromImage(&bmp);

	DrawFrame(memGraph);
	DrawString(memGraph);

	graphics.DrawImage(&bmp,rect.left,rect.top,rect.right,rect.bottom);

	delete memGraph;
}

void SubPlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	this->BringWindowToTop();

	if (!this->isDraging)
	{
		this->GetWindowRect(&this->rectBeforeDrag);
		this->GetParent()->ScreenToClient(&this->rectBeforeDrag);
		::GetCursorPos(&this->cursorBeforeDrag);

		this->SetCapture();
		this->isDraging = true;

		Invokable::OnLButtonDown(nFlags, point);
	}
}


void SubPlotWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (this->isDraging)
	{
		::ReleaseCapture();
		this->isDraging = false;
	}

	Invokable::OnLButtonUp(nFlags, point);
}


void SubPlotWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (this->isDraging)
	{
		CPoint cursorPoint;
		::GetCursorPos(&cursorPoint);
		CPoint cursorPointOld = cursorPoint;

		CRect parentClientRect;
		this->GetParent()->GetClientRect(&parentClientRect);
		this->GetParent()->ClientToScreen(&parentClientRect);

		cursorPoint.x = max(cursorPoint.x, parentClientRect.left);
		cursorPoint.x = min(cursorPoint.x, parentClientRect.right);
		cursorPoint.y = max(cursorPoint.y, parentClientRect.top);
		cursorPoint.y = min(cursorPoint.y, parentClientRect.bottom);

		if (cursorPoint != cursorPointOld)
			::SetCursorPos(cursorPoint.x, cursorPoint.y);
		CRect rectWindow = this->rectBeforeDrag;
		rectWindow.OffsetRect(cursorPoint - this->cursorBeforeDrag);
		this->MoveWindow(rectWindow.left, rectWindow.top, rectWindow.Width(), rectWindow.Height(), 1);

		Invokable::OnMouseMove(nFlags, point);
	}
}


int SubPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (Invokable::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	HWND hwnd = this->m_hWnd;

	isDraging = false;

	return 0;
}


void SubPlotWnd::OnCaptureChanged(CWnd *pWnd)
{
	// TODO: Add your message handler code here
	this->isDraging = false;

	Invokable::OnCaptureChanged(pWnd);
}

void SubPlotWnd::PlotText(CString & string)
{
	this->Invoke([this, string](bool close){
		if (close) return;

		this->string.SetString(string);
		if (this->m_hWnd)
			this->Invalidate(1);
	});
}

void SubPlotWnd::DrawString(Graphics * g)
{
	Color fontColor;
	
	COLORREF color = _color;

	fontColor.SetFromCOLORREF(color);

	SolidBrush fontBrush(fontColor);
	
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	Gdiplus::Font font(L"Arial", 10);
	PointF pointF(5, 2);
	CRect rect;
	this->GetClientRect(&rect);
	RectF boundRect(
		(Gdiplus::REAL)rect.left,
		(Gdiplus::REAL)rect.top,
		(Gdiplus::REAL)rect.right,
		(Gdiplus::REAL)rect.bottom
		);

	CString stringToDraw(string);
	g->DrawString(stringToDraw, -1, &font, boundRect, &format, &fontBrush);
}


void SubPlotWnd::DrawFrame(Graphics * g)
{
	CRect clientRect;
	this->GetClientRect(&clientRect);
	RectF rect(
		(Gdiplus::REAL)clientRect.left,
		(Gdiplus::REAL)clientRect.top,
		(Gdiplus::REAL)(clientRect.right - 1),
		(Gdiplus::REAL)(clientRect.bottom - 1)
		);

	SolidBrush brush(Color(255, 0, 0, 0));
	g->FillRectangle(&brush, rect);
	if (!this->isStatic)
	{
		Pen pen(Color(255, 100, 100, 100));
		g->DrawRectangle(&pen, rect);
	}
}

void SubPlotWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	EventMoveWindow.Reset();
	EventClosed.Raise(this, true);
	
	Invokable::PostNcDestroy();
}

void SubPlotWnd::OnSizing(UINT fwSide, LPRECT pRect)
{
	Invokable::OnSizing(fwSide, pRect);

	// TODO: Add your message handler code here
	//CRect rectWindow;
	//this->GetWindowRect(&rectWindow);
	//this->GetParent()->GetParent()->ScreenToClient(&rectWindow);

	//this->EventMoveWindow.Raise(this, rectWindow);
}


void SubPlotWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	Invokable::OnWindowPosChanged(lpwndpos);

	// TODO: Add your message handler code here
	CRect rectWindow;
	this->GetWindowRect(&rectWindow);
	this->GetParent()->ScreenToClient(&rectWindow);

	this->EventMoveWindow.Raise(this, rectWindow);
}


void SubPlotWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	Invokable::OnWindowPosChanging(lpwndpos);

	return;

	// TODO: Add your message handler code here
	const int GRID = 6;

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

	lpwndpos->cx = max(lpwndpos->cx, GRID);
	lpwndpos->cy = max(lpwndpos->cy, GRID);
}

void SubPlotWnd::SetColor(COLORREF color)
{
	this->Invoke([this, color](bool close){
		if (close) return;

		this->_color = color;
	});
}


void SubPlotWnd::OnSize(UINT nType, int cx, int cy)
{
	Invokable::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	// TODO: Add your message handler code here
	CRect rectWindow;
	this->GetWindowRect(&rectWindow);
	this->GetParent()->ScreenToClient(&rectWindow);

	this->EventMoveWindow.Raise(this, rectWindow);
}
