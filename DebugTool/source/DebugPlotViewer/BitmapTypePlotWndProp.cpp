#include "stdafx.h"

#include "BitmapTypePlotWndProp.h"
//#include "DebugPlotViewerDoc.h"
#include "SeriesBitmapType.h"
#include "BitmapTypePlotWndProp.h"
#include "BitmapPlotWnd.h"
#include "SubBitmapPlotWnd.h"
#include "HelperFunc.h"
#include "BaseProperty.h"
#include "LinePlotWndProperty.h"
#include "AppMessage.h"
#include "SubPlotWnd.h"

/***********************************

Line Plot Wnd

************************************/
BitmapTypePlotWndProp::BitmapTypePlotWndProp()
{
	this->_latestIdx = 0;
	this->_playSpeed = 16;
	this->_rangeSize = 4024;
	this->_isPlaying = true;
	this->_subPlotWnd = 0;

	this->EventNameChanged.Subscribe([=](const void * sender, const CString & name){
		if (_subPlotWnd)
			_subPlotWnd->Invalidate();
	});
}

BitmapTypePlotWndProp::~BitmapTypePlotWndProp()
{
	//if (this->_subPlotWnd)
	//	delete this->_subPlotWnd;
}

void BitmapTypePlotWndProp::SeekTextWnd()
{
	vector<unsigned long long> vecTimeStamp;

	for_each(this->seriesInPlotWndProp.begin(), this->seriesInPlotWndProp.end(), [this, &vecTimeStamp](SeriesProp * series){
		BitmapTypeSeriesProp * bitmapSeries = dynamic_cast<BitmapTypeSeriesProp *>(series);
		if (bitmapSeries)
		{
			unsigned long long timestamp;
			bool succ = bitmapSeries->GetTimeStamp(this->_latestIdx, timestamp);
			if (succ)
				vecTimeStamp.push_back(timestamp);
		}
	});

	for_each(this->seriesInPlotWndProp.begin(), this->seriesInPlotWndProp.end(), [&vecTimeStamp](SeriesProp * series){
		TextSeriesProp * textSeries = dynamic_cast<TextSeriesProp *>(series);
		if (textSeries)
		{
			textSeries->LockData();
			textSeries->SeekTimeStamp(vecTimeStamp);
			textSeries->UnlockData();
		}
	});
}

size_t BitmapTypePlotWndProp::LatestIdx()
{
	return _latestIdx;
}

CPlotWnd * BitmapTypePlotWndProp::CreatePlotWnd(CWnd * parent)
{
	if (_subPlotWnd)
		delete _subPlotWnd;

	_subPlotWnd = new BitmapPlotWnd;

	_subPlotWnd->EventSeek.Subscribe([this](const void * sender, const BitmapPlotWnd::SeekEvent & e) {

		size_t maxSize = this->MaxDataSize();

		this->_latestIdx = (size_t)(maxSize * (1.0 - e.Pos));

		this->UpdateTrackBarRange();

		this->SeekTextWnd();

		if (this->_latestIdx != 0)
			this->DisableDataUpdate();
	});

	_subPlotWnd->EventPlayClicked.Subscribe([this](const void * sender, const BitmapPlotWnd::PlayPauseEvent & e) {

		if (e.IsPlay)
		{
			this->_isPlaying = true;
			this->_subPlotWnd->SetButtonStatePause();
		}
		else
		{
			this->_isPlaying = false;
			this->_subPlotWnd->SetButtonStatePlay();
			this->DisableDataUpdate();
		}
	});

	_subPlotWnd->EventWheel.Subscribe([this](const void * sender, const BitmapPlotWnd::WheelEvent & e) {
		this->ZoomInOut(e.IsUp);
	});

	_subPlotWnd->EventResize.Subscribe([this](const void * sender, const BitmapPlotWnd::ResizeEvent & e) {
		this->_clientWidth = e.cx;
		this->_clientHeight = e.cy;
		this->ForceUpdateGraph();
	});

	_subPlotWnd->EventAddTarget.Subscribe([this](const void * sender, const CPlotWnd::AddTargetEvent & e) {
		auto series = (SeriesProp *)e.obj;
		
		
		this->AddSeries(series);
		auto textSeries = dynamic_cast<TextSeriesProp *>(series);
		if (textSeries)
		{
			auto subWnd = textSeries->CreateSubPlotWnd();
			//subWnd->plotWndProp = this;
			this->_subPlotWnd->AddSubPlotWnd(subWnd, textSeries->rect, textSeries);
		}

		series->isOpened = true;
		series->GetProcess()->openCount++;
		::AfxGetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_CONTROLLER, 0);
	});

	_subPlotWnd->EventSpeedClicked.Subscribe([this](const void * sender, const BitmapPlotWnd::SpeedChangeEvent & e){
		if (e.IsUp)
		{
			this->_playSpeed = min(this->_playSpeed * 2, 1*1024*1024);
		}
		else
		{
			this->_playSpeed = max(1, this->_playSpeed / 2);
		}
	});

	return _subPlotWnd;
}

BaseProperty * BitmapTypePlotWndProp::GetPropertyPage()
{
	BaseProperty * property = new LinePlotWndProperty;
	property->SetTarget(this);
	return property;
}

void BitmapTypePlotWndProp::PlayAFrame() {

	if (!this->_isPlaying)
		return;

	if (_latestIdx > _playSpeed)
		_latestIdx -= _playSpeed;
	else
	{
		_latestIdx = 0;
		this->EnableDataUpdate();
	}

	this->UpdateTrackBarRange();
}

void BitmapTypePlotWndProp::UpdateTrackBarRange()
{
	size_t maxSize = MaxDataSize();

	double right = 0.0;
	double range = 0.0;

	if (maxSize != 0)
	{
		right = (double)(maxSize - this->_latestIdx) / maxSize;
		range = (double)this->_rangeSize / maxSize;
	}

	_subPlotWnd->SetTrackBarViewWndRange(right, range);
}

void BitmapTypePlotWndProp::SetDataRange(size_t range)
{
	_rangeSize = range;
}

size_t BitmapTypePlotWndProp::GetRangeSize()
{
	return _rangeSize;
}

void BitmapTypePlotWndProp::ZoomInOut(bool isIn)
{
	size_t range = _rangeSize;
	
	if (isIn)
	{
		range = (size_t)max(1, range / 1.1);
	}
	else
	{
		size_t newRange = (size_t)(range * 1.1);
		if (newRange == range)
			newRange++;

		range = (size_t)min(1*1024*1024, newRange);
	}

	ModifyDataRange(range);

	bool propertyChanged = _rangeSize != range;

	_rangeSize = range;

	if (propertyChanged)
	{
		BaseProperty * property = this->GetPropertyPage();
		::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);
		this->ForceUpdateGraph();
		this->UpdateTrackBarRange();
	}
}


void BitmapTypePlotWndProp::ModifyDataRange(size_t & oldRange)
{
	// default: do nothing
}

int BitmapTypePlotWndProp::ClientWidth()
{
	return _clientWidth;
}

int BitmapTypePlotWndProp::ClientHeight()
{
	return _clientHeight;
}


size_t BitmapTypePlotWndProp::RangeSize()
{
	return _rangeSize;
}

void BitmapTypePlotWndProp::ModifyTrackBarDispRange(double & range)
{
	// default do nothing
}

void BitmapTypePlotWndProp::UpdatePlotWnd()
{
	::WaitForSingleObject(this->hMutexTree, INFINITE);

	if (this->IsDataUpdateEnabled())
	{
		for_each(this->seriesInPlotWndProp.begin(), this->seriesInPlotWndProp.end(), [](SeriesProp * series){
			series->UpdateFromDataQueue();
		});
	}

	Bitmap * bmp = 0;

	this->PlayAFrame();
	this->SeekTextWnd();
	bmp = this->CreateBitmap();

	for_each(this->seriesInPlotWndProp.begin(), this->seriesInPlotWndProp.end(), [](SeriesProp * series){
		TextSeriesProp * textSeries = dynamic_cast<TextSeriesProp *>(series);
		if (textSeries)
		{
			textSeries->UpdateSubWnd();
		}
	});

	::ReleaseMutex(this->hMutexTree);

	if (bmp)
	{
		CPlotWnd * plotWnd = (CPlotWnd *)this->targetWnd;

		BitmapPlotWnd * bitmapPlotWnd = dynamic_cast<BitmapPlotWnd *>(plotWnd);
		if (bitmapPlotWnd)
		{
			plotWnd->Invoke([bmp, bitmapPlotWnd](){
				bitmapPlotWnd->SetBitmap(bmp);
				bitmapPlotWnd->UpdateView();
			});
		}
	}
}

void BitmapTypePlotWndProp::DrawXAxis(Graphics * g, const CRect & rectClient)
{
	// draw x-axis

	if ( this->seriesInPlotWndProp.size() == 0 )
		return;

	SeriesProp * seriesProp = this->seriesInPlotWndProp[0];
	DebuggingProcessProp * process = seriesProp->GetProcess();

	//double maxX = (double)process->replayReadPos / process->replayCapacity;
	double maxX = (double)this->replayReadPos / process->replayCapacity;
	//double minX = (double)(process->replayReadPos - plotWndProp->frameCount) / process->replayCapacity;
	double minX = (double)(this->replayReadPos - this->frameCount) / process->replayCapacity;


	SolidBrush fontBrush(Color(255, 150, 150, 150));
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	format.SetFormatFlags(StringFormatFlagsNoWrap);
	format.SetTrimming(StringTrimmingEllipsisCharacter);
	Gdiplus::Font captionFont(L"Arial", 10);
	PointF pointF(5, 2);

	//{
	//	format.SetFormatFlags(StringFormatFlagsDirectionVertical);
	//}

	RectF rectBound(0, 0, 500, 500);
	RectF rectMeasure;
	g->MeasureString(L"0", -1, &captionFont, rectBound, &rectMeasure);
	float height = rectMeasure.Height;

	CString strCount;
	strCount.Format(L"%d", this->GetRangeSize());
	format.SetAlignment(StringAlignmentFar);
	RectF rectMax(
		(Gdiplus::REAL)(rectClient.Width()/2 + rectClient.left),
		(Gdiplus::REAL)rectClient.bottom-height,
		(Gdiplus::REAL)rectClient.Width()/2,
		(Gdiplus::REAL)height);
	g->DrawString(strCount, wcslen(strCount.GetBuffer()), &captionFont, rectMax, &format, &fontBrush);
}

void BitmapTypePlotWndProp::ModifySeekedPos(size_t & idx)
{
	return;
}

void BitmapTypePlotWndProp::AddTextPlotWnd(TextSeriesProp * series)
{
	auto subWnd = series->CreateSubPlotWnd();
	this->_subPlotWnd->AddSubPlotWndRelative(subWnd, series->rect, series);
}


void BitmapTypePlotWndProp::XLabel(const CString & label)
{
	_lockYLabel.Lock();
	_xLabel = label;
	_lockYLabel.Unlock();
}

CString BitmapTypePlotWndProp::GetXLabel()
{
	_lockYLabel.Lock();
	CString str = _xLabel;
	_lockYLabel.Unlock();

	return str;
}

void BitmapTypePlotWndProp::YLabel(const CString & label)
{
	_lockYLabel.Lock();
	_yLabel = label;
	_lockYLabel.Unlock();
}

CString BitmapTypePlotWndProp::GetYLabel()
{
	_lockYLabel.Lock();
	CString str = _yLabel;
	_lockYLabel.Unlock();

	return str;
}

void BitmapTypePlotWndProp::GetCanvasRect(CRect & out)
{
	CRect clientRect(0, 0, this->ClientWidth(), this->ClientHeight());
	CString strYLabel = this->GetYLabel();
	if (strYLabel.GetLength() != 0)
	{
		clientRect.left += LABEL_HEIGHT;
	}

	CString strXLabel = this->GetXLabel();
	if (strXLabel.GetLength() != 0)
	{
		clientRect.bottom -= LABEL_HEIGHT;
	}

	out = clientRect;
}

void BitmapTypePlotWndProp::DrawLabel(Graphics * g)
{
	CRect rect(0, 0, this->ClientWidth(), this->ClientHeight());

	bool bDrawYLabel = false;
	bool bDrawXLabel = false;
	CRect clientRect = rect;

	CString strYLabel = this->GetYLabel();
	if (strYLabel.GetLength() != 0)
	{
		bDrawYLabel = true;
		clientRect.left += LABEL_HEIGHT;
	}

	CString strXLabel = this->GetXLabel();
	if (strXLabel.GetLength() != 0)
	{
		bDrawXLabel = true;
		clientRect.bottom -= LABEL_HEIGHT;
	}

	if (bDrawYLabel)
	{
		CRect yLabelRect = rect;
		yLabelRect.right = yLabelRect.left + LABEL_HEIGHT;
		if (bDrawXLabel)
		{
			yLabelRect.bottom -= LABEL_HEIGHT;
		}

		([&strYLabel, &yLabelRect, &g](){
			SolidBrush fontBrush(Color(255, 150, 150, 150));
			StringFormat format;
			format.SetAlignment(StringAlignmentNear);
			format.SetFormatFlags(StringFormatFlagsNoWrap);
			format.SetTrimming(StringTrimmingEllipsisCharacter);
			Gdiplus::Font captionFont(L"Arial", 10);
			PointF pointF(5, 2);

			format.SetFormatFlags(StringFormatFlagsDirectionVertical);

			RectF rectBound(0, 0, 500, 500);
			RectF rectMeasure;
			g->MeasureString(L"0", -1, &captionFont, rectBound, &rectMeasure);
			float height = rectMeasure.Height;

			format.SetAlignment(StringAlignmentCenter);

			RectF rectMax(
				(Gdiplus::REAL)yLabelRect.left,
				(Gdiplus::REAL)yLabelRect.top,
				(Gdiplus::REAL)yLabelRect.Width(),
				(Gdiplus::REAL)yLabelRect.Height()
				);

			g->DrawString(strYLabel, wcslen(strYLabel.GetBuffer()), &captionFont, rectMax, &format, &fontBrush);
		})();

		Color colorLine(50, 50, 50);
		SolidBrush brushLine(colorLine);
		Pen pen(&brushLine, 1.0f);
		g->DrawLine(&pen, yLabelRect.right, yLabelRect.top, yLabelRect.right, yLabelRect.bottom);
	}

	if (bDrawXLabel)
	{
		CRect xLabelRect = rect;
		xLabelRect.top = xLabelRect.bottom - LABEL_HEIGHT;
		if (bDrawYLabel)
		{
			xLabelRect.left += LABEL_HEIGHT;
		}

		([&strXLabel, &xLabelRect, &g](){
			SolidBrush fontBrush(Color(255, 150, 150, 150));
			StringFormat format;
			format.SetAlignment(StringAlignmentNear);
			format.SetFormatFlags(StringFormatFlagsNoWrap);
			format.SetTrimming(StringTrimmingEllipsisCharacter);
			Gdiplus::Font captionFont(L"Arial", 10);
			PointF pointF(5, 2);

			RectF rectBound(0, 0, 500, 500);
			RectF rectMeasure;
			g->MeasureString(L"0", -1, &captionFont, rectBound, &rectMeasure);
			float height = rectMeasure.Height;

			format.SetAlignment(StringAlignmentCenter);

			RectF rectMax(
				(Gdiplus::REAL)xLabelRect.left,
				(Gdiplus::REAL)xLabelRect.top,
				(Gdiplus::REAL)xLabelRect.Width(),
				(Gdiplus::REAL)xLabelRect.Height()
				);
			g->DrawString(strXLabel, wcslen(strXLabel.GetBuffer()), &captionFont, rectMax, &format, &fontBrush);
		})();

		Color colorLine(50, 50, 50);
		SolidBrush brushLine(colorLine);
		Pen pen(&brushLine, 1.0f);
		g->DrawLine(&pen, xLabelRect.left, xLabelRect.top, xLabelRect.right, xLabelRect.top);		
	}
}

void BitmapTypePlotWndProp::PlayPauseProcess(bool bPlay)
{
	if (bPlay)
	{
		this->_isPlaying = true;
		this->_subPlotWnd->SetButtonStatePause();
	}
	else
	{
		this->_isPlaying = false;
		this->_subPlotWnd->SetButtonStatePlay();
		this->DisableDataUpdate();
	}
}
