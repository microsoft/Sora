// SubDotsPlotWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SubDotsPlotWnd.h"
#include "HelperFunc.h"

// SubDotsPlotWnd

IMPLEMENT_DYNAMIC(SubDotsPlotWnd, CWnd)

SubDotsPlotWnd::SubDotsPlotWnd()
{
	bmp = 0;
}

SubDotsPlotWnd::~SubDotsPlotWnd()
{
	if (bmp)
		delete bmp;
}


BEGIN_MESSAGE_MAP(SubDotsPlotWnd, CWnd)
	ON_WM_PAINT()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// SeriesGraph message handlers

void SubDotsPlotWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);

	graphics.DrawImage(bmp,rect.left,rect.top,rect.right,rect.bottom);
}


void SubDotsPlotWnd::DrawData(Graphics * g, DotsSeriesProp * seriesProp, bool testAutoScaleOnly)
{
	int nDotsDrawn = 0;

	CRect rectClient;
	this->GetClientRect(&rectClient);

	double oldRange = this->plotWndProp->GetDispMaxValue() * 2;
	int newRangeX = rectClient.Width();
	int newRangeY = rectClient.Height();

	Color color;
	if (seriesProp->colorIsSet)
		color.SetFromCOLORREF(seriesProp->color);
	else
		color.SetFromCOLORREF(RGB(0, 255,0));

	int redColor = color.GetRed();
	int greenColor = color.GetGreen();
	int blueColor = color.GetBlue();

	for (int frameIndex = plotWndProp->replayReadPos - 1; frameIndex >= 0; frameIndex--)
	{
		int frameDataCount = seriesProp->replayBuffer[frameIndex].len / sizeof(COMPLEX16);
		if (frameDataCount == 0)
			continue;

		COMPLEX16 * data = (COMPLEX16 *)seriesProp->replayBuffer[frameIndex].data;
		
		ASSERT(data);

		for (int dataIdx = 0; dataIdx < frameDataCount; dataIdx++)
		{
			int alphaDot;
			if (plotWndProp->luminescence)
				alphaDot = 255 * (plotWndProp->dotShowCount - nDotsDrawn) / plotWndProp->dotShowCount;
			else
				alphaDot = 255;

			Color colorFrame(alphaDot, redColor, greenColor, blueColor);

			Pen pen(colorFrame);

			if (testAutoScaleOnly)
			{
				plotWndProp->autoScaleMaxValue = max(plotWndProp->autoScaleMaxValue, abs(data[dataIdx].re));
				plotWndProp->autoScaleMaxValue = max(plotWndProp->autoScaleMaxValue, abs(data[dataIdx].im));
			}
			else
			{
				Gdiplus::REAL re = float((data[dataIdx].re - (-this->plotWndProp->GetDispMaxValue())) * newRangeX / oldRange + 0);
				Gdiplus::REAL im = float((rectClient.bottom - (data[dataIdx].im - (-this->plotWndProp->GetDispMaxValue())) * newRangeY / oldRange));

				g->DrawLine(&pen, re-1.0f, im, re+1.0f, im);
				g->DrawLine(&pen, re, im-1.0f, re, im+1.0f);
			}

			nDotsDrawn++;
			if (nDotsDrawn > this->plotWndProp->dotShowCount)
				break;
		}

		if (nDotsDrawn > this->plotWndProp->dotShowCount)
			break;
	}

	if (testAutoScaleOnly)
	{
		double maxValueD = plotWndProp->autoScaleMaxValue * 3/2;
		int maxValue32 = (int)min(maxValueD, INT_MAX);
		maxValue32 = max(maxValue32, 1);

		if (plotWndProp->GetDispMaxValue() != maxValue32)
		{
			plotWndProp->SetDispMaxValue(maxValue32);
			BaseProperty * property = this->plotWndProp->GetPropertyPage();
			AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY_VALUE, (LPARAM)property);
		}
	}
}

LRESULT SubDotsPlotWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_QUERY_TYPE:
		this->GetParent()->SendMessage(WM_APP, wParam, lParam);
		return 0;
	case CMD_NEW_DOCUMENT:
		this->doc = (CDebugPlotViewerDoc *)lParam;
		return 0;
	case CMD_UPDATE_VIEW:
		this->InvalidateRgn(NULL, 0);
		return 0;
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

void SubDotsPlotWnd::DrawGrid(Graphics * g)
{
	CRect clientRect;
	this->GetClientRect(&clientRect);
	int ycenter = (clientRect.top + clientRect.bottom) / 2;
	int xcenter = (clientRect.left + clientRect.right) / 2;
	int xleft = clientRect.left;
	int xright = clientRect.right;
	int ytop = clientRect.top;
	int ybottom = clientRect.bottom;

	Pen pen(Color(255, 50, 50, 50));
	pen.SetDashStyle(DashStyleDash);

	// horizantal line
	g->DrawLine(&pen, xleft, ycenter, xright, ycenter);

	// vertical line
	g->DrawLine(&pen, xcenter, ytop, xcenter, ybottom);

	// outer ellipse
	g->DrawEllipse(
		&pen, 
		0, 
		0, 
		clientRect.Width(), 
		clientRect.Height() );

	// innner ellipse

	if ( min(clientRect.Width(), clientRect.Height() ) > 100 )
	{
		g->DrawEllipse(
			&pen,
			clientRect.right / 4,
			clientRect.bottom / 4,
			clientRect.Width() / 2,
			clientRect.Height() / 2 );

		// draw text
		SolidBrush fontBrush(Color(255, 150, 150, 150));
		StringFormat format;
		format.SetAlignment(StringAlignmentNear);
		CString strValue;
		::FormatCommaString(strValue, this->plotWndProp->GetDispMaxValue());
		CString strDisp;
		strDisp.Format(L"r = %s", strValue);
		Gdiplus::Font gridFont(L"Arial", 10);
		PointF pointF((Gdiplus::REAL)(xleft + 5), (Gdiplus::REAL)(ybottom - 16));
		g->DrawString(strDisp, -1, &gridFont, pointF, &format, &fontBrush);
	}
}


void SubDotsPlotWnd::DrawBackground(Graphics * g)
{
	CRect clientRect;
	this->GetClientRect(&clientRect);

	SolidBrush brush(Color(255, 0, 0, 0));
	g->FillRectangle(&brush, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
}


void SubDotsPlotWnd::DrawDot(Graphics * g, COMPLEX16 data, Color& color)
{
	SolidBrush brush(color);
	g->FillEllipse(&brush, data.re-2, data.im-2, 5, 5);
}


BOOL SubDotsPlotWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CWnd::OnEraseBkgnd(pDC);
	//return TRUE;
}

void SubDotsPlotWnd::SetPlotWndProp(PlotWndProp * prop)
{
	this->plotWndProp = (PlotWndPropDots *)prop;
}

BOOL SubDotsPlotWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);
}


int SubDotsPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_REGISTER_WND, (LPARAM)this);

	this->plotWndProp = (PlotWndPropDots *)lpCreateStruct->lpCreateParams;

	return 0;
}


void SubDotsPlotWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_UNREGISTER_WND, (LPARAM)this);
	delete this;

	CWnd::PostNcDestroy();
}


void SubDotsPlotWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	SetFocus();

	CWnd::OnMouseMove(nFlags, point);
}


BOOL SubDotsPlotWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default

	do {
		if (this->plotWndProp->seriesInPlotWndProp.size() == 0)
			break;

		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);
		if (!rect.PtInRect(pt))
			break;		

		int nDotsShow = 1;
		while(nDotsShow < this->plotWndProp->dotShowCount)
		{
			nDotsShow *= 2;
		}

		if (zDelta > 0)
		{
			if (nDotsShow >= this->plotWndProp->dotShowCount)
				nDotsShow /= 2;
			nDotsShow = max(1, nDotsShow);
		}
		else
		{
			if (nDotsShow == this->plotWndProp->dotShowCount)
				nDotsShow *= 2;
			nDotsShow = min(nDotsShow, PlotWndPropDots::MAX_DOTS_SHOWN);
		}

		int lastDotsShown = plotWndProp->dotShowCount;
		plotWndProp->dotShowCount = nDotsShow;
		if (nDotsShow != lastDotsShown)
		{
			BaseProperty * property = this->plotWndProp->GetPropertyPage();
			::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);
			this->GetParent()->PostMessageW(WM_APP, CMD_FORCE_UPDATE_GRAPH, 0);		
		}
	} while(0);

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


void SubDotsPlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	LPARAM lParam = MAKELONG(point.x, point.y);

	GetParent()->SendMessage(WM_LBUTTONDOWN, nFlags, lParam);

	CWnd::OnLButtonDown(nFlags, point);
}
