// SubSpectrumPlotWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SubSpectrumPlotWnd.h"
#include "HelperFunc.h"

// SubSpectrumPlotWnd

IMPLEMENT_DYNAMIC(SubSpectrumPlotWnd, CWnd)

SubSpectrumPlotWnd::SubSpectrumPlotWnd()
{
	dataBuf = 0;
	dataBufLen = 0;
	bmp = 0;
}

SubSpectrumPlotWnd::~SubSpectrumPlotWnd()
{
	if (dataBuf)
		delete dataBuf;

	if (bmp)
		delete bmp;

	//std::map<void *, NewData *>::iterator iter;
	//for (iter = this->dataMap.begin();
	//	iter != this->dataMap.end();
	//	iter++)
	//{
	//	delete (*iter).second;	
	//}
}

BEGIN_MESSAGE_MAP(SubSpectrumPlotWnd, CWnd)
	ON_WM_PAINT()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// SeriesGraph message handlers

void SubSpectrumPlotWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);

	graphics.DrawImage(bmp,rect.left,rect.top,rect.right,rect.bottom);
}

LRESULT SubSpectrumPlotWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_QUERY_TYPE:
		{
		this->GetParent()->SendMessage(WM_APP, wParam, lParam);
		return 0;
		}
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


void SubSpectrumPlotWnd::DrawGrid(Graphics * g)
{
	CRect clientRect;
	GetClientRect(&clientRect);

	int gridData = this->CalcOptimizedGridSize();
	if (gridData == 0)
		return;

	__int64 ycenterData = ((__int64)(this->plotWndProp->GetDispMaxValue() + this->plotWndProp->GetDispMinValue()) / 2) / gridData * gridData;

	int fontReserve = 20;

	for (__int64 yData = ycenterData;
		yData < this->plotWndProp->GetDispMaxValue();
		yData += gridData)
	{
		float yView = GetClientY((double)yData);
		if (yView - clientRect.top < fontReserve)
			continue;
		DrawGridLine(g, yData);
	}

	for (__int64 yData = ycenterData - gridData;
		yData > this->plotWndProp->GetDispMinValue();
		yData -= gridData)
	{
		float yView = GetClientY((double)yData);
		if (clientRect.bottom - yView < fontReserve)
			continue;
		DrawGridLine(g, yData);
	}

	DrawXAxis(g);
}

void SubSpectrumPlotWnd::DrawXAxis(Graphics * g)
{
	// draw x-axis
	if ( this->plotWndProp->seriesInPlotWndProp.size() == 0 )
		return;

	SeriesProp * seriesProp = this->plotWndProp->seriesInPlotWndProp[0];
	DebuggingProcessProp * process = seriesProp->GetProcess();

	double maxX = (double)plotWndProp->replayReadPos / process->replayCapacity;
	double minX = (double)(plotWndProp->replayReadPos - plotWndProp->frameCount) / process->replayCapacity;


	SolidBrush fontBrush(Color(255, 150, 150, 150));
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	format.SetFormatFlags(StringFormatFlagsNoWrap);
	format.SetTrimming(StringTrimmingEllipsisCharacter);
	Gdiplus::Font captionFont(L"Arial", 10);
	PointF pointF(5, 2);

	CRect rectClient;
	GetClientRect(&rectClient);

	RectF rectBound(0, 0, 500, 500);
	RectF rectMeasure;
	g->MeasureString(L"0", -1, &captionFont, rectBound, &rectMeasure);
	float height = rectMeasure.Height;

	CString strValueMin;
	strValueMin.Format(L"%.3f", minX);

	RectF rectMin(
		(Gdiplus::REAL)0,
		(Gdiplus::REAL)(rectClient.bottom - height),
		(Gdiplus::REAL)rectClient.Width()/2,
		(Gdiplus::REAL)height );
	g->DrawString(strValueMin, -1, &captionFont, rectMin, &format, &fontBrush);

	CString strValueMax;
	strValueMax.Format(L"%.3f", maxX);



	//int offsetY = rectClient.right - rectMax.Y)
	format.SetAlignment(StringAlignmentFar);
	RectF rectMax(
		(Gdiplus::REAL)rectClient.Width()/2,
		(Gdiplus::REAL)rectClient.bottom-height,
		(Gdiplus::REAL)rectClient.Width()/2,
		(Gdiplus::REAL)height);
	g->DrawString(strValueMax, wcslen(strValueMax.GetBuffer()), &captionFont, rectMax, &format, &fontBrush);
}

void SubSpectrumPlotWnd::DrawBackground(Graphics * g)
{
	CRect clientRect;
	this->GetClientRect(&clientRect);
	SolidBrush brush(Color(255, 0, 0, 0));
	g->FillRectangle(&brush, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
}

#if 0
void SubSpectrumPlotWnd::DrawDataFrame(Graphics * g, int totalFrame, int frameIndex, int frameDispIndex, SpectrumSeriesProp * seriesProp)
{
	if (frameIndex < 0)
		return;

	int * data = (int *)seriesProp->replayBuffer[frameIndex].data;
	ASSERT(data);

	Color color;
	if (seriesProp->colorIsSet)
		color.SetFromCOLORREF(seriesProp->color);
	else
		color.SetFromCOLORREF(RGB(0, 255,0));

	Pen pen(color);

	int frameLen = seriesProp->replayBuffer[frameIndex].len / sizeof(int);	//TODO

	int lastX = GetClientX(0, frameLen, frameDispIndex, totalFrame);
	int lastY = GetClientY(data[0]);

	for (int i = 1; i < frameLen; i++)
	{
		int x = GetClientX(i, frameLen, frameDispIndex, totalFrame);
		int y = GetClientY(data[i]);

		g->DrawLine(&pen, lastX, lastY, x, y);

		lastX = x;
		lastY = y;
	}
}

#endif


void SubSpectrumPlotWnd::DrawData(Graphics * g, SpectrumSeriesProp * seriesProp, bool testAutoScaleOnly)
{
	int startIndex = plotWndProp->replayReadPos - 1;

	double autoScaleMaxValue;
	double autoScaleMinValue;
	bool autoScaleReset = true;

	while ( startIndex >= 0 )
	{
		int len = seriesProp->replayBuffer[startIndex].len;
		if (len > 0)
			break;

		startIndex--;
	}

	Color color;
	if (seriesProp->colorIsSet)
		color.SetFromCOLORREF(seriesProp->color);
	else
		color.SetFromCOLORREF(RGB(0, 255,0));

	Pen pen(color);

	//int firstFrameIndex = seriesProp->GetProcess()->replayReadPos - 1;
	int firstFrameIndex = startIndex;
	if (firstFrameIndex < 0)
		return;

	if (seriesProp->spectrumDataSize == 0)
	{
		if (seriesProp->smSeriesInfo == 0)
			return;
		else
		{
			SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)seriesProp->smSeriesInfo->GetAddress();
			seriesProp->spectrumDataSize = sharedSeriesInfo->spectrumDataSize; 
		}
	}


	//SharedSeriesInfo * sharedSeriesInfo = (SharedSeriesInfo *)seriesProp->smSeriesInfo->GetAddress();
	int dataCount = seriesProp->spectrumDataSize;
	int frameDataCount = seriesProp->replayBuffer[firstFrameIndex].len / sizeof(int);

	if (frameDataCount < dataCount)
		return;

	if (dataCount < 1)
		return;

	int startIdx = frameDataCount - dataCount;
	int * data = (int *)seriesProp->replayBuffer[firstFrameIndex].data;

	CRect rectClient;
	this->GetClientRect(&rectClient);

	float lastX, lastY;
	//plotWndProp->autoScaleReset = true;
	for (int i = 0; i < dataCount; i++)
	{
		double dataY = ::TransformCoordinate(data[i+startIdx], plotWndProp->isLog);
		if (testAutoScaleOnly)
		{
			if (autoScaleReset)
			{
				autoScaleMaxValue = dataY;
				autoScaleMinValue = dataY;
				autoScaleReset = false;
			}
			else
			{
				autoScaleMaxValue = max(autoScaleMaxValue, dataY);
				autoScaleMinValue = min(autoScaleMinValue, dataY);
			}
		}
		else
		{
			double rangeOld = plotWndProp->GetDispMaxValue() - plotWndProp->GetDispMinValue();
			double rangeNew = rectClient.bottom - rectClient.top;
			float dispY =  float(rectClient.bottom - 
				(dataY - plotWndProp->GetDispMinValue()) * rangeNew / rangeOld);

			if (dataCount == 1)
			{
				float dispX = float(rectClient.Width() / 2);
				g->DrawEllipse(&pen, dispX, dispY, 5.0f, 5.0f);
				//g->DrawLine(&pen, lastX, lastY, dispX, dispY);
			}
			else
			{
				float dispX = float(rectClient.left + rectClient.Width() * i / (dataCount-1));

				if (i == 0)
				{
					lastX = dispX;
					lastY = dispY;
				}
				else
				{
					g->DrawLine(&pen, lastX, lastY, dispX, dispY);
					lastX = dispX;
					lastY = dispY;
				}
			}
		}
	}

	if (testAutoScaleOnly)
	{
		const float SCALE_FACTOR = 0.7f;
		double center = (autoScaleMaxValue + autoScaleMinValue) / 2;
		double range = autoScaleMaxValue - autoScaleMinValue;
		double maxValueD = center + range * 2/3;
		double maxValue32 = maxValueD > LONG_MAX ? LONG_MAX : maxValueD;
		double minValueD = center - range * 2/3;
		double minValue32 = minValueD < LONG_MIN ? LONG_MIN : minValueD;

		if (maxValue32 == minValue32)
		{
			if (maxValue32 == INT_MAX)
				minValue32 -= 2;
			else if (minValue32 == INT_MIN)
				maxValue32 += 2;
			else
			{
				maxValue32 += 1;
				minValue32 -= 1;
			}
		}

		bool valueChanged = false;

		if (plotWndProp->autoScaleReset)
		{
			plotWndProp->autoScaleReset = false;
			plotWndProp->SetDispMaxValue(maxValue32);
			plotWndProp->SetDispMinValue(minValue32);
			valueChanged = true;
			AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Max Value");				
			AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Min Value");				
		}
		else
		{
			double lastMaxValue = plotWndProp->GetDispMaxValue();
			if (lastMaxValue < maxValue32)
			{
				plotWndProp->SetDispMaxValue(maxValue32);
				if (lastMaxValue != plotWndProp->GetDispMaxValue())
				{
					AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Max Value");				
				}
			}

			double lastMinValue = plotWndProp->GetDispMinValue();
			if (lastMinValue > minValue32)
			{
				plotWndProp->SetDispMinValue(minValue32);
				if (lastMinValue != plotWndProp->GetDispMinValue())
				{
					AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Min Value");				
				}
			}
		}

		//if (valueChanged)
		//{
		//	BaseProperty * property = this->plotWndProp->GetPropertyPage();
		//	AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY_VALUE, (LPARAM)property);
		//}
	}
}

Gdiplus::REAL SubSpectrumPlotWnd::GetClientY(double y)
{
	CRect clientRect;
	this->GetClientRect(&clientRect);
	double rangeOld = this->plotWndProp->GetDispMaxValue() - this->plotWndProp->GetDispMinValue();
	int rangeNew = clientRect.bottom - clientRect.top;
	return float(clientRect.bottom - 
		(y - this->plotWndProp->GetDispMinValue()) * rangeNew / rangeOld);
}

//int SubSpectrumPlotWnd::GetClientX(int x, int count, int frameIdx, int frameCount)
//{
//	CRect clientRect;
//	this->GetClientRect(&clientRect);
//	int left = clientRect.right * frameIdx / frameCount;
//	int right = clientRect.right * (frameIdx + 1) / frameCount;
//	if (count == 1)
//		return (left + right)/2;
//	return x * (right - left) / (count - 1) + left;
//}

//int SubSpectrumPlotWnd::GetClientX(int x, int count)
//{
//	CRect clientRect;
//	this->GetClientRect(&clientRect);
//	return x * (clientRect.right - clientRect.left) / (count - 1) + clientRect.left;
//}

int SubSpectrumPlotWnd::CalcOptimizedGridSize()
{
	double rangeData = this->plotWndProp->GetDispMaxValue() - this->plotWndProp->GetDispMinValue();
	CRect clientRect;
	this->GetClientRect(&clientRect);
	int rangeView = clientRect.Height();

	if (rangeView == 0)
		return 0;

	int gridView = rangeView / 8;
	gridView = max(gridView, 30);

	double gridData = gridView * rangeData / rangeView;
	__int64 test10_64 = 1;
	while(test10_64 < gridData)
		test10_64 *= 10;

	int test10 = (int)min(test10_64, INT_MAX);

	int candidate[3];
	candidate[0] = test10/5;
	candidate[1] = test10/2;
	candidate[2] = test10;

	for (int i = 0; i < 3; i++)
	{
		if (candidate[i] > gridData)
			return candidate[i];
	}

	return 0;
}

void SubSpectrumPlotWnd::DrawGridLine(Graphics * g, __int64 yData)
{
	CRect clientRect;
	this->GetClientRect(&clientRect);
	int ycenter = (clientRect.top + clientRect.bottom) / 2;
	int xleft = clientRect.left;
	int xright = clientRect.right;

	Gdiplus::REAL fontHeight = 10.0;

	int yView = (int)this->GetClientY((double)yData);

	// Draw data
	Pen pen(Color(255, 50, 50, 50));
	pen.SetDashStyle(DashStyleDash);
	Gdiplus::Font gridFont(L"Arial", fontHeight);
	SolidBrush fontBrush(Color(255, 150, 150, 150));
	RectF layoutRect(0.0f, 0.0f, 200.0f, 50.0f);
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	
	int fontLeftMargin = 0;

	CString strValue;

	::FormatCommaString(strValue, (__int64)::ReverseTransformCoordinate((double)yData, plotWndProp->isLog));

	PointF pointF((Gdiplus::REAL)(xleft + fontLeftMargin), (Gdiplus::REAL)(yView - 16));
	g->DrawString(strValue, -1, &gridFont, pointF, &format, &fontBrush);

	// Draw line
	g->DrawLine(&pen, xleft, yView, xright, yView);
}

void SubSpectrumPlotWnd::SetPlotWndProp(PlotWndProp * prop)
{
	this->plotWndProp = (PlotWndPropSpectrum *)prop;
}

// SubSpectrumPlotWnd message handlers


BOOL SubSpectrumPlotWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);
}


int SubSpectrumPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	
	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_REGISTER_WND, (LPARAM)this);

	this->plotWndProp = (PlotWndPropSpectrum *)lpCreateStruct->lpCreateParams;

	return 0;
}


void SubSpectrumPlotWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_UNREGISTER_WND, (LPARAM)this);

	delete this;

	CWnd::PostNcDestroy();
}


void SubSpectrumPlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	LPARAM lParam = MAKELONG(point.x, point.y);

	GetParent()->SendMessage(WM_LBUTTONDOWN, nFlags, lParam);

	CWnd::OnLButtonDown(nFlags, point);
}


BOOL SubSpectrumPlotWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default

	double maxValue = this->plotWndProp->GetDispMaxValue();
	double minValue = this->plotWndProp->GetDispMinValue();
	double centerValue = (maxValue + minValue) / 2;
	__int64 range = (__int64)(maxValue - minValue);
	__int64 delta;

	if (zDelta < 0)
		delta = range;
	else
		delta = range / 4;

	delta = max(delta, 2);

	int newMaxValue;
	int newMinValue;
	if (!plotWndProp->isLog)
	{
		newMaxValue = (int)min(centerValue + delta, INT_MAX);
		newMinValue = (int)max(centerValue - delta, INT_MIN);
	}
	else
	{
		newMaxValue = (int)min(centerValue + delta, ::TransformCoordinate(INT_MAX, true));
		newMinValue = (int)max(centerValue - delta, 0);
	}

	this->plotWndProp->SetDispMaxValue(newMaxValue);
	this->plotWndProp->SetDispMinValue(newMinValue);
	this->plotWndProp->autoScale = false;

	this->plotWndProp->targetWnd->PostMessage(WM_APP, CMD_FORCE_UPDATE_GRAPH, 0);

	//BaseProperty * property = this->plotWndProp->GetPropertyPage();
	//::AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);
	::AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Max Value");
	::AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Min Value");
	::AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Auto Scale");


	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


void SubSpectrumPlotWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	SetFocus();

	CWnd::OnMouseMove(nFlags, point);
}
