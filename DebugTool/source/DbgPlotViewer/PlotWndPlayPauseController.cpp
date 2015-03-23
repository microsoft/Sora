#include "stdafx.h"
#include "PlotWndPlayPauseController.h"

using namespace std;

PlotWndPlayPauseController::~PlotWndPlayPauseController()
{
}

void PlotWndPlayPauseController::AddProcessPlotWnd(shared_ptr<ProcessOpened> process, shared_ptr<PlotWindowOpened> plotWnd)
{
	auto SThis = dynamic_pointer_cast<PlotWndPlayPauseController, AsyncObject>(shared_from_this());
	this->DoLater([SThis, process, plotWnd](){
		auto pair = SThis->_processWndMap.find(process);
		if (pair == SThis->_processWndMap.end())
		{
			SThis->_processWndMap.insert(
				make_pair(process, set<shared_ptr<PlotWindowOpened> >())
				);
			pair = SThis->_processWndMap.find(process);
		}
		pair->second.insert(plotWnd);
	});
}

void PlotWndPlayPauseController::RemovePlotWnd(shared_ptr<PlotWindowOpened> plotWnd)
{
	auto SThis = dynamic_pointer_cast<PlotWndPlayPauseController, AsyncObject>(shared_from_this());
	this->DoLater([SThis, plotWnd](){
		for (
			auto iterPs = SThis->_processWndMap.begin();
			iterPs != SThis->_processWndMap.end();
			++iterPs)
		{
			if (iterPs->second.find(plotWnd) != iterPs->second.end())
			{
				iterPs->second.erase(plotWnd);
				if (iterPs->second.size() == 0)
				{
					if (iterPs->first == SThis->_processSelected)
						SThis->_processSelected = 0;

					SThis->_processWndMap.erase(iterPs);
				}

				return;
			}
		}

		assert(false);
	});
}

void PlotWndPlayPauseController::PauseProcess(std::shared_ptr<ProcessOpened> process)
{
	auto SThis = dynamic_pointer_cast<PlotWndPlayPauseController, AsyncObject>(shared_from_this());
	this->DoLater([SThis, process](){
		auto iterPs = SThis->_processWndMap.find(process);
		if (iterPs == SThis->_processWndMap.end())
			return;
		
		for (
			auto iterWnd = iterPs->second.begin();
			iterWnd != iterPs->second.end();
			++iterWnd)
		{
			(*iterWnd)->PlayPauseToLatest(false);
		}
	});
}

void PlotWndPlayPauseController::SelectProcess(std::shared_ptr<ProcessOpened> process)
{
	auto SThis = dynamic_pointer_cast<PlotWndPlayPauseController, AsyncObject>(shared_from_this());
	this->DoLater([SThis, process](){
		SThis->_processSelected = process;
	});
}

void PlotWndPlayPauseController::PlayPauseSelectedProcess(bool bPlay)
{
	auto SThis = dynamic_pointer_cast<PlotWndPlayPauseController, AsyncObject>(shared_from_this());
	this->DoLater([SThis, bPlay](){
		if (SThis->_processSelected == 0)
			return;

		auto iterPs = SThis->_processWndMap.find(SThis->_processSelected);
		if (iterPs == SThis->_processWndMap.end())
			return;
		
		for (
			auto iterWnd = iterPs->second.begin();
			iterWnd != iterPs->second.end();
			++iterWnd)
		{
			(*iterWnd)->PlayPauseToLatest(bPlay);
		}
	});
}
