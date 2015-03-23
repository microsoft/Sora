#pragma once

#include <Windows.h>
#include "SeriesObj.h"
#include "PlotWnd.h"

#ifndef CPLOT_WND_DEFINED
#error compile error
#endif

class PlotWndProp : public ObjProp
{
public:	// serialize
	CRect rect;
	int frameCount;
	bool nameIsSet;							// new_for_lili

public:
	SoraDbgPlot::Event::Event<CString> EventNameChanged;
	void SetName(const CString & name);
	CString GetName();
private:
	CString _name;


private:	// serialize(ptr)
	PlotWndAreaProp * plotWndAreaProp;		// weak ptr
public:
	PlotWndAreaProp * GetPlotWndArea();
	void SetPlotWndArea(PlotWndAreaProp * plotWndArea);

public:		// serialize(ptr)
	std::vector<SeriesProp *> seriesInPlotWndProp;		// strong ptr

public:
	CWnd * targetWnd;

public:
	PlotWndProp();
	virtual ~PlotWndProp();

	DebuggingProcessProp * GetProcess();
	void ForceUpdateGraph();
	void AddSeries(SeriesProp *);
	void RemoveSeries(SeriesProp *);
	void NotifyClosed();
	virtual HRESULT CreateElementPlot(IXMLDOMDocument *pDom, IXMLDOMElement *pe) = 0;
	CPlotWnd * GetPlotWnd(CWnd * parent);

protected:
	virtual CPlotWnd * CreatePlotWnd(CWnd * parent) = 0;
	virtual void AddPlotWndStrategy(CPlotWnd *);

public: // const
	int captionHeight;
	int margin;
	int resizeMargin;
	int closeButtonSize;
	int closeButtonMargin;
	
	COLORREF frameColor;
	COLORREF frameHighlightColor;
	COLORREF closeColor;
	COLORREF captionFontColor;

	virtual void PlotGraphToScreen();
	virtual Bitmap * CreateBitmap();

	HANDLE hMutexTree;

	int replayReadPos;
	int lastReplayReadPos;
	bool forceUpdate;

	// sync
public:
	virtual void PlayAFrame();

	// play/pause
public:
	void EnableDataUpdate();
	void DisableDataUpdate();
	bool IsDataUpdateEnabled();
private:
	bool _isDataUpdateEnabled;

	// Update Task
private:
	static DWORD WINAPI ThreadProcUpdate(__in  LPVOID lpParameter);

private:
	SoraDbgPlot::Task::WaitableLatestTaskQueue _taskQueue;
protected:
	virtual void UpdatePlotWnd() = 0;

public:
	virtual bool Accept(SeriesProp * series) = 0;

public:
	virtual void PlayPauseProcess(bool bPlay) = 0;
};

