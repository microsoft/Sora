// SubPlotWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SubPlotWnd.h"
#include "AppMessage.h"

// SubPlotWnd

IMPLEMENT_DYNAMIC(SubPlotWnd, CWnd)

SubPlotWnd::SubPlotWnd()
{
	isStatic = false;
}

SubPlotWnd::~SubPlotWnd()
{
}


BEGIN_MESSAGE_MAP(SubPlotWnd, CWnd)
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_CAPTURECHANGED()
	ON_WM_SIZING()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_WINDOWPOSCHANGING()
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

LRESULT SubPlotWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_QUERY_TYPE:
		{
		this->GetParent()->SendMessage(WM_APP, wParam, lParam);
		return 0;
		}
	case CMD_SERIES_CLOSED:
		this->DestroyWindow();
		return 1;
	//case CMD_UPDATE_VIEW:
	//	this->seriesProp->UpdateSubWnd();
	//	this->InvalidateRgn(NULL, 0);
	//	return 0;
	default:
		return 0;
	}
}



void SubPlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!this->isDraging)
	{
		this->GetWindowRect(&this->rectBeforeDrag);
		this->GetParent()->ScreenToClient(&this->rectBeforeDrag);
		::GetCursorPos(&this->cursorBeforeDrag);

		this->SetCapture();
		this->isDraging = true;

		CWnd::OnLButtonDown(nFlags, point);
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

	CWnd::OnLButtonUp(nFlags, point);
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

		CWnd::OnMouseMove(nFlags, point);
	}
}


int SubPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	
	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_REGISTER_WND, (LPARAM)this);

	isDraging = false;

	return 0;
}


void SubPlotWnd::OnCaptureChanged(CWnd *pWnd)
{
	// TODO: Add your message handler code here
	this->isDraging = false;

	CWnd::OnCaptureChanged(pWnd);
}

void SubPlotWnd::PlotText(CString & string)
{
	this->string.SetString(string);
	this->Invalidate(1);
}

void SubPlotWnd::DrawString(Graphics * g)
{
	Color fontColor;
	
	COLORREF color;
	bool succ = this->StrategyGetColor.Call(this, 0, color);
	ASSERT(succ);

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

	//int frameCount = 1;

	//int stringLen = 0;

	//int validFrameCount = 0;
	//int frameIdx = plotWndProp->replayReadPos;

	//while( (frameIdx > 0) && (validFrameCount < frameCount) )
	//{
	//	frameIdx--;
	//	int frameDataCount = seriesProp->replayBuffer[frameIdx].len / sizeof(char);
	//	if (frameDataCount > 0)
	//	{
	//		validFrameCount++;
	//	}
	//	stringLen += frameDataCount;
	//	if (stringLen > 4*1024)
	//		break;
	//}

	//char * string = new char[stringLen+1];
	//char * dstPtr = string;


	//while( frameIdx < plotWndProp->replayReadPos )
	//{
	//	int frameDataCount = seriesProp->replayBuffer[frameIdx].len / sizeof(char);
	//	if (frameDataCount > 0)
	//	{
	//		memcpy(dstPtr, seriesProp->replayBuffer[frameIdx].data, frameDataCount * sizeof(char));
	//		dstPtr += frameDataCount;
	//	}
	//	frameIdx++;
	//}

	//*dstPtr = 0;

	CString stringToDraw(string);
	g->DrawString(stringToDraw, -1, &font, boundRect, &format, &fontBrush);

	//delete [] string;

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

//void SubPlotWnd::SetPlotWndProp(PlotWndProp * prop)
//{
//	this->plotWndProp = prop;
//}


void SubPlotWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_UNREGISTER_WND, (LPARAM)this);
	delete this;

	CWnd::PostNcDestroy();
}

//void SubPlotWnd::SetData(StringData * dataInfo)
//{
//	RemoveData(dataInfo->sender);
//	data = dataInfo;
//}
//
//void SubPlotWnd::RemoveData(void * sender)
//{
//	if (this->sender)
//	{
//		ASSERT(sender == this->sender);
//	}
//	else
//	{
//		this->sender = 0;
//	}
//
//	if (data)
//	{
//		delete data;
//		data = 0;
//	}
//}


void SubPlotWnd::OnSizing(UINT fwSide, LPRECT pRect)
{
	CWnd::OnSizing(fwSide, pRect);

	// TODO: Add your message handler code here
	CRect rectWindow;
	this->GetWindowRect(&rectWindow);
	this->GetParent()->ScreenToClient(&rectWindow);

	// update doc data
	this->EventMoveWindow.Raise(this, rectWindow);
	//this->seriesProp->rect.SetRect(
	//	rectWindow.left, 
	//	rectWindow.top, 
	//	rectWindow.right, 
	//	rectWindow.bottom );
}


void SubPlotWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanged(lpwndpos);

	// TODO: Add your message handler code here
	CRect rectWindow;
	this->GetWindowRect(&rectWindow);
	this->GetParent()->ScreenToClient(&rectWindow);

	// update doc data
	this->EventMoveWindow.Raise(this, rectWindow);
	//this->seriesProp->rect.SetRect(
	//	rectWindow.left, 
	//	rectWindow.top, 
	//	rectWindow.right, 
	//	rectWindow.bottom );
}


void SubPlotWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanging(lpwndpos);

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
