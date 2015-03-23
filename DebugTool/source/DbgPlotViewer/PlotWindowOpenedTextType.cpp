#include "stdafx.h"
#include "PlotWindowOpenedTextType.h"
#include "TextTypePlotWnd.h"
#include "PlotWindowTextTypeProperty.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotWindowOpenedTextType::PlotWindowOpenedTextType(std::shared_ptr<ProcessOpened> process) :
	PlotWindowOpened(process)
{
	_operationUpdate = make_shared<UpdateOperation>();
	_operationUpdate->_updated = std::make_shared<bool>();
}


PlotWindowOpenedTextType::~PlotWindowOpenedTextType()
{

}

void PlotWindowOpenedTextType::PlayPauseToLatest(bool bPlay)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedTextType, AsyncObject>(shared_from_this());
	this->DoLater([SThis, bPlay](){
		SThis->SetPlayPause(bPlay, true);
		if (SThis->_plotWnd)
		{
			auto textPlotWnd = (TextTypePlotWnd *)SThis->_plotWnd;
			if (bPlay)
				textPlotWnd->SetButtonStatePause();
			else
				textPlotWnd->SetButtonStatePlay();
		}
	});
}

CPlotWnd * PlotWindowOpenedTextType::CreatePlotWnd()
{
	auto plotWnd = new TextTypePlotWnd(this);


	auto SThis = dynamic_pointer_cast<PlotWindowOpenedTextType, AsyncObject>(shared_from_this());	
	plotWnd->EventPlayClicked.Subscribe([SThis](const void * sender, const TextTypePlotWnd::PlayPauseEvent & e){
		auto SThis2 = SThis;
		SThis->DoLater([SThis2, e](){
			SThis2->SetPlayPause(e.IsPlay, false);
		});
	});

	plotWnd->StrategyGetText.Set([SThis](const void * sender, const size_t & index, char * & addr){
		bool succ;
		auto SThis2 = SThis;
		SThis->TaskQueue()->DoTask([SThis2, &succ, &addr, index](){
			if (SThis2->_chMap.size() == 0)
			{
				succ = false;
				return;
			}

			auto channel = dynamic_pointer_cast<ChannelOpenedTextType, ChannelOpened>(*(SThis2->_chMap.begin()));

			addr = channel->GetData(index, true);

		});
	});

	return plotWnd;
}

std::shared_ptr<SoraDbgPlot::Task::TaskSimple> PlotWindowOpenedTextType::TaskUpdate()
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedTextType, AsyncObject>(shared_from_this());
	bool bUpdate = false;
	TaskQueue()->DoTask([SThis, &bUpdate](){

		if (SThis->_chMap.size() == 1)
		{
			SThis->_operationUpdate->_channel = 
				dynamic_pointer_cast<ChannelOpenedTextType, ChannelOpened>(
				*(SThis->_chMap.begin()));
		}
		else
		{
			SThis->_operationUpdate->_channel = 0;
		}

		bUpdate = SThis->_isPlay;
	});

	if (SThis->_operationUpdate->_channel == 0)
	{
		return make_shared<TaskSimple>(TaskQueue(), [](){});
	}

	shared_ptr<TaskSimple> taskUpdate;
	
	if (bUpdate)
		taskUpdate = SThis->_operationUpdate->_channel->TaskUpdateData(SThis->_operationUpdate->_updated);
	else
		taskUpdate = make_shared<TaskSimple>(TaskQueue(), [](){});

	auto size = make_shared<size_t>(0);

	auto taskGetSize = SThis->_operationUpdate->_channel->TaskGetSize(size);

	auto TaskFinish = make_shared<TaskSimple>(TaskQueue(), [SThis, size](){
		if (SThis->_plotWnd)
		{
			auto plotWndText = (TextTypePlotWnd *)SThis->_plotWnd;
			if (SThis->_isPlay)
			{
				size_t itemCount = *size;
				if (*SThis->_operationUpdate->_updated)
					plotWndText->UpdateView(itemCount);
			}

			if (SThis->_isPlay)
			{
				plotWndText->SetButtonStatePause();
			}
			else
			{
				plotWndText->SetButtonStatePlay();
			}
		}
	});

	taskUpdate
		->ContinueWith(taskGetSize)
		->ContinueWith(TaskFinish);

	return taskUpdate;
}

HRESULT PlotWindowOpenedTextType::AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent)
{
	PlotWindowOpened::AppendXmlProperty(pDom, pParent);

	return S_OK;
}

HRESULT PlotWindowOpenedTextType::LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement)
{
	PlotWindowOpened::LoadXmlElement(pElement);

	return S_OK;
}

std::shared_ptr<BaseProperty> PlotWindowOpenedTextType::CreatePropertyPage()
{
	shared_ptr<PlotWindowTextTypeProperty> propertyPage;
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedTextType, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, &propertyPage](){
		propertyPage = make_shared<PlotWindowTextTypeProperty>(
			SThis->GetTypeName(),
			SThis->_name
			);

		auto SThis2 = SThis;
		propertyPage->EventName.Subscribe([SThis2](const void * sender, const wstring & name){
			SThis2->SetName(name);
		});

	});

	return propertyPage;
}

void PlotWindowOpenedTextType::OnChannelAdded(std::shared_ptr<ChannelOpened> channel, const CRect & rect)
{
	auto channelTextType = dynamic_pointer_cast<ChannelOpenedTextType, ChannelOpened>(channel);
	assert(channelTextType != 0);

	auto SThis = dynamic_pointer_cast<PlotWindowOpenedTextType, AsyncObject>(shared_from_this());

	channelTextType->EventColor.Subscribe([SThis](const void * sender, const COLORREF & color){
		SThis->SetColor(color);
	});

	channelTextType->ForColor([SThis](COLORREF color){
		SThis->SetColor(color);
	});
}

void PlotWindowOpenedTextType::SetColor(COLORREF color)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedTextType, AsyncObject>(shared_from_this());

	this->DoLater([SThis, color](){
		if (SThis->_plotWnd)
		{
			auto plotWndTextType = dynamic_cast<TextTypePlotWnd *>(SThis->_plotWnd);
			assert(plotWndTextType);
			plotWndTextType->SetColor(color);
		}
	});
}
