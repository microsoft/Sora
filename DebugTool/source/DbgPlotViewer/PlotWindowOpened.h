#pragma once

#include <memory>
#include <vector>
#include <string>
#include "ChannelOpened.h"
#include "TaskQueue.h"
#include "Event.h"
#include "PlotWnd.h"
#include "TaskSimple.h"
#include "ChannelAddable.h"

#import <msxml6.dll>

class PlotWindowOpened : public ChannelAddable
{
public:
	PlotWindowOpened(std::shared_ptr<ProcessOpened>);
	PlotWindowOpened(std::shared_ptr<ProcessOpened>, const std::wstring & name, bool bNameIsSet, const CRect & rect);

	virtual void PlayPauseToLatest(bool) = 0;

	virtual void Highlight(bool bHighlight);
	virtual void RequestAddChannel(std::shared_ptr<ProcessOpened> process, std::shared_ptr<ChannelOpened> channel, CPoint pointIn);

	void AddChannelOpened(std::shared_ptr<ChannelOpened>, const CRect & rect);
	void RemoveChannelOpened(std::shared_ptr<ChannelOpened>);
	void GetRect(CRect &);
	void SetRectSync(const CRect &);
	void ProcessAttatchDetatch(std::shared_ptr<ProcessOpened>, bool);

	void Close();

	CPlotWnd * GetPlotWnd();

	SoraDbgPlot::Event::Event<std::shared_ptr<PlotWindowOpened> > EventBringToFront;

	SoraDbgPlot::Event::Event<std::shared_ptr<ProcessOpened> > EventProcessSelected;

	struct AddChannelEvent
	{
		std::shared_ptr<PlotWindowOpened> _plotWnd;
		std::shared_ptr<ChannelOpened> _channel;
		CRect _rect;
	};
	SoraDbgPlot::Event::Event<AddChannelEvent> EventAddChannelRequest;

	struct CloseChannelEvent
	{
		std::shared_ptr<ChannelOpened> _channel;
	};
	SoraDbgPlot::Event::Event<CloseChannelEvent> EventCloseChannel;

	SoraDbgPlot::Event::Event<std::shared_ptr<PlotWindowOpened> > EventPlotWndClosed;

	void SetName(const std::wstring & name);
	SoraDbgPlot::Event::Event<std::wstring> EventNameChanged;

	void SetRect(const CRect & rect);
	SoraDbgPlot::Event::Event<CRect> EventRectChanged;

	void SetSnapGridSize(int size);

	virtual std::shared_ptr<SoraDbgPlot::Task::TaskSimple> TaskUpdate();
	virtual HRESULT CreateXmlElement(IXMLDOMDocument *pDom, IXMLDOMElement *pe);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);

protected:
	virtual void SeekToLatest();
	virtual CPlotWnd * CreatePlotWnd() = 0;
	virtual bool Accept(std::shared_ptr<ChannelOpened>, CPoint pointIn, CPoint & pointOut) = 0;
	virtual void OnChannelAdded(std::shared_ptr<ChannelOpened>, const CRect & rect);
	virtual void OnChannelClosed(std::shared_ptr<ChannelOpened>);
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);

	void SetPlayPause(bool bPlay, bool seekToLatest);
	virtual void OnProcessAttatched();

private:
	void AddPlotWndStrategy(CPlotWnd *);
	void UpdateWndCaption();

protected:
	std::shared_ptr<ProcessOpened> _processOpened;
	std::wstring _processName;
	int _pid;

	CPlotWnd * _plotWnd;
	std::wstring _name;
	bool _bNameIsSet;
	CRect _rect;
	std::set<std::shared_ptr<ChannelOpened> > _chMap;
	std::set<int> _newChSet;

	bool _isPlay;
	bool _isPlayLastState;

private:
	bool _bWndValid;
};

