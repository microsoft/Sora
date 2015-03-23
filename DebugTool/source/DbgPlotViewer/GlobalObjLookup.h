#pragma once

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "SharedChannel.h"
#include "SharedProcess.h"
#include "TaskQueue.h"
#include "ProcessOpened.h"
#include "ChannelOpened.h"
#include "Event.h"
#include "TaskSimple.h"
#include "PlotWindowOpened.h"

class ObjLookup : public std::enable_shared_from_this<ObjLookup>
{
public:
	typedef std::map<std::shared_ptr<ProcessOpened>, std::set<std::shared_ptr<ChannelOpened> > > ProcessChannelTree;

	SoraDbgPlot::Event::Event<ProcessChannelTree> EventProcessChannelTreeChanged;
	SoraDbgPlot::Event::Event<std::shared_ptr<ProcessOpened> > EventNewProcessOpened;
	SoraDbgPlot::Event::Event<std::shared_ptr<ProcessOpened> > EventProcessOpenedAttatched;
	SoraDbgPlot::Event::Event<std::shared_ptr<ProcessOpened> > EventProcessOpenedDeattatched;
	SoraDbgPlot::Event::Event<std::shared_ptr<ProcessOpened> > EventPauseProcess;

	ObjLookup();
	~ObjLookup();
	void Clear();
	
	void PollPauseEvent();
	void AddSharedChannel(const std::vector<std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> > &);
	void RemoveSharedChannel(const std::vector<std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel> > &);
	void AddSharedProcess(const std::vector<std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> > &);
	void RemoveSharedProcess(const std::vector<std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess> > &);
	void OpenProcess(std::shared_ptr<ProcessOpened>);
	void OpenChannel(std::shared_ptr<ChannelOpened>, const std::function<void(bool)> & );
	void CloseProcess(std::shared_ptr<ProcessOpened>);
	void CloseChannel(std::shared_ptr<ChannelOpened>);
	void CloseAll();

private:
	void RaiseProcessChannelUpdatedEvent();
	std::shared_ptr<ProcessOpened> MatchProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>);
	std::shared_ptr<ChannelOpened> MatchChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>, std::shared_ptr<ProcessOpened> & );

	std::shared_ptr<ProcessOpened> MatchProcess(const CString & name);
	std::shared_ptr<ProcessOpened> AddProcessUnattached(const CString & name);
	std::shared_ptr<ChannelOpened> MatchChannel(const CString & name, std::shared_ptr<ProcessOpened> process, const CString & type);
	void AddChannelUnattached(std::shared_ptr<ChannelOpened>, std::shared_ptr<ProcessOpened>);


	// add/remove one shared process/channel
	void AddSharedChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>);
	void AddSharedProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>);
	void RemoveSharedChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>);
	void RemoveSharedProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>);

	// 
	void AddProcess(std::shared_ptr<ProcessOpened>);
	void AddChannel(std::shared_ptr<ChannelOpened>, std::shared_ptr<ProcessOpened>);
	void RemoveProcess(std::shared_ptr<ProcessOpened>);
	void RemoveChannel(std::shared_ptr<ChannelOpened>);


	// channel factory function
	std::shared_ptr<ChannelOpened> MakeChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>);	

	// module name -> process
	std::multimap<std::wstring, std::shared_ptr<ProcessOpened> > _allProcessMap;

	// modulename|channelname -> channel
	std::multimap<std::wstring, std::shared_ptr<ChannelOpened> > _allChannelMap;
	
	// process -> map( id -> channel)
	std::map<std::shared_ptr<ProcessOpened>, std::set<std::shared_ptr<ChannelOpened> > > _processChannelMap;

	// channel -> process
	std::map<std::shared_ptr<ChannelOpened>, std::shared_ptr<ProcessOpened> > _channelProcessMap;

	// shared process -> process
	bool IsAttatched(std::shared_ptr<ProcessOpened>);
	std::shared_ptr<ProcessOpened> GetProcessByShared(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>);
	void AttatchShared2Process(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>, std::shared_ptr<ProcessOpened>);
	void UnattatchSharedProcess(std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>);
	std::map<std::shared_ptr<SoraDbgPlot::SharedObj::SharedProcess>, std::shared_ptr<ProcessOpened> > _shared2ProcessMap;
	std::set<std::shared_ptr<ProcessOpened> > _processAttatchedSet;

	bool IsAttatched(std::shared_ptr<ChannelOpened>);
	std::shared_ptr<ChannelOpened> GetChannelByShared(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>);
	void AttatchShared2Channel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>, std::shared_ptr<ChannelOpened>);	
	void UnattatchSharedChannel(std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>);
	std::map<std::shared_ptr<SoraDbgPlot::SharedObj::SharedChannel>, std::shared_ptr<ChannelOpened> > _shared2ChannelMap;
	std::set<std::shared_ptr<ChannelOpened> > _channelAttatchedSet;

	// attatch opend state helper func

	bool IsOpened(std::shared_ptr<ProcessOpened>);
	void MarkOpened(std::shared_ptr<ProcessOpened>, bool);
	std::set<std::shared_ptr<ProcessOpened> > _processOpenedSet;

	bool IsOpened(std::shared_ptr<ChannelOpened>);
	void MarkOpened(std::shared_ptr<ChannelOpened>, bool);
	std::set<std::shared_ptr<ChannelOpened> > _channelOpenedSet;

	// helper function for name
	std::wstring ModuleName(int pid);
	void ReleaseModuleName(int pid);

	std::wstring GetProcessNameByPid(int pid);

	// pid -> name
	std::map<int, std::wstring> _moduleName;

	// id -> name
	std::map<int, std::wstring> _channelName;

//-------------------------------------------------
public:
	void Save(const CString & filepath);
	void Load(const CString & filepath);
	void AddPlotWnd(std::shared_ptr<PlotWindowOpened>, std::shared_ptr<ProcessOpened>);
	void RemovePlotWnd(std::shared_ptr<PlotWindowOpened>);

	SoraDbgPlot::Event::Event<bool> EventSaveComplete;

	SoraDbgPlot::Event::Event<bool> EventLoadComplete;

// load
	struct PlotWindowLoadedEvent
	{
		std::shared_ptr<PlotWindowOpened> _plotWnd;
		std::shared_ptr<ProcessOpened> _process;
		CRect _rect;
	};
	SoraDbgPlot::Event::Event<PlotWindowLoadedEvent> EventPlotWindowLoaded;

	struct ChannelLoadedEvent
	{
		std::shared_ptr<PlotWindowOpened> _plotWnd;
		std::shared_ptr<ChannelOpened> _channel;
		CRect _rect;
	};
	SoraDbgPlot::Event::Event<ChannelLoadedEvent> EventChannelLoaded;
//

	void ClearPlotWindows();

private:
	void CloseAllPlotWindow();
	std::multimap<std::shared_ptr<ProcessOpened>, std::shared_ptr<PlotWindowOpened> > _mapProcessPlotwnd;

private:
	std::set<std::shared_ptr<ProcessOpened> > _loadLookupProcessSet;

private:
	volatile long _pollPauseEventToken;

private:
	// our great task queue
	std::shared_ptr<SoraDbgPlot::Task::TaskQueue> _taskQueue;
};

