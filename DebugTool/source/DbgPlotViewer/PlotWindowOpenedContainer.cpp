#include "stdafx.h"
#include <memory>
#include "DbgPlotViewer.h"
#include "ChannelOpened.h"
#include "ChannelOpenedLine.h"
#include "ChannelOpenedDots.h"
#include "ChannelOpenedLog.h"
#include "PlotWindowOpened.h"
#include "PlotWindowOpenedLine.h"
#include "PlotWindowOpenedSpectrum.h"
#include "PlotWindowOpenedDots.h"
#include "PlotWindowOpenedText.h"
#include "PlotWindowOpenedLog.h"
#include "PlotWindowOpenedContainer.h"
#include "ViewerAppConst.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotWindowOpenedContainer::PlotWindowOpenedContainer()
{
	_gridSize = 20;
	_plotWndContainer = 0;
	_taskQueue = make_shared<SoraDbgPlot::Task::TaskQueue>();
}

PlotWindowOpenedContainer::~PlotWindowOpenedContainer()
{
	delete _plotWndContainer;
}

void PlotWindowOpenedContainer::Clear()
{

	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());

	auto clearTask = [SThis](){

		SThis->EventBringToFront.Reset();
		SThis->EventPlotWndAdded.Reset();
		SThis->EventAddChannelRequest.Reset();
		SThis->EventAddPlotWndRequest.Reset();
		SThis->EventCloseChannel.Reset();
		SThis->EventClosePlotWnd.Reset();
		SThis->EventProcessSelected.Reset();

		SThis->ChannelAddable::Clear();
	};

	TaskQueue()->QueueTask(clearTask, clearTask);
}

void PlotWindowOpenedContainer::AddPlotWindow(std::shared_ptr<PlotWindowOpened> plotWindowOpened, CRect rect)
{
// route the event up for arbitrate

	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());

	plotWindowOpened->EventAddChannelRequest.Subscribe([SThis](const void * sender, const PlotWindowOpened::AddChannelEvent & e){
		auto SThis2 = SThis;
		SThis2->TaskQueue()->QueueTask([SThis2, e](){

			for (auto iterWnd = SThis2->_vecPlotWindowOpened.begin(); iterWnd != SThis2->_vecPlotWindowOpened.end(); ++iterWnd)
			{
				auto plotWnd = *iterWnd;
				plotWnd->RemoveChannelOpened(e._channel);
			}

			PlotWindowOpenedContainer::AddChannelRequestEvent ee;
			ee._plotWnd = e._plotWnd;
			ee._channel = e._channel;
			ee._rect = e._rect;
			SThis2->EventAddChannelRequest.Raise(SThis2.get(), ee);
		});
	});

	plotWindowOpened->EventBringToFront.Subscribe([SThis](const void * sender, const shared_ptr<PlotWindowOpened> & e){
		SThis->EventBringToFront.Raise(sender, e); 
	});

	plotWindowOpened->EventProcessSelected.Subscribe([SThis](const void * sender, const shared_ptr<ProcessOpened> & e){
		SThis->EventProcessSelected.Raise(sender, e);
	});

	// raise the event up to notify
	plotWindowOpened->EventCloseChannel.Subscribe([SThis](const void * sender, const PlotWindowOpened::CloseChannelEvent & e){
		auto SThis2 = SThis;
		SThis2->TaskQueue()->QueueTask([SThis2, e](){
			PlotWindowOpenedContainer::CloseChannelEvent ee;
			ee._channel = e._channel;
			SThis2->EventCloseChannel.Raise(SThis2.get(), ee);
		});
	});

	// remove channel from vec
	// raise the event up to notify
	plotWindowOpened->EventPlotWndClosed.Subscribe([SThis](const void * sender,  const std::shared_ptr<PlotWindowOpened> & e){
		auto SThis2 = SThis;
		SThis->TaskQueue()->QueueTask([SThis2, e](){
			auto plotWnd = e;
			SThis2->RemovePlotWindow(plotWnd);
			PlotWindowOpenedContainer::ClosePlotWndEvent e;
			e._plotWnd = plotWnd;
			SThis2->EventClosePlotWnd.Raise(SThis2.get(), e);
		});
	});

	//plotWindowOpened->AddChannelOpened(channel, rect);

	_vecPlotWindowOpened.push_back(plotWindowOpened);

	CPlotWnd * plotWnd = plotWindowOpened->GetPlotWnd();

	_plotWndContainer->AddPlotWindow(plotWnd, rect);

	//struct PlotWndAddedParam e;
	//e._process = process;
	//e._plotWnd = plotWindowOpened;
	//EventPlotWndAdded.Raise(this, e);
}

void PlotWindowOpenedContainer::AddPlotWindow(shared_ptr<ProcessOpened> process, shared_ptr<ChannelOpened> channel, CPoint point)
{
	shared_ptr<PlotWindowOpened> plotWindowOpened;
	
	// factory
	if (dynamic_pointer_cast<ChannelOpenedLine, ChannelOpened>(channel))
	{
		plotWindowOpened = make_shared<PlotWindowOpenedLine>(process);
	}
	else if (dynamic_pointer_cast<ChannelOpenedSpectrum, ChannelOpened>(channel))
	{
		plotWindowOpened = make_shared<PlotWindowOpenedSpectrum>(process);
	}
	else if (dynamic_pointer_cast<ChannelOpenedDots, ChannelOpened>(channel))
	{
		plotWindowOpened = make_shared<PlotWindowOpenedDots>(process);
	}
	else if (dynamic_pointer_cast<ChannelOpenedText, ChannelOpened>(channel))
	{
		plotWindowOpened = make_shared<PlotWindowOpenedText>(process);
	}
	else if (dynamic_pointer_cast<ChannelOpenedLog, ChannelOpened>(channel))
	{
		plotWindowOpened = make_shared<PlotWindowOpenedLog>(process);
	}

	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());

	// route the event up for arbitrate
	plotWindowOpened->EventAddChannelRequest.Subscribe([SThis](const void * sender, const PlotWindowOpened::AddChannelEvent & e){
		auto SThis2 = SThis;
		SThis2->TaskQueue()->QueueTask([SThis2, e](){

			for (auto iterWnd = SThis2->_vecPlotWindowOpened.begin(); iterWnd != SThis2->_vecPlotWindowOpened.end(); ++iterWnd)
			{
				auto plotWnd = *iterWnd;
				plotWnd->RemoveChannelOpened(e._channel);
			}

			PlotWindowOpenedContainer::AddChannelRequestEvent ee;
			ee._plotWnd = e._plotWnd;
			ee._channel = e._channel;
			ee._rect = e._rect;
			SThis2->EventAddChannelRequest.Raise(SThis2.get(), ee);
		});
	});

	plotWindowOpened->EventBringToFront.Subscribe([SThis](const void * sender, const shared_ptr<PlotWindowOpened> & e){
		SThis->EventBringToFront.Raise(sender, e); 
	});

	plotWindowOpened->EventProcessSelected.Subscribe([SThis](const void * sender, const shared_ptr<ProcessOpened> & e){
		SThis->EventProcessSelected.Raise(sender, e);
	});

	// raise the event up to notify
	plotWindowOpened->EventCloseChannel.Subscribe([SThis](const void * sender, const PlotWindowOpened::CloseChannelEvent & e){
		auto SThis2 = SThis;
		SThis2->TaskQueue()->QueueTask([SThis2, e](){
			PlotWindowOpenedContainer::CloseChannelEvent ee;
			ee._channel = e._channel;
			SThis2->EventCloseChannel.Raise(SThis2.get(), ee);
		});
	});

	// remove channel from vec
	// raise the event up to notify
	plotWindowOpened->EventPlotWndClosed.Subscribe([SThis](const void * sender,  const std::shared_ptr<PlotWindowOpened> & e){
		auto SThis2 = SThis;
		SThis->TaskQueue()->QueueTask([SThis2, e](){
			auto plotWnd = e;
			SThis2->RemovePlotWindow(plotWnd);
			PlotWindowOpenedContainer::ClosePlotWndEvent e;
			e._plotWnd = plotWnd;
			SThis2->EventClosePlotWnd.Raise(SThis2.get(), e);
		});
	});
	
	this->DoLater([SThis, plotWindowOpened, process, channel](){

		SThis->_vecPlotWindowOpened.push_back(plotWindowOpened);

		CPlotWnd * plotWnd = plotWindowOpened->GetPlotWnd();

		SThis->_plotWndContainer->AddPlotWindow(plotWnd);

		struct PlotWindowOpenedContainer::PlotWndAddedParam e;
		e._process = process;
		e._plotWnd = plotWindowOpened;
		SThis->EventPlotWndAdded.Raise(SThis.get(), e);

		//--

		CRect rect;
		rect.SetRectEmpty();
		plotWindowOpened->AddChannelOpened(channel, rect);

	});
}

void PlotWindowOpenedContainer::RemovePlotWindow(std::shared_ptr<PlotWindowOpened> plotWnd)
{
	for (auto iterWnd = _vecPlotWindowOpened.begin(); iterWnd != _vecPlotWindowOpened.end(); ++iterWnd)
	{
		if (*iterWnd == plotWnd)
		{
			_vecPlotWindowOpened.erase(iterWnd);
			break;
		}
	}
}

void PlotWindowOpenedContainer::AddChannel(std::shared_ptr<PlotWindowOpened> plotWnd, std::shared_ptr<ChannelOpened> channel, const CRect & rect)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, plotWnd, channel, rect]() {
		for (auto iterPlotWnd = SThis->_vecPlotWindowOpened.begin(); iterPlotWnd != SThis->_vecPlotWindowOpened.end(); ++iterPlotWnd)
		{
			auto plotWndExisted = *iterPlotWnd;
			if (plotWndExisted == plotWnd)
			{
				plotWndExisted->AddChannelOpened(channel, rect);
			}
		}
	});
}

void PlotWindowOpenedContainer::CloseChannel(std::shared_ptr<ChannelOpened> channel)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, channel]() {
		for (auto iterPlotWnd = SThis->_vecPlotWindowOpened.begin(); iterPlotWnd != SThis->_vecPlotWindowOpened.end(); ++iterPlotWnd)
		{
			auto plotWndExisted = *iterPlotWnd;
			plotWndExisted->RemoveChannelOpened(channel);
		}

		CloseChannelEvent e;
		e._channel = channel;
		SThis->EventCloseChannel.Raise(SThis.get(), e);
	});
}


void PlotWindowOpenedContainer::PrcessAttatchDetatch(std::shared_ptr<ProcessOpened> process, bool bIsAttatch)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, process, bIsAttatch]() {
		for (auto iterPlotWnd = SThis->_vecPlotWindowOpened.begin(); iterPlotWnd != SThis->_vecPlotWindowOpened.end(); ++iterPlotWnd)
		{
			auto plotWndExisted = *iterPlotWnd;
			plotWndExisted->ProcessAttatchDetatch(process, bIsAttatch);
		}
	});
}

CWnd * PlotWindowOpenedContainer::GetWnd()
{
	_plotWndContainer = new PlotWndContainer(this);
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());

	_plotWndContainer->EventSize.Subscribe([SThis](const void * sender, CRect rect){
		auto SThis2 = SThis;
		SThis2->TaskQueue()->QueueTask([SThis2, rect](){
			SThis2->_rect = rect;
		});
	});

	return _plotWndContainer;
}

void PlotWindowOpenedContainer::AutoLayout(int sizeSlider)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());
	TaskQueue()->QueueTask([SThis, sizeSlider](){

		int size = (sizeSlider - (sizeSlider % SThis->_gridSize));
		if (size == 0)
			return;

		int clientWidth = SThis->_rect.Width();
		int clientHeight = SThis->_rect.Height();

		int xNum = max(clientWidth / size, 1);

		int index = 0;
		for (auto iterPlotWnd = SThis->_vecPlotWindowOpened.begin(); iterPlotWnd != SThis->_vecPlotWindowOpened.end(); ++iterPlotWnd)
		{

			auto plotWnd = *iterPlotWnd;
			int xIndex = index % xNum;
			int yIndex = index / xNum;		
			plotWnd->SetRect(CRect(xIndex * size, yIndex * size, xIndex * size + size, yIndex * size + size));
			index++;
		}
	});
}

std::shared_ptr<SoraDbgPlot::Task::TaskQueue> PlotWindowOpenedContainer::TaskQueue()
{
	return _taskQueue;
}

void PlotWindowOpenedContainer::Highlight(bool bHighlight)
{
	if (_plotWndContainer)
		_plotWndContainer->HighLight(bHighlight);
}

bool PlotWindowOpenedContainer::Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint & pointOut)
{
	if (_plotWndContainer)
	{
		pointOut = pointIn;
		_plotWndContainer->ScreenToClient(&pointOut);
		CPoint dropPoint = pointOut;
		dropPoint.x -= dropPoint.x % _gridSize;
		dropPoint.y -= dropPoint.y % _gridSize;
		_plotWndContainer->SetDropPoint(dropPoint);
		return true;
	}
	else
		return true;
}


shared_ptr<BaseProperty> PlotWindowOpenedContainer::CreatePropertyPage()
{
	return 0;
}

void PlotWindowOpenedContainer::RequestAddChannel(std::shared_ptr<ProcessOpened> process, shared_ptr<ChannelOpened> channel, CPoint pointIn)
{
	auto SThis = dynamic_pointer_cast<PlotWindowOpenedContainer, AsyncObject>(shared_from_this());
	SThis->TaskQueue()->QueueTask([SThis, process, channel, pointIn](){

		for (auto iterWnd = SThis->_vecPlotWindowOpened.begin(); iterWnd != SThis->_vecPlotWindowOpened.end(); ++iterWnd)
		{
			auto plotWnd = *iterWnd;
			plotWnd->RemoveChannelOpened(channel);
		}

		PlotWindowOpenedContainer::CloseChannelEvent ee;
		ee._channel = channel;
		SThis->EventCloseChannel.Raise(SThis.get(), ee);

		PlotWindowOpenedContainer::AddPlotWndParam param;
		param._process = process;
		param._channel = channel;
		param._point = pointIn;
		SThis->EventAddPlotWndRequest.Raise(SThis.get(), param);
	});
}
