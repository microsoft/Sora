#include "stdafx.h"

#include <memory>
#include "PlotWindowOpenedBitmapType.h"
#include "ChannelOpenedFixedLengthType.h"
#include "ChannelOpenedText.h"
#include "BitmapPlotWnd.h"
#include "HelperFunc.h"
#include "AppSettings.h"

using namespace std;

PlotWindowOpenedBitmapType::PlotWindowOpenedBitmapType(std::shared_ptr<ProcessOpened> process) :
	PlotWindowOpened(process)
{
	_plotWnd = 0;
	_isLog = false;
	_latestIdx = 0;
	_range = 1024;
	_bAutoScale = true;
	_seekPos = 0.0f;
	_playSpeed = 16.0;
	_playSpeedAccumulate = 0.0;
	_seekFlag = false;
	_bShowGrid = true;
}

PlotWindowOpenedBitmapType::~PlotWindowOpenedBitmapType()
{
	//if (_plotWnd)
	//	delete _plotWnd;
}

void PlotWindowOpenedBitmapType::PlayPauseToLatest(bool bPlay)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	this->DoLater([SThis, bPlay](){
		SThis->SetPlayPause(bPlay, true);
		if (SThis->_plotWnd)
		{
			auto bitmapPlotWnd = (BitmapPlotWnd *)SThis->_plotWnd;
			if (bPlay)
				bitmapPlotWnd->SetButtonStatePause();
			else
				bitmapPlotWnd->SetButtonStatePlay();
			
			bitmapPlotWnd->EnableSpeedButtons(false);
		}
	});
}

void PlotWindowOpenedBitmapType::SetPlayRange(size_t index, size_t range)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, index, range](){
		shared_me->_latestIdx = index;
		shared_me->_range = range;
		RangeChangedEvent e;
		e.Index = index;
		e.Range = range;
		shared_me->EventRangeChanged.Raise(shared_me.get(), e);
	});
}

void PlotWindowOpenedBitmapType::SetPlaySpeed(double playSpeed)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, playSpeed](){
		shared_me->_playSpeed = playSpeed;
		shared_me->EventSpeedChanged.Raise(shared_me.get(), shared_me->_playSpeed);
	});
}

void PlotWindowOpenedBitmapType::SetXLabel(const wstring & label)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, label](){
		shared_me->_xLabel = std::move(label);
		shared_me->EventXLabelChanged.Raise(shared_me.get(), shared_me->_xLabel);
	});
}

void PlotWindowOpenedBitmapType::SetYLabel(const wstring & label)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, label](){
		shared_me->_yLabel = std::move(label);
		shared_me->EventYLableChanged.Raise(shared_me.get(), shared_me->_yLabel);
	});
}

void PlotWindowOpenedBitmapType::SetShowGrid(bool bShow)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, bShow](){
		shared_me->_bShowGrid = bShow;
		shared_me->EventShowGrid.Raise(shared_me.get(), bShow);
	});	
}

void PlotWindowOpenedBitmapType::SetAutoScale(bool bAutoScale)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, bAutoScale](){
		shared_me->_bAutoScale = bAutoScale;
		shared_me->EventAutoScale.Raise(shared_me.get(), shared_me->_bAutoScale);
	});
}

void PlotWindowOpenedBitmapType::Seek(double pos)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, pos](){
		SThis->_seekPos = pos;
		SThis->_seekFlag = true;
	});
}

void PlotWindowOpenedBitmapType::SetClientRect(const CRect & rect)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, rect](){
		shared_me->_bitmapRect = rect;
	});
}

void PlotWindowOpenedBitmapType::ChangeSpeed(bool isUp)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, isUp](){
		if (isUp)
			shared_me->_playSpeed = min(shared_me->_playSpeed * 2, SettingGetMaxSpeed());
		else
			shared_me->_playSpeed = max((1.0/64), shared_me->_playSpeed / 2);
	});
}

void PlotWindowOpenedBitmapType::SetLog(bool isLog)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, isLog](){
		SThis->_isLog = isLog;
		SThis->OnCoordinateChanged();
	});	
}

void PlotWindowOpenedBitmapType::SetDataCount(size_t dataCount)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, dataCount](){
		SThis->_range = dataCount;
	});	
}

void PlotWindowOpenedBitmapType::ZoomInOut(bool isUp)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, isUp](){

		size_t range = SThis->_range;

		if (isUp)
		{
			range = (size_t)max(1, range / 1.1);
		}
		else
		{
			size_t newRange = (size_t)(range * 1.1);
			if (newRange == range)
				newRange++;

			range = (size_t)min(SettingGetReplayBufferSize()/sizeof(int), newRange);
		}

		SThis->ModifyDataRange(range);

		bool propertyChanged = SThis->_range != range;

		SThis->_range = range;
	});
}

void PlotWindowOpenedBitmapType::DrawBackground(Graphics * g, const CRect & clientRect)
{
	SolidBrush brush(Color(255, 0, 0, 0));
	g->FillRectangle(&brush, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
}

void PlotWindowOpenedBitmapType::DrawLabel(Graphics * g, const CRect & clientRectParam)
{
	CRect rect(0, 0, clientRectParam.Width(), clientRectParam.Height());

	bool bDrawYLabel = false;
	bool bDrawXLabel = false;

	CString strYLabel = _yLabel.c_str();
	if (strYLabel.GetLength() != 0)
	{
		bDrawYLabel = true;
	}

	CString strXLabel = this->_xLabel.c_str();
	if (strXLabel.GetLength() != 0)
	{
		bDrawXLabel = true;
	}

	if (bDrawYLabel)
	{
		CRect yLabelRect = rect;
		yLabelRect.top = rect.bottom;
		yLabelRect.bottom = yLabelRect.top + LABEL_HEIGHT;
		yLabelRect.left = bDrawXLabel ? rect.left + LABEL_HEIGHT : rect.left;
		yLabelRect.right = yLabelRect.left + (bDrawXLabel ? rect.Height() - LABEL_HEIGHT : rect.Height());
		Matrix matrixTransform;
		matrixTransform.RotateAt(-90.0f, PointF((Gdiplus::REAL)rect.left, (Gdiplus::REAL)rect.bottom));

		([&strYLabel, &yLabelRect, &g, &matrixTransform](){
			SolidBrush fontBrush(Color(255, 150, 150, 150));
			StringFormat format;
			format.SetFormatFlags(StringFormatFlagsNoWrap);
			format.SetTrimming(StringTrimmingEllipsisCharacter);
			Gdiplus::Font captionFont(L"Arial", 10);
			PointF pointF(5, 2);

			//format.SetFormatFlags(StringFormatFlagsDirectionVertical);

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

			g->SetTransform(&matrixTransform);
			g->DrawString(strYLabel, wcslen(strYLabel.GetBuffer()), &captionFont, rectMax, &format, &fontBrush);

			Color colorLine(50, 50, 50);
			SolidBrush brushLine(colorLine);
			Pen pen(&brushLine, 1.0f);
			g->DrawLine(&pen, yLabelRect.left, yLabelRect.bottom, yLabelRect.right, yLabelRect.bottom);

			Matrix matrixReset;
			g->SetTransform(&matrixReset);
		})();
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

			Color colorLine(50, 50, 50);
			SolidBrush brushLine(colorLine);
			Pen pen(&brushLine, 1.0f);
			g->DrawLine(&pen, xLabelRect.left, xLabelRect.top, xLabelRect.right, xLabelRect.top);	
		})();
	}
}

CPlotWnd * PlotWindowOpenedBitmapType::CreatePlotWnd()
{
	//if (_plotWnd)
	//	delete _plotWnd;

	BitmapPlotWnd * subPlotWnd = new BitmapPlotWnd(this);

	subPlotWnd->EventSeek.Subscribe([this](const void * sender, const BitmapPlotWnd::SeekEvent & e) {

		this->Seek(e.Pos);

	});

	subPlotWnd->EventPlayClicked.Subscribe([this](const void * sender, const BitmapPlotWnd::PlayPauseEvent & e) {
		this->SetPlayPause(e.IsPlay, false);
	});

	subPlotWnd->EventWheel.Subscribe([this](const void * sender, const BitmapPlotWnd::WheelEvent & e) {
		this->ZoomInOut(e.IsUp);
		auto SThis = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(this->shared_from_this());
		bool bIsUp = e.IsUp;
		this->DoLater([SThis, bIsUp](){
			SThis->OnMouseWheel(bIsUp);
		});
	});

	subPlotWnd->EventBitmapResize.Subscribe([this](const void * sender, const BitmapPlotWnd::ResizeEvent & e) {
		CRect rect;
		rect.left = 0;
		rect.right = rect.left + e.cx;
		rect.top = 0;
		rect.bottom = rect.bottom + e.cy;
		this->SetClientRect(rect);
	});

	subPlotWnd->EventSpeedClicked.Subscribe([this](const void * sender, const BitmapPlotWnd::SpeedChangeEvent & e){
		this->ChangeSpeed(e.IsUp);
	});

	return subPlotWnd;
}

void PlotWindowOpenedBitmapType::UpdateTrackBarRange()
{
	size_t maxSize = MaxDataSize();

	double right = 0.0;
	double range = 0.0;

	if (maxSize != 0)
	{
		right = (double)(maxSize - this->_latestIdx) / maxSize;
		range = (double)this->_range / maxSize;
	}

	auto plotwnd = (BitmapPlotWnd *)_plotWnd;
	plotwnd->SetTrackBarViewWndRange(right, range);
}

size_t PlotWindowOpenedBitmapType::MaxDataSize()
{
	//size_t maxSize = 0;
	//std::for_each(
	//	this->_chMap.begin(), 
	//	this->_chMap.end(), 
	//	[&maxSize](shared_ptr<ChannelOpened> channel){
	//		auto lChannel = dynamic_pointer_cast<ChannelOpenedFixedLengthType, ChannelOpened>(channel);
	//		if (lChannel)
	//		{
	//			size_t size = lChannel->DataSize();
	//			if (maxSize < size)
	//				maxSize = size;
	//		}
	//});

	//return maxSize;
	return 0;
}

void PlotWindowOpenedBitmapType::DrawXAxis(Graphics * g, const CRect & rectClient, size_t rangeDisp)
{

//	auto seriesProp = this->seriesInPlotWndProp[0];
//	auto process = seriesProp->GetProcess();
//
//	double maxX = (double)this->replayReadPos / process->replayCapacity;
//>replayCapacity;
//	double minX = (double)(this->replayReadPos - this->frameCount) / process->replayCapacity;


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
	strCount.Format(L"%d", rangeDisp);
	format.SetAlignment(StringAlignmentFar);
	RectF rectMax(
		(Gdiplus::REAL)(rectClient.Width()/2 + rectClient.left),
		(Gdiplus::REAL)rectClient.bottom-height,
		(Gdiplus::REAL)rectClient.Width()/2,
		(Gdiplus::REAL)height);
	g->DrawString(strCount, wcslen(strCount.GetBuffer()), &captionFont, rectMax, &format, &fontBrush);
}


void PlotWindowOpenedBitmapType::ModifyDataRange(size_t & range)
{
	// do nothing
}

CRect PlotWindowOpenedBitmapType::GetBitmapRect()
{
	CRect rect = _bitmapRect;
	CString strYLabel = _yLabel.c_str();
	if (strYLabel.GetLength() != 0)
	{
		rect.left += LABEL_HEIGHT;
	}

	CString strXLabel = this->_xLabel.c_str();
	if (strXLabel.GetLength() != 0)
	{
		rect.bottom -= LABEL_HEIGHT;
	}

	return rect;
}

void PlotWindowOpenedBitmapType::OnChannelAdded(std::shared_ptr<ChannelOpened> channel, const CRect & rect)
{
	auto textChannel = dynamic_pointer_cast<ChannelOpenedText, ChannelOpened>(channel);
	if (textChannel == 0)
		return;

	// add text channel sub plot window
	if (_plotWnd)
	{
		auto bitmapPlotWnd = dynamic_cast<BitmapPlotWnd *>(_plotWnd);
		assert(bitmapPlotWnd);
		auto subPlotWnd = textChannel->CreateSubPlotWnd();
		bitmapPlotWnd->AddSubPlotWndRelative(subPlotWnd, rect);
	}
}

void PlotWindowOpenedBitmapType::OnChannelClosed(std::shared_ptr<ChannelOpened> channel)
{
	auto textChannel = dynamic_pointer_cast<ChannelOpenedText, ChannelOpened>(channel);
	if (textChannel == 0)
		return;

	textChannel->CloseSubPlotWnd();
}

void PlotWindowOpenedBitmapType::SeekToLatest()
{
	this->_latestIdx = 0;
}

HRESULT PlotWindowOpenedBitmapType::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	PlotWindowOpened::AppendXmlProperty(pDom, pParent);

	auto SThis = dynamic_pointer_cast<PlotWindowOpenedBitmapType, AsyncObject>(shared_from_this());
	this->DoNow([SThis, pDom, pParent](){
		
		IXMLDOMElement *pElement = NULL;

		CreateAndAddElementNode(pDom, L"WndshowGrid", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_bShowGrid);	

		CreateAndAddElementNode(pDom, L"AutoScale", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_bAutoScale);	

		CreateAndAddElementNode(pDom, L"XLabel", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_xLabel.c_str());

		CreateAndAddElementNode(pDom, L"YLabel", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_yLabel.c_str());

		CreateAndAddElementNode(pDom, L"DataRange", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_range);

	});

	return S_OK;
}


HRESULT PlotWindowOpenedBitmapType::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	PlotWindowOpened::LoadXmlElement(pElement);

	CString cs;
	MSXML2::IXMLDOMNodePtr pNode;

	pNode = pElement->selectSingleNode(L"WndshowGrid");
	if (pNode != 0)
	{
		if(pNode->text == (_bstr_t)(L"0"))
			this->_bShowGrid = false;
		else
			this->_bShowGrid = true;
	}

	pNode = pElement->selectSingleNode(L"AutoScale");
	if (pNode != 0)
	{
		if (pNode->text == (_bstr_t)(L"0"))
			this->_bAutoScale = false;
		else
			this->_bAutoScale = true;
	}

	pNode = pElement->selectSingleNode(L"XLabel");
	if (pNode != 0)
	{
		cs.Format(_T("%S"),(LPCSTR)pNode->text);
		this->_xLabel = cs;
	}

	pNode = pElement->selectSingleNode(L"YLabel");
	if (pNode != 0)
	{
		cs.Format(_T("%S"),(LPCSTR)pNode->text);
		this->_yLabel = cs;
	}

	pNode = pElement->selectSingleNode(L"DataRange");
	if (pNode != 0)
	{
		this->_range = atoi((LPCSTR)(pNode->text));
	}

	return S_OK;
}


void PlotWindowOpenedBitmapType::OnCoordinateChanged()
{
	;
}

void PlotWindowOpenedBitmapType::OnProcessAttatched()
{
	this->_latestIdx = 0;
}

void PlotWindowOpenedBitmapType::OnMouseWheel(bool bIsUp)
{
	;
}
