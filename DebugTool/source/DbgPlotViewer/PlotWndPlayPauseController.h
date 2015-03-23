#pragma once

#include <map>
#include "AsyncObject.h"
#include "ProcessOpened.h"
#include "PlotWindowOpened.h"

class PlotWndPlayPauseController : public AsyncObject
{
public:
	~PlotWndPlayPauseController();
	void PauseProcess(std::shared_ptr<ProcessOpened>);
	void AddProcessPlotWnd(std::shared_ptr<ProcessOpened>, std::shared_ptr<PlotWindowOpened>);
	void RemovePlotWnd(std::shared_ptr<PlotWindowOpened>);
	void SelectProcess(std::shared_ptr<ProcessOpened>);
	void PlayPauseSelectedProcess(bool);
private:
	std::map<std::shared_ptr<ProcessOpened>, std::set<std::shared_ptr<PlotWindowOpened> > > _processWndMap;
	std::shared_ptr<ProcessOpened> _processSelected;
};
