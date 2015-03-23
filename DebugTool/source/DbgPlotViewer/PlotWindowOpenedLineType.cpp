#include "stdafx.h"
#include "ChannelOpenedLineType.h"
#include "ChannelOpenedText.h"
#include "PlotWindowOpenedLineType.h"
#include "PlotWindowOpenedSpectrum.h"
#include "HelperFunc.h"
#include "TaskSimple.h"
#include "TaskCoordinator.h"
#include "BitmapPlotWnd.h"
#include "PlotWindowLineTypeProperty.h"
#include "GridAlg.h"
#include "BitmapTypeSettings.h"

using namespace std;
using namespace SoraDbgPlot::Task;

struct SnapshotInfo
{
	struct ChannelInfo
	{
		double _maxValue;
		double _minValue;
		shared_ptr<ChannelOpened> _ch;
		vector<int> points;
	};
	
	Graphics * g;
	CRect _rect;
	double _maxValue;
	double _minValue;
	list<shared_ptr<ChannelInfo> > _chList;
	list<shared_ptr<ChannelInfo> >::iterator chIter;
};

PlotWindowOpenedLineType::PlotWindowOpenedLineType(std::shared_ptr<ProcessOpened> process) :
	PlotWindowOpenedBitmapType(process)
{
	_maxValue = 10.0;
	_minValue = -10.0;
	_maxValueAfterAutoScale = 10.0;
	_minValueAfterAutoScale = -10.0;
	_operationUpdate = make_shared<UpdateOperation>();
}

std::shared_ptr<BaseProperty> PlotWindowOpenedLineType::CreatePropertyPage()
{
	shared_ptr<PlotWindowLineTypeProperty> propertyPage;
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedLineType, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, &propertyPage](){
		propertyPage = shared_ptr<PlotWindowLineTypeProperty>(new PlotWindowLineTypeProperty(SThis->GetTypeName(),
			SThis->_name,
			SThis->_xLabel,
			SThis->_yLabel,
			SThis->_bAutoScale,
			SThis->_maxValue,
			SThis->_minValue,
			SThis->_bShowGrid,
			SThis->_isLog,
			SThis->IsRangeSettingEnabled(),
			SThis->_range
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

		propertyPage->EventMinValue.Subscribe([SThis2](const void * sender, const double & e){
			SThis2->SetMinValue(e);
		});

		propertyPage->EventShowGrid.Subscribe([SThis2](const void * sender, const bool & e){
			SThis2->SetShowGrid(e);
		});

		propertyPage->EventLog.Subscribe([SThis2](const void * sender, const bool & e){
			SThis2->SetLog(e);
		});

		propertyPage->EventDataCount.Subscribe([SThis2](const void * sender, const size_t & e){
			size_t dataRange = e;
			SThis2->ModifyDataRange(dataRange);
			SThis2->SetDataCount(dataRange);
		});

	});

	return propertyPage;
}

void PlotWindowOpenedLineType::SetMaxValue(double maxValue)
{
	auto me = 
		dynamic_pointer_cast<PlotWindowOpenedLineType, AsyncObject>
		(shared_from_this());

	TaskQueue()->QueueTask([me, maxValue](){
		me->_maxValue = min(maxValue, ::SettingGetBitmapRangeMax(me->_isLog));
	});
}

void PlotWindowOpenedLineType::SetMinValue(double minValue)
{
	auto me = 
		dynamic_pointer_cast<PlotWindowOpenedLineType, AsyncObject>
		(shared_from_this());

	TaskQueue()->QueueTask([me, minValue](){
		me->_minValue = max(minValue, (double)INT_MIN);
	});
}

void PlotWindowOpenedLineType::UpdatePropertyAutoScale(bool bAutoScale)
{
	if (_propertyPage)
	{
		auto propertyPageLineType = dynamic_pointer_cast<PlotWindowLineTypeProperty, BaseProperty>(_propertyPage);
		propertyPageLineType->UpdateAutoScale(bAutoScale);
	}	
}

void PlotWindowOpenedLineType::UpdatePropertyMaxMin(double max, double min)
{
	if (_propertyPage)
	{
		auto propertyPageLineType = dynamic_pointer_cast<PlotWindowLineTypeProperty, BaseProperty>(_propertyPage);
		propertyPageLineType->UpdateMaxMin(max, min);
	}
}

void PlotWindowOpenedLineType::DrawGrid(Graphics * g, const CRect &clientRect, double dispMax, double dispMin)
{
	gridSolutionSolver.Solve(dispMax, dispMin, clientRect.Height(), _isLog);

	gridSolutionSolver.ForEachGridLine([=](double dataStep, double dataValue, int pixelValue){
		DrawGridLine(g, dataStep, dataValue, clientRect, dispMax, dispMin);
	});

	DrawXAxis(g, clientRect, this->_range);
}

void PlotWindowOpenedLineType::DrawGridLine(Graphics * g, double resolution, double yData, const CRect & clientRect, double dispMax, double dispMin)
{

	int ycenter = (clientRect.top + clientRect.bottom) / 2;
	int xleft = clientRect.left;
	int xright = clientRect.right;

	Gdiplus::REAL fontHeight = 10.0;

	int yView = (int)this->GetClientY((double)yData, clientRect, dispMax, dispMin);

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
	::FormatCommaString(strValue, yData, resolution); 

	PointF pointF((Gdiplus::REAL)(xleft + fontLeftMargin), (Gdiplus::REAL)(yView - 16));
	g->DrawString(strValue, -1, &gridFont, pointF, &format, &fontBrush);

	// Draw line
	g->DrawLine(&pen, xleft, yView, xright, yView);
}

Gdiplus::REAL PlotWindowOpenedLineType::GetClientY(double y, const CRect & clientRect, double dispMax, double dispMin)
{
	double rangeOld = dispMax - dispMin;
	int rangeNew = clientRect.bottom - clientRect.top;
	return float(clientRect.bottom - 
		(y - dispMin) * rangeNew / rangeOld);
}

std::shared_ptr<SoraDbgPlot::Task::TaskSimple> PlotWindowOpenedLineType::TaskUpdate()
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedLineType, AsyncObject>(shared_from_this());

	TaskQueue()->DoTask([SThis](){
		SThis->_operationUpdate->_vecChLineType.clear();
		SThis->_operationUpdate->_vecChText.clear();
		SThis->_operationUpdate->_vecChUpdate.clear();

		bool bUpdateChannel = SThis->_isPlay && (SThis->_latestIdx == 0);

		for (auto iter = SThis->_chMap.begin(); iter != SThis->_chMap.end(); ++iter)
		{
			auto channel = *(iter);
			auto channelLine = dynamic_pointer_cast<ChannelOpenedLineType, ChannelOpened>(channel);
			if (channelLine)
				SThis->_operationUpdate->_vecChLineType.push_back(channelLine);
			auto channelText = dynamic_pointer_cast<ChannelOpenedText, ChannelOpened>(channel);
			if (channelText)
				SThis->_operationUpdate->_vecChText.push_back(channelText);
			if ( bUpdateChannel || (SThis->_newChSet.find(channel->Id()) != SThis->_newChSet.end()) )
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
		SThis->_operationUpdate->param->_bMaxMinInitialized = false;
		SThis->_operationUpdate->param->_rect = SThis->GetBitmapRect();
		SThis->_operationUpdate->param->_index = SThis->_latestIdx;
		SThis->_operationUpdate->param->_range = SThis->_range;
		SThis->_operationUpdate->param->_bTimeStampTaken = false;
		SThis->_operationUpdate->param->_timeStamp = 0;
		SThis->_operationUpdate->_applyAutoScale = SThis->_bAutoScale;

		if (dynamic_pointer_cast<PlotWindowOpenedSpectrum, PlotWindowOpenedLineType>(SThis))
		{
			*(SThis->_operationUpdate->dataSize) = (size_t)(-1); // max unsigned int
		}
		else
		{
			*(SThis->_operationUpdate->dataSize) = 0;
		}
	});

	auto taskUpdateChannel = make_shared<TaskCoordinator>(TaskQueue());
	for (auto iterChannel = _operationUpdate->_vecChUpdate.begin(); iterChannel != _operationUpdate->_vecChUpdate.end(); ++iterChannel)
	{
		taskUpdateChannel->AddTask((*iterChannel)->TaskUpdateData(std::shared_ptr<bool>()));
	}

	//auto taskProcesslineGet = make_shared<TaskCoordinator>(TaskQueue());
	for (auto iterChannel = _operationUpdate->_vecChLineType.begin(); iterChannel != _operationUpdate->_vecChLineType.end(); ++iterChannel)
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
	for (auto iterChannel = _operationUpdate->_vecChLineType.begin(); iterChannel != _operationUpdate->_vecChLineType.end(); ++iterChannel)
	{
		taskCalcInfo->ContinueWith((*iterChannel)->TaskProcessData(this->_operationUpdate->param, false));
	}

	auto taskModifyParam = make_shared<TaskSimple>(TaskQueue(), [SThis](){
		double max = SThis->_operationUpdate->param->_dispMaxValue;
		double min = SThis->_operationUpdate->param->_dispMinValue;

		bool bMaxMinValid = SThis->_operationUpdate->param->_bMaxMinInitialized;

		if (SThis->_operationUpdate->_applyAutoScale)
		{
			if (bMaxMinValid)
			{
				double center = (max + min) / 2;
				double dist = (max - min) / 2;
				dist = max(dist, 0.001);
				double maxHigh = center + dist * 11 / 4;
				double minHigh = center - dist * 11 / 4;
				double maxDest = center + dist * 3 / 2;
				double minDest = center - dist * 3 / 2;
				double maxLow = center + dist * 9 / 8;
				double minLow = center - dist * 9 / 8;

				if ( (SThis->_maxValueAfterAutoScale < maxLow) || (SThis->_maxValueAfterAutoScale > maxHigh) )
					SThis->_maxValueAfterAutoScale = maxDest;

				if ( (SThis->_minValueAfterAutoScale > minLow) || (SThis->_minValueAfterAutoScale < minHigh) )
					SThis->_minValueAfterAutoScale = minDest;

				SThis->gridSolutionSolver.SolveWithMaxMinOptimization(
					SThis->_maxValueAfterAutoScale,	// in
					SThis->_minValueAfterAutoScale,	// in
					SThis->_maxValue,	// out
					SThis->_minValue,	// out
					SThis->_operationUpdate->param->_rect.Height(), 
					SThis->_operationUpdate->param->_bIsLog);
			}
		}
		else
		{}
		
		SThis->_operationUpdate->param->_dispMaxValue = SThis->_maxValue;
		SThis->_operationUpdate->param->_dispMinValue = SThis->_minValue;
	});

	auto taskDraw = make_shared<TaskSimple>(TaskQueue(), [SThis](){
		Gdiplus::Graphics * g = SThis->_operationUpdate->param->_g;
		CRect rect = SThis->_operationUpdate->param->_rect;
		SThis->DrawBackground(g, SThis->_bitmapRect);
		SThis->DrawLabel(g, SThis->_bitmapRect);

		if (SThis->_operationUpdate->param->_bMaxMinInitialized)
		{
			double max = SThis->_operationUpdate->param->_dispMaxValue;
			double min = SThis->_operationUpdate->param->_dispMinValue;

			if (SThis->_bShowGrid)
			{
				SThis->DrawGrid(g, rect, max, min);
			}

			if (SThis->_operationUpdate->_applyAutoScale)
			{
				SThis->UpdatePropertyMaxMin(SThis->_maxValue, SThis->_minValue);
			}
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

	for (auto iterChannel = _operationUpdate->_vecChLineType.begin(); iterChannel != _operationUpdate->_vecChLineType.end(); ++iterChannel)
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
				delete SThis->_operationUpdate->bmp;

			SThis->_operationUpdate->bmp = 0;
			delete SThis->_operationUpdate->param->_g;
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

HRESULT PlotWindowOpenedLineType::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	PlotWindowOpenedBitmapType::AppendXmlProperty(pDom, pParent);

	auto SThis = dynamic_pointer_cast<PlotWindowOpenedLineType, AsyncObject>(shared_from_this());
	this->DoNow([SThis, pDom, pParent](){
		
		IXMLDOMElement *pElement = NULL;

		CreateAndAddElementNode(pDom, L"Logarithm", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_isLog);

		CreateAndAddElementNode(pDom, L"WndautoScale", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_bAutoScale);

		CreateAndAddElementNode(pDom, L"WndmaxValue", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_maxValue);

		CreateAndAddElementNode(pDom, L"WndminValue", L"\n\t\t", pParent, &pElement);
		pElement->put_text((_bstr_t)SThis->_minValue);

	});

	return S_OK;
}

HRESULT PlotWindowOpenedLineType::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	PlotWindowOpenedBitmapType::LoadXmlElement(pElement);

	CString cs;
	MSXML2::IXMLDOMNodePtr pNode;
	
	pNode = pElement->selectSingleNode(L"Logarithm");
	if (pNode != 0)
	{
		if(pNode->text == (_bstr_t)(L"0"))
			this->_isLog = false;
		else
			this->_isLog = true;
	}

	pNode = pElement->selectSingleNode(L"WndautoScale");
	if (pNode != 0)
	{
		if(pNode->text == (_bstr_t)(L"0"))
			this->_bAutoScale = false;
		else
			this->_bAutoScale = true;
	}

	pNode = pElement->selectSingleNode(L"WndmaxValue");
	if (pNode != 0)
	{
		this->_maxValue = atof((LPCSTR)(pNode->text));
	}

	pNode = pElement->selectSingleNode(L"WndminValue");
	if (pNode != 0)
	{
		this->_minValue = atof((LPCSTR)(pNode->text));
	}

	return S_OK;
}

void PlotWindowOpenedLineType::OnCoordinateChanged()
{
	if (_bAutoScale)
		return;

	if (_isLog)			// change from general coordinate to log coordinate
	{
		double maxValue = _maxValue;
		if (maxValue == 0)
			maxValue = 0.01;

		double minValue = _minValue;
		if (minValue == 0)
			minValue = -0.01;

		double logValue1 = log10(abs(maxValue));
		double logValue2 = log10(abs(minValue));
		
		_maxValue = max(logValue1, logValue2);

		if ( ((_maxValue > 0) && (_minValue > 0)) ||
			((_maxValue < 0) && (_minValue < 0)) ) // same sign
		{
			_minValue = min(logValue1, logValue2);
		}
		else
		{
			_minValue = -0.1;
		}
	}
	else
	{
		double maxValueGeneral = pow(10.0, (double)_maxValue);
		double minValueGeneral = pow(10.0, (double)_minValue);

		_maxValue = maxValueGeneral;

		if (_minValue > 0)
			_minValue = minValueGeneral;
		else
			_minValue = -(_maxValue / 6);
	}

	double maxValidValue = ::SettingGetBitmapRangeMax(_isLog);
	double minValidValue = ::SettingGetBitmapRangeMin(_isLog);

	_maxValue = max(_minValue + ::SettingGetBitmapRangeHalfDelta(_isLog), _maxValue);
	_maxValue = min(_maxValue, maxValidValue);
	_minValue = min(_maxValue - ::SettingGetBitmapRangeHalfDelta(_isLog), _minValue);
	_minValue = max(_minValue, minValidValue);

	this->UpdatePropertyMaxMin(_maxValue, _minValue);
}

bool PlotWindowOpenedLineType::IsRangeSettingEnabled()
{
	return false;
}
