#pragma once

#include <memory>
#include <vector>
#include <map>
#include "PlotWindowOpened.h"
#include "TaskQueue.h"

class PlotOperationDriver : public std::enable_shared_from_this<PlotOperationDriver>
{
public:
	PlotOperationDriver();
	~PlotOperationDriver();
	void AddPlotWnd(std::shared_ptr<PlotWindowOpened>);
	void RemovePlotWnd(std::shared_ptr<PlotWindowOpened>);
	void Plot();

private:
	std::map<std::shared_ptr<PlotWindowOpened>, bool> _mapPlotWnds;
	std::shared_ptr<SoraDbgPlot::Task::TaskQueue> _taskQueue;
};
