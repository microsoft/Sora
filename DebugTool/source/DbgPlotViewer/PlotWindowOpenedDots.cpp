#include "stdafx.h"
#include <memory>
#include "PlotWindowOpenedDots.h"
#include "TaskSimple.h"
#include "TaskCoordinator.h"
#include "BitmapPlotWnd.h"
#include "HelperFunc.h"
#include "PlotWindowDotsProperty.h"
#include "HelperFunc.h"
#include "BitmapTypeSettings.h"
#include "SettingsPlotWindow.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotWindowOpenedDots::PlotWindowOpenedDots(std::shared_ptr<ProcessOpened> process) :
	PlotWindowOpenedBitmapType(process)
{
	_maxValue = 10.0;
	_luminescence = true;
	_operationUpdate = make_shared<UpdateOperation>();
}

PlotWindowOpenedDots::~PlotWindowOpenedDots()
{
	
}

bool PlotWindowOpenedDots::Accept(std::shared_ptr<ChannelOpened> channel, CPoint pointIn, CPoint & pointOut)
{
	bool bAccepted = false;
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedDots, AsyncObject>(shared_from_this());

	TaskQueue()->DoTask([&bAccepted, SThis, channel, &pointIn, &pointOut](){

		if (SThis->_processOpened == 0)
		{
			bAccepted = false;
			return;
		}

		if (SThis->_processOpened->Pid() != channel->Pid())
		{
			bAccepted = false;
			return;
		}

		if (SThis->_chMap.size() == 0)
		{
			bAccepted = false;
			return;
		}

		for (auto iterCh = SThis->_chMap.begin(); iterCh != SThis->_chMap.end(); ++iterCh)
		{
			if (*iterCh == channel)
			{
				bAccepted = false;
				return;
			}
		}

		if (dynamic_pointer_cast<ChannelOpenedDots, ChannelOpened>(channel) == 0 &&
			dynamic_pointer_cast<ChannelOpenedText, ChannelOpened>(channel) == 0 )
		{
			bAccepted = false;
			return;
		}

		if (SThis->_plotWnd == 0)
		{
			bAccepted = false;
			return;
		}

		pointOut = pointIn;
		auto bitmapPlotWnd = dynamic_cast<BitmapPlotWnd *>(SThis->_plotWnd);
		assert(bitmapPlotWnd);
		bitmapPlotWnd->MyScreenToClient(pointOut);

		bAccepted = true;
	});

	return bAccepted;
}

void PlotWindowOpenedDots::SetMaxValue(double maxValue)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedDots, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, maxValue](){
		shared_me->_maxValue = min(maxValue, ::SettingGetBitmapRangeMax(false));
	});
}

void PlotWindowOpenedDots::SetLuminescence(bool bLuminescence)
{
	auto shared_me = dynamic_pointer_cast<PlotWindowOpenedDots, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([shared_me, bLuminescence](){
		shared_me->_luminescence = bLuminescence;
	});
}

std::shared_ptr<SoraDbgPlot::Task::TaskSimple> PlotWindowOpenedDots::TaskUpdate()
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedDots, AsyncObject>(shared_from_this());

	TaskQueue()->DoTask([SThis](){
		SThis->_operationUpdate->_vecChDots.clear();
		SThis->_operationUpdate->_vecChText.clear();
		SThis->_operationUpdate->_vecChUpdate.clear();
		
		bool bUpdateChannel = SThis->_isPlay && (SThis->_latestIdx == 0);

		for (auto iter = SThis->_chMap.begin(); iter != SThis->_chMap.end(); ++iter)
		{
			auto channel = *(iter);
			auto channelDots = dynamic_pointer_cast<ChannelOpenedDots, ChannelOpened>(channel);
			if (channelDots)
				SThis->_operationUpdate->_vecChDots.push_back(channelDots);
			auto channelText = dynamic_pointer_cast<ChannelOpenedText, ChannelOpened>(channel);
			if (channelText)
				SThis->_operationUpdate->_vecChText.push_back(channelText);

			if (bUpdateChannel || (SThis->_newChSet.find(channel->Id()) != SThis->_newChSet.end()))
			{
				SThis->_operationUpdate->_vecChUpdate.push_back(channel);
			}
		}

		SThis->_newChSet.clear();

	});

	auto taskStart = make_shared<TaskSimple>(TaskQueue(), [SThis](){
		//TRACE1("task start %x\n", SThis.get());

		if (SThis->_isPlay)
		{
			SThis->_playSpeedAccumulate += SThis->_playSpeed;
			int speedAccumulate = (int)SThis->_playSpeedAccumulate;
			if (SThis->_latestIdx > (size_t)speedAccumulate)
				SThis->_latestIdx -= (size_t)speedAccumulate;
			else
				SThis->_latestIdx = 0;
			SThis->_playSpeedAccumulate -= speedAccumulate;
		}

		SThis->_operationUpdate->bmp = new Bitmap(SThis->_bitmapRect.Width(), SThis->_bitmapRect.Height());
		SThis->_operationUpdate->param->_g = Graphics::FromImage(SThis->_operationUpdate->bmp);
		//SThis->_operationUpdate->param->_g->SetSmoothingMode(SmoothingModeAntiAlias);
		SThis->_operationUpdate->param->_bIsLog = SThis->_isLog;
		SThis->_operationUpdate->param->_bMaxInitialized = false;
		SThis->_operationUpdate->param->_rect = SThis->GetBitmapRect();
		SThis->_operationUpdate->param->_index = SThis->_latestIdx;
		SThis->_operationUpdate->param->_range = SThis->_range;
		SThis->_operationUpdate->param->_luminescence = SThis->_luminescence;
		SThis->_operationUpdate->param->_timeStamp = 0;
		SThis->_operationUpdate->param->_bTimeStampTaken = false;
		*(SThis->_operationUpdate->dataSize) = 0;
		SThis->_operationUpdate->_applyAutoScale =  SThis->_bAutoScale;

	});

	auto taskUpdateChannel = make_shared<TaskCoordinator>(TaskQueue());
	for (auto iterChannel = _operationUpdate->_vecChUpdate.begin(); iterChannel != _operationUpdate->_vecChUpdate.end(); ++iterChannel)
	{
		taskUpdateChannel->AddTask((*iterChannel)->TaskUpdateData(std::shared_ptr<bool>()));
	}

	//auto taskProcesslineGet = make_shared<TaskCoordinator>(TaskQueue());
	for (auto iterChannel = _operationUpdate->_vecChDots.begin(); iterChannel != _operationUpdate->_vecChDots.end(); ++iterChannel)
	{
		taskUpdateChannel->ContinueWith((*iterChannel)->TaskUpdateDataSize(this->_operationUpdate->dataSize));
	}

	auto taskCalcInfo = make_shared<TaskSimple>(TaskQueue(), [SThis](){
		if (SThis->_seekFlag)
		{
			SThis->_seekFlag = false;
			double seekPos = SThis->_seekPos;
			size_t dataSize = *(SThis->_operationUpdate->dataSize);
			size_t latestIndex = (size_t) ((dataSize) * (1.0 - seekPos));
			SThis->_operationUpdate->param->_index = latestIndex;
			SThis->_latestIdx = latestIndex;
		}
		else
		{
			;	// do nothing
		}
	});

	//auto taskProcesslineGet = make_shared<TaskCoordinator>(TaskQueue());
	for (auto iterChannel = _operationUpdate->_vecChDots.begin(); iterChannel != _operationUpdate->_vecChDots.end(); ++iterChannel)
	{
		taskCalcInfo->ContinueWith((*iterChannel)->TaskProcessData(this->_operationUpdate->param, false));
	}

	auto taskModifyParam = make_shared<TaskSimple>(TaskQueue(), [SThis](){
		double max = SThis->_operationUpdate->param->_dispMaxValue;
		bool bMaxValid = SThis->_operationUpdate->param->_bMaxInitialized;
		if (SThis->_operationUpdate->_applyAutoScale)
		{
			if (bMaxValid)
			{
				max = max(max, 2.0);
				double maxHigh = max * 11 / 4;
				double maxDest = max * 3 / 2;
				double maxLow = max * 9 / 8;

				if ( (SThis->_maxValue < maxLow) || (SThis->_maxValue > maxHigh))
					SThis->_maxValue = maxDest;
			}
		}
		else
		{}

		SThis->_operationUpdate->param->_dispMaxValue = SThis->_maxValue;
	});

	auto taskDraw = make_shared<TaskSimple>(TaskQueue(), [SThis](){
		Gdiplus::Graphics * g = SThis->_operationUpdate->param->_g;
		CRect rect = SThis->_operationUpdate->param->_rect;
		SThis->DrawBackground(g, SThis->_bitmapRect);
		SThis->DrawLabel(g, SThis->_bitmapRect);
		
		if (SThis->_operationUpdate->param->_bMaxInitialized)
		{
			double max = SThis->_operationUpdate->param->_dispMaxValue;

			if (SThis->_bShowGrid)
			{
				SThis->DrawGrid(g, rect, SThis->_operationUpdate->param->_dispMaxValue, SThis->_operationUpdate->param->_range);
			}

			if (SThis->_operationUpdate->_applyAutoScale)
				SThis->UpdatePropertyMax(SThis->_maxValue);
		}

		if (SThis->_plotWnd)
		{
			auto plotWndBitmap = (BitmapPlotWnd *)SThis->_plotWnd;
			if (SThis->_isPlay != SThis->_isPlayLastState)
			{
				if (SThis->_isPlay)
					plotWndBitmap->SetButtonStatePause();
				else
					plotWndBitmap->SetButtonStatePlay();

				SThis->_isPlayLastState = SThis->_isPlay;
			}

			plotWndBitmap->EnableSpeedButtons(SThis->_operationUpdate->param->_index != 0);

			size_t totalSize = *(SThis->_operationUpdate->dataSize);
			size_t latestIndex = SThis->_operationUpdate->param->_index;
			size_t range = SThis->_operationUpdate->param->_range;
			
			if (totalSize > 0)
			{
				double rightF = (double)(totalSize - latestIndex) / totalSize;
				double rangeF = (double)range / totalSize;
				plotWndBitmap->SetTrackBarViewWndRange(rightF, rangeF);
			}

			for (auto iterTxt = SThis->_operationUpdate->_vecChText.begin(); iterTxt != SThis->_operationUpdate->_vecChText.end(); ++iterTxt)
			{
				if (SThis->_operationUpdate->param->_bTimeStampTaken)
					(*iterTxt)->SeekTimeStamp(SThis->_operationUpdate->param->_timeStamp);
				(*iterTxt)->UpdateSubPlotWnd();
			}
		}
	});

	for (auto iterChannel = _operationUpdate->_vecChDots.begin(); iterChannel != _operationUpdate->_vecChDots.end(); ++iterChannel)
	{
		taskDraw->ContinueWith((*iterChannel)->TaskProcessData(this->_operationUpdate->param, true));
	}

	auto taskFinal = make_shared<TaskSimple>(TaskQueue(), [SThis](){
		if (SThis->_plotWnd)
		{
			auto plotWnd = (BitmapPlotWnd *)SThis->_plotWnd;
			if (plotWnd)
				plotWnd->SetBitmap(SThis->_operationUpdate->bmp);
			else
			{
				delete SThis->_operationUpdate->bmp;
			}

			SThis->_operationUpdate->bmp = 0;
			delete SThis->_operationUpdate->param->_g;
			SThis->_operationUpdate->param->_g = 0;
		}
		//TRACE0("task final\n");
	}, [SThis](){
		if (SThis->_operationUpdate->bmp) {
			delete SThis->_operationUpdate->bmp;
			SThis->_operationUpdate->bmp = 0;
		}
		if (SThis->_operationUpdate->param->_g)
		{
			delete SThis->_operationUpdate->param->_g;
			SThis->_operationUpdate->param->_g = 0;
		}
	});

	taskStart
		->ContinueWith(taskUpdateChannel)
		->ContinueWith(taskCalcInfo)
		->ContinueWith(taskModifyParam)
		->ContinueWith(taskDraw)
		->ContinueWith(taskFinal);

	return taskStart;
}

void PlotWindowOpenedDots::DrawGrid(Graphics * g, const CRect & clientRect, double max, size_t range)
{
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
		xleft, 
		ytop, 
		clientRect.Width() - 1, 
		clientRect.Height() - 1);

	// innner ellipse

	if ( min(clientRect.Width(), clientRect.Height() ) > 100 )
	{
		g->DrawEllipse(
			&pen,
			clientRect.Width() / 4 + xleft,
			clientRect.Height() / 4 + ytop,
			clientRect.Width() / 2 - 1,
			clientRect.Height() / 2 - 1);

		// draw text
		SolidBrush fontBrush(Color(255, 150, 150, 150));
		StringFormat format;
		format.SetAlignment(StringAlignmentNear);
		CString strValue;
		::FormatCommaString(strValue, max, max/100.0);
		CString strDisp;
		strDisp.Format(L"r = %s", strValue);
		Gdiplus::Font gridFont(L"Arial", 10);
		PointF pointF((Gdiplus::REAL)(xleft + 5), (Gdiplus::REAL)(ybottom - 16));
		g->DrawString(strDisp, -1, &gridFont, pointF, &format, &fontBrush);
	}

	this->DrawXAxis(g, clientRect, range);
}

shared_ptr<BaseProperty> PlotWindowOpenedDots::CreatePropertyPage()
{
	shared_ptr<PlotWindowDotsProperty> propertyPage;
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedDots, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, &propertyPage](){

		propertyPage = shared_ptr<PlotWindowDotsProperty>(new PlotWindowDotsProperty(
			SThis->_name,
			SThis->_xLabel,
			SThis->_yLabel,
			SThis->_bAutoScale,
			SThis->_maxValue,
			SThis->_range,
			SThis->_bShowGrid,
			SThis->_isLog,
			SThis->_luminescence
			));

		auto SThis2 = SThis;
		propertyPage->EventName.Subscribe([SThis2](const void * sender, const wstring & name){
			SThis2->SetName(name);
		});

		propertyPage->EventXLabel.Subscribe([SThis2](const void * sender, const wstring & xLabel){
			SThis2->SetXLabel(xLabel);
		});

		propertyPage->EventYLabel.Subscribe([SThis2](const void * sender, const wstring & yLabel){
			SThis2->SetYLabel(yLabel);
		});

		propertyPage->EventAutoScale.Subscribe([SThis2](const void * sender, const bool & e){
			SThis2->SetAutoScale(e);
		});

		propertyPage->EventMaxValue.Subscribe([SThis2](const void * sender, const double & e){
			SThis2->SetMaxValue(e);
		});

		propertyPage->EventShowGrid.Subscribe([SThis2](const void * sender, const bool & e){
			SThis2->SetShowGrid(e);
		});

		propertyPage->EventLog.Subscribe([SThis2](const void * sender, const bool & e){
			SThis2->SetLog(e);
		});

		propertyPage->EventLuminescence.Subscribe([SThis2](const void * sender, const bool & e){
			SThis2->SetLuminescence(e);
		});

		propertyPage->EventDataCount.Subscribe([SThis2](const void * sender, const size_t & e){
			SThis2->SetDataCount(e);
		});

	});

	return propertyPage;
}


void PlotWindowOpenedDots::ModifyDataRange(size_t & range)
{
	range = min(range, ::SettingDotMaxDataCount());
}

void PlotWindowOpenedDots::UpdatePropertyMax(double max)
{
	if (_propertyPage)
	{
		auto propertyPageLineType = dynamic_pointer_cast<PlotWindowDotsProperty, BaseProperty>(_propertyPage);
		propertyPageLineType->UpdateMax(max);
	}
}

void PlotWindowOpenedDots::UpdatePropertyDataRange(size_t dataCount)
{
	if (_propertyPage)
	{
		auto propertyPageLineType = dynamic_pointer_cast<PlotWindowDotsProperty, BaseProperty>(_propertyPage);
		propertyPageLineType->UpdateDataRange(dataCount);
	}
}


HRESULT PlotWindowOpenedDots::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	PlotWindowOpenedBitmapType::AppendXmlProperty(pDom, pParent);

	auto SThis = dynamic_pointer_cast<PlotWindowOpenedDots, AsyncObject>(shared_from_this());
	this->DoNow([SThis, pDom, pParent](){
		
		IXMLDOMElement *pElement = NULL;

		CreateAndAddElementNode(pDom, L"WndmaxValue", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_maxValue);

		CreateAndAddElementNode(pDom, L"Wndluminescence", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_luminescence);
	});

	return S_OK;
}

HRESULT PlotWindowOpenedDots::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	PlotWindowOpenedBitmapType::LoadXmlElement(pElement);

	CString cs;
	MSXML2::IXMLDOMNodePtr pNode;

	pNode = pElement->selectSingleNode(L"WndmaxValue");
	if (pNode != 0)
	{
		this->_maxValue = atof((LPCSTR)(pNode->text));
	}

	pNode = pElement->selectSingleNode(L"Wndluminescence");
	if (pNode != 0)
	{
		if (pNode->text == (_bstr_t)(L"0"))
			this->_luminescence = false;
		else
			this->_luminescence = true;
	}

	return S_OK;
}

void PlotWindowOpenedDots::OnMouseWheel(bool bIsUp)
{
	if (_propertyPage)
	{
		auto propertyPageLineType = dynamic_pointer_cast<PlotWindowDotsProperty, BaseProperty>(_propertyPage);
		propertyPageLineType->UpdateDataRange(_range);
	}	
}
