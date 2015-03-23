#pragma once

#include <memory>
#include <set>
#include "AsyncObject.h"
#include "ProcessOpened.h"
#include "ControlPanelWnd.h"
#include "ControlPanelList.h"

class ControlPanelDriver : public AsyncObject
{
public:
	~ControlPanelDriver();
	std::shared_ptr<CWnd> GetControlWnd();
	void AddProcessOpened(std::shared_ptr<ProcessOpened>);
	void RemoveProcess(std::shared_ptr<ProcessOpened>);
	void Update();
	void Clear();
private:
	std::shared_ptr<ControlPanelList> _controlPanelList;
	std::map<std::shared_ptr<ProcessOpened>, std::shared_ptr<ControlPanelWnd> > _processWndMap;
	std::map<std::shared_ptr<ProcessOpened>, bool> _processIsUpdating;
};
