#include "stdafx.h"
//#include "DebugPlotViewerDoc.h"
#include "SeriesLineType.h"
#include "PlotWndPropLineType.h"
#include "PlotWnd.h"
#include "SubBitmapPlotWnd.h"
#include "HelperFunc.h"
#include "BaseProperty.h"
#include "LinePlotWndProperty.h"
#include "AppMessage.h"

/***********************************

Line Plot Wnd

************************************/
PlotWndPropLineType::PlotWndPropLineType()
{
	maxValue = 100;
	minValue = -100;
	autoScaleReset = true;
	autoScale = true;
	showGrid = true;
	isLog = false;
}

void PlotWndPropLineType::SeekTextWnd()
{
	vector<unsigned long long> vecTimeStamp;

	for_each(this->seriesInPlotWndProp.begin(), this->seriesInPlotWndProp.end(), [this, &vecTimeStamp](SeriesProp * series){
		LineTypeSeriesProp * lineSeries = dynamic_cast<LineTypeSeriesProp *>(series);
		if (lineSeries)
		{
			unsigned long long timestamp;
			bool succ = lineSeries->GetTimeStamp(this->LatestIdx(), timestamp);
			if (succ)
				vecTimeStamp.push_back(timestamp);
		}
	});

	for_each(this->seriesInPlotWndProp.begin(), this->seriesInPlotWndProp.end(), [&vecTimeStamp](SeriesProp * series){
		TextSeriesProp * textSeries = dynamic_cast<TextSeriesProp *>(series);
		if (textSeries)
		{
			textSeries->SeekTimeStamp(vecTimeStamp);
		}
	});
}

Gdiplus::REAL PlotWndPropLineType::GetClientY(double y, const CRect & clientRect)
{
	double rangeOld = this->GetDispMaxValue() - this->GetDispMinValue();
	int rangeNew = clientRect.bottom - clientRect.top;
	return float(clientRect.bottom - 
		(y - this->GetDispMinValue()) * rangeNew / rangeOld);
}

void PlotWndPropLineType::DrawGrid(Graphics * g, const CRect &clientRect)
{

	int gridData = this->CalcOptimizedGridSize(clientRect);
	if (gridData == 0)
		return;

	__int64 ycenterData = ((__int64)(this->GetDispMaxValue() + this->GetDispMinValue()) / 2) / gridData * gridData;

	int fontReserve = 20;

	for (__int64 yData = ycenterData;
		yData < this->GetDispMaxValue();
		yData += gridData)
	{
		float yView = GetClientY((double)yData, clientRect);
		if (yView - clientRect.top < fontReserve)
			continue;
		DrawGridLine(g, yData, clientRect);
	}

	for (__int64 yData = ycenterData - gridData;
		yData > this->GetDispMinValue();
		yData -= gridData)
	{
		float yView = GetClientY((double)yData, clientRect);
		if (clientRect.bottom - yView < fontReserve)
			continue;
		DrawGridLine(g, yData, clientRect);
	}

	DrawXAxis(g, clientRect);
}

//void PlotWndPropLineType::DrawXAxis(Graphics * g, const CRect & rectClient)
//{
//	// draw x-axis
//
//	if ( this->seriesInPlotWndProp.size() == 0 )
//		return;
//
//	SeriesProp * seriesProp = this->seriesInPlotWndProp[0];
//	DebuggingProcessProp * process = seriesProp->GetProcess();
//
//	//double maxX = (double)process->replayReadPos / process->replayCapacity;
//	double maxX = (double)this->replayReadPos / process->replayCapacity;
//	//double minX = (double)(process->replayReadPos - plotWndProp->frameCount) / process->replayCapacity;
//	double minX = (double)(this->replayReadPos - this->frameCount) / process->replayCapacity;
//
//
//	SolidBrush fontBrush(Color(255, 150, 150, 150));
//	StringFormat format;
//	format.SetAlignment(StringAlignmentNear);
//	format.SetFormatFlags(StringFormatFlagsNoWrap);
//	format.SetTrimming(StringTrimmingEllipsisCharacter);
//	Gdiplus::Font captionFont(L"Arial", 10);
//	PointF pointF(5, 2);
//
//
//	RectF rectBound(0, 0, 500, 500);
//	RectF rectMeasure;
//	g->MeasureString(L"0", -1, &captionFont, rectBound, &rectMeasure);
//	float height = rectMeasure.Height;
//
//	CString strCount;
//	strCount.Format(L"%d", this->GetRangeSize());
//	format.SetAlignment(StringAlignmentFar);
//	RectF rectMax(
//		(Gdiplus::REAL)rectClient.Width()/2,
//		(Gdiplus::REAL)rectClient.bottom-height,
//		(Gdiplus::REAL)rectClient.Width()/2,
//		(Gdiplus::REAL)height);
//	g->DrawString(strCount, wcslen(strCount.GetBuffer()), &captionFont, rectMax, &format, &fontBrush);
//}


void PlotWndPropLineType::DrawGridLine(Graphics * g, __int64 yData, const CRect & clientRect)
{

	int ycenter = (clientRect.top + clientRect.bottom) / 2;
	int xleft = clientRect.left;
	int xright = clientRect.right;

	Gdiplus::REAL fontHeight = 10.0;

	int yView = (int)this->GetClientY((double)yData, clientRect);

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
	//::FormatCommaString(strValue, yData);
	::FormatCommaString(strValue, (__int64)::ReverseTransformCoordinate((double)yData, this->isLog));

	PointF pointF((Gdiplus::REAL)(xleft + fontLeftMargin), (Gdiplus::REAL)(yView - 16));
	g->DrawString(strValue, -1, &gridFont, pointF, &format, &fontBrush);

	// Draw line
	g->DrawLine(&pen, xleft, yView, xright, yView);
}

int PlotWndPropLineType::CalcOptimizedGridSize(const CRect & clientRect)
{
	double rangeData = this->GetDispMaxValue() - this->GetDispMinValue();
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

Bitmap * PlotWndPropLineType::CreateBitmap()
{
	CRect rect(0, 0, this->ClientWidth(), this->ClientHeight());
	Bitmap * bmp = new Bitmap(rect.right,rect.bottom);

	Graphics* memGraph = Graphics::FromImage(bmp);

	memGraph->SetSmoothingMode(SmoothingModeAntiAlias);

	double maxOfAll, minOfAll;
	bool initialized = false;
	
	this->DrawBackground(memGraph, rect);

	this->DrawLabel(memGraph);
	CRect clientRect;
	this->GetCanvasRect(clientRect);

	// long ugly auto scale
	if (clientRect.right > clientRect.left && clientRect.bottom > clientRect.top)
	{

		if (showGrid)
		{
			this->DrawGrid(memGraph, clientRect);
		}

		if (autoScale)
		{
			for (auto iter = seriesInPlotWndProp.begin();
				iter != seriesInPlotWndProp.end();
				iter++)
			{
				SeriesProp * seriesPropBase = *iter;
				LineTypeSeriesProp * seriesProp = dynamic_cast<LineTypeSeriesProp *>(seriesPropBase);
				if (seriesProp)
				{
					LineTypeSeriesProp * seriesProp = (LineTypeSeriesProp *)(*iter);
					double max, min;
					size_t rangeSize;
					bool rangeGot = seriesProp->GetMaxMinRange(this->RangeSize(), rangeSize);
					if (rangeGot)
					{
						bool calculatedMaxMin = seriesProp->CalcMaxMin(clientRect, this->LatestIdx(), rangeSize, max, min);
						if (calculatedMaxMin)
						{
							if (!initialized)
							{
								maxOfAll = max;
								minOfAll = min;
								initialized = true;
							}
							else
							{
								maxOfAll = max(max, maxOfAll);
								minOfAll = min(min, minOfAll);
							}
						}
					}
				}
			}
		}

		if (initialized)
		{
			const float SCALE_FACTOR = 0.7f;
			double center = (maxOfAll + minOfAll) / 2;
			double range = maxOfAll - minOfAll;
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

			double lastMaxValue = GetDispMaxValue();
			double lastMinValue = GetDispMinValue();

			if (lastMaxValue < maxValue32 ||
				lastMinValue > minValue32 ||
				lastMaxValue - lastMinValue > (maxValue32 - minValue32) * 1.5)
			{
				SetDispMaxValue(maxValue32);
				if (lastMaxValue != GetDispMaxValue())
				{
					AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Max Value");	
				}

				SetDispMinValue(minValue32);
				if (lastMinValue != GetDispMinValue())
				{
					AfxGetApp()->GetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PROPERTY_PANEL, (LPARAM)L"Min Value");				
				}
			}
		}


		std::vector<SeriesProp *>::iterator iter;
		for (iter = seriesInPlotWndProp.begin();
			iter != seriesInPlotWndProp.end();
			iter++)
		{
			SeriesProp * seriesPropBase = *iter;
			LineTypeSeriesProp * seriesProp = dynamic_cast<LineTypeSeriesProp *>(seriesPropBase);
			if (seriesProp)
			{
				seriesProp->Draw(memGraph, clientRect, this->LatestIdx(), this->RangeSize());
			}
		}

	}
	delete memGraph;

	return bmp;
}

void PlotWndPropLineType::DrawBackground(Graphics * g, const CRect & clientRect)
{
	SolidBrush brush(Color(255, 0, 0, 0));
	g->FillRectangle(&brush, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
}

size_t PlotWndPropLineType::MaxDataSize()
{
	size_t maxSize = 0;
	std::for_each(
		this->seriesInPlotWndProp.begin(), 
		this->seriesInPlotWndProp.end(), 
		[&maxSize](SeriesProp * series){
			LineTypeSeriesProp * lSeries = dynamic_cast<LineTypeSeriesProp *>(series);
			if (lSeries != 0)
			{
				size_t size = lSeries->DataSize();
				if (maxSize < size)
					maxSize = size;
			}
	});

	return maxSize;
}

double PlotWndPropLineType::GetDispMaxValue() {
	return ::TransformCoordinate(this->maxValue, isLog);
}

double PlotWndPropLineType::GetDispMinValue()
{
	return ::TransformCoordinate(this->minValue, isLog);
}

void PlotWndPropLineType::SetDispMaxValue(double value) {
	this->maxValue = min(::ReverseTransformCoordinate(value, isLog), INT_MAX);
}

void PlotWndPropLineType::SetDispMinValue(double value) {
	this->minValue = max(::ReverseTransformCoordinate(value, isLog), INT_MIN);
}

double PlotWndPropLineType::GetMaxValue() {
	return this->maxValue;
}

void PlotWndPropLineType::SetMaxValue(double value) {
	this->maxValue = min(value, INT_MAX);
}

double PlotWndPropLineType::GetMinValue() {
	return this->minValue;
}

void PlotWndPropLineType::SetMinValue(double value) {
	this->minValue = max(value, INT_MIN);
}
