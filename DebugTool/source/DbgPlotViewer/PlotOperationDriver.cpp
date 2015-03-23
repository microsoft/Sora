#include "stdafx.h"
#include <assert.h>
#include "PlotOperationDriver.h"

using namespace std;
using namespace SoraDbgPlot::Task;

PlotOperationDriver::PlotOperationDriver()
{
	_taskQueue = make_shared<TaskQueue>();
}

PlotOperationDriver::~PlotOperationDriver()
{
}

void PlotOperationDriver::AddPlotWnd(shared_ptr<PlotWindowOpened> plotWnd)
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis, plotWnd](){
		auto iter = SThis->_mapPlotWnds.find(plotWnd);
		assert(iter == SThis->_mapPlotWnds.end());
		SThis->_mapPlotWnds.insert(
			make_pair(plotWnd, true)
			);
	});
}

void PlotOperationDriver::RemovePlotWnd(shared_ptr<PlotWindowOpened> plotWnd)
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis, plotWnd](){
		auto iter = SThis->_mapPlotWnds.find(plotWnd);
		assert(iter != SThis->_mapPlotWnds.end());
		SThis->_mapPlotWnds.erase(iter);
	});	
}

void PlotOperationDriver::Plot()
{
	auto SThis = shared_from_this();
	_taskQueue->QueueTask([SThis](){
		for (auto iter = SThis->_mapPlotWnds.begin(); iter != SThis->_mapPlotWnds.end(); ++iter)
		{
			auto plotWnd = iter->first;
			auto tokenReturned = iter->second;
			if (tokenReturned)
			{
				auto task = plotWnd->TaskUpdate();
				auto SThis2 = SThis;
				task->ContinueWith(make_shared<TaskSimple>(SThis->_taskQueue, [SThis2, plotWnd](){
					auto iter = SThis2->_mapPlotWnds.find(plotWnd);
					if (iter != SThis2->_mapPlotWnds.end())
					{
						// return token
						iter->second = true;
					}
				}));
				
				// give token
				iter->second = false;
				task->Run();
			}
		}
	});	
}
