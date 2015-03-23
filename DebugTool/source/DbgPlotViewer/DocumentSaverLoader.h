#pragma once
#include <memory>
#include <map>
#include "AsyncObject.h"
#include "PlotWindowOpened.h"
#include "Event.h"
#include "TaskSimple.h"

#import <msxml6.dll>

class DocumentSaverLoader : public AsyncObject
{
public:
	DocumentSaverLoader();
	~DocumentSaverLoader();

	void Save(const CString & filepath);
	void Load(const CString & filepath);
	void AddPlotWnd(std::shared_ptr<PlotWindowOpened>, std::shared_ptr<ProcessOpened>);
	void RemovePlotWnd(std::shared_ptr<PlotWindowOpened>);

	SoraDbgPlot::Event::Event<bool> EventSaveComplete;

	SoraDbgPlot::Event::Event<bool> EventLoadComplete;

	struct PlotWindowLoadedEvent
	{
		std::shared_ptr<PlotWindowOpened> _plotWnd;
		std::shared_ptr<ProcessOpened> _process;
	};

	SoraDbgPlot::Event::Event<PlotWindowLoadedEvent> EventPlotWindowLoaded;

	struct ChannelLoadedEvent
	{
		std::shared_ptr<PlotWindowOpened> _plotWnd;
		std::shared_ptr<ChannelOpened> _channel;
	};

	SoraDbgPlot::Event::Event<ChannelLoadedEvent> EventChannelLoaded;


public:
	void SaveXMLDoc();
	void AddElement(const wchar_t * name, const wchar_t * value);
	void DoneElement();
	void ProcessChild(const std::function<void(const wchar_t *, const wchar_t *)> &);
	void ClearPlotWindows();
	void Clear();

private:
	void CloseAllPlotWindow();
	std::multimap<std::shared_ptr<ProcessOpened>, std::shared_ptr<PlotWindowOpened> > _mapProcessPlotwnd;
	bool _bWorking;

private:
	IXMLDOMDocument * pXmlDoc;
	IXMLDOMElement * pCurElement;
	CString currentName;
};



