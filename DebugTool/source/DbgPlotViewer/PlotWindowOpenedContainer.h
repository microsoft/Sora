#pragma once

#include <memory>
#include <vector>
#include "BaseProperty.h"
#include "TaskQueue.h"
#include "ChannelOpened.h"
#include "PlotWindowOpened.h"
#include "PlotWndContainer.h"
#include "Event.h"
#include "ChannelAddable.h"

class PlotWindowOpenedContainer : public ChannelAddable
{
public:
	PlotWindowOpenedContainer();
	~PlotWindowOpenedContainer();

	virtual std::shared_ptr<SoraDbgPlot::Task::TaskQueue> TaskQueue();
	virtual void Highlight(bool bHighlight);
	virtual bool Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint & pointOut);
	virtual void RequestAddChannel(std::shared_ptr<ProcessOpened> process, std::shared_ptr<ChannelOpened> channel, CPoint pointIn);

	virtual std::shared_ptr<BaseProperty> CreatePropertyPage();

	void Clear();

	void AddPlotWindow(std::shared_ptr<ProcessOpened>, std::shared_ptr<ChannelOpened>, CPoint point);
	void AddPlotWindow(std::shared_ptr<PlotWindowOpened>, CRect rect);
	void AddChannel(std::shared_ptr<PlotWindowOpened>, std::shared_ptr<ChannelOpened>, const CRect & rect);
	void CloseChannel(std::shared_ptr<ChannelOpened>);
	void AutoLayout(int size);
	void PrcessAttatchDetatch(std::shared_ptr<ProcessOpened>, bool);

	CWnd * GetWnd();

	struct AddPlotWndParam
	{
		std::shared_ptr<ProcessOpened> _process;
		std::shared_ptr<ChannelOpened> _channel;
		CPoint _point;
	};

	SoraDbgPlot::Event::Event<AddPlotWndParam> EventAddPlotWndRequest;

	struct PlotWndAddedParam
	{
		std::shared_ptr<ProcessOpened> _process;
		std::shared_ptr<PlotWindowOpened> _plotWnd;
	};

	SoraDbgPlot::Event::Event<PlotWndAddedParam> EventPlotWndAdded;

	struct ClosePlotWndEvent
	{
		std::shared_ptr<PlotWindowOpened> _plotWnd;
	};
	SoraDbgPlot::Event::Event<ClosePlotWndEvent> EventClosePlotWnd;

	struct AddChannelRequestEvent
	{
		std::shared_ptr<PlotWindowOpened> _plotWnd;
		std::shared_ptr<ChannelOpened> _channel;
		CRect _rect;
	};
	SoraDbgPlot::Event::Event<AddChannelRequestEvent> EventAddChannelRequest;

	struct CloseChannelEvent
	{
		std::shared_ptr<ChannelOpened> _channel;		
	};
	SoraDbgPlot::Event::Event<CloseChannelEvent> EventCloseChannel;

	SoraDbgPlot::Event::Event<std::shared_ptr<PlotWindowOpened> > EventBringToFront;
	SoraDbgPlot::Event::Event<std::shared_ptr<ProcessOpened> > EventProcessSelected;

private:
	void RemovePlotWindow(std::shared_ptr<PlotWindowOpened>);
	PlotWndContainer * _plotWndContainer;
	std::vector<std::shared_ptr<PlotWindowOpened> > _vecPlotWindowOpened;

	CRect _rect;
	int _gridSize;

	std::shared_ptr<SoraDbgPlot::Task::TaskQueue> _taskQueue;
};
