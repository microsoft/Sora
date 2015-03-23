#include "stdafx.h"
#include "PlotWndProp.h"
#include "HelperFunc.h"
#include "SubTextPlotWnd.h"
#include "TextPlotWndProperty.h"

/***********************************

Plot Wnd

************************************/
PlotWndProp::PlotWndProp()
{
	captionHeight = 20;
	margin = 3;
	resizeMargin = 2;
	closeButtonSize = 10;
	closeButtonMargin = 5;
	frameColor = RGB(28,48,69);
	closeColor = RGB(70,110,140);
	captionFontColor = RGB(150, 150, 150);
	frameHighlightColor = RGB(0, 50, 0);
	targetWnd = 0;
	frameCount = 10;
	nameIsSet = false;
	replayReadPos = -1;
	lastReplayReadPos = -1;
	forceUpdate = false;
	_isDataUpdateEnabled = true;
	hMutexTree = ::CreateMutex(NULL, FALSE, NULL);
}

PlotWndProp::~PlotWndProp()
{
	std::vector<SeriesProp *>::iterator iter;
	for (iter = seriesInPlotWndProp.begin();
		iter != seriesInPlotWndProp.end();
		iter++)
	{
		(*iter)->DecRefCount();
	}

	this->seriesInPlotWndProp.clear();

	::CloseHandle(hMutexTree);
	//CloseHandle(hEventUpdateComplete);
}

void PlotWndProp::AddSeries(SeriesProp * p)
{
	::WaitForSingleObject(hMutexTree, INFINITE);
	p->IncRefCount();
	this->seriesInPlotWndProp.push_back(p);
	p->SetPlotWndProp(this);
	::ReleaseMutex(hMutexTree);

	if (this->targetWnd)
		this->targetWnd->InvalidateRgn(NULL, 1);
}

void PlotWndProp::RemoveSeries(SeriesProp * p)
{
	::WaitForSingleObject(hMutexTree, INFINITE);
	std::vector<SeriesProp *>::iterator iter;
	for (iter = this->seriesInPlotWndProp.begin();
		iter != this->seriesInPlotWndProp.end();
		iter++)
	{
		if (*iter == p)
		{
			p->SetPlotWndProp(0);
			this->seriesInPlotWndProp.erase(iter);
			p->DecRefCount();
			break;
		}
	}

	if (this->targetWnd)
	{
		if (seriesInPlotWndProp.size() == 0)
			this->targetWnd->PostMessage(WM_CLOSE, 0, 0);
		else
			this->targetWnd->InvalidateRgn(NULL, 1);
	}
	::ReleaseMutex(hMutexTree);
}

void PlotWndProp::SetName(const CString & name)
{
	_name = name;
	nameIsSet = true;
	EventNameChanged.Raise(this, name);
}

CString PlotWndProp::GetName()
{
	return _name;
}

void PlotWndProp::PlayAFrame()
{
	// do nothing
}

PlotWndAreaProp * PlotWndProp::GetPlotWndArea()
{
	return this->plotWndAreaProp;
}

void PlotWndProp::SetPlotWndArea(PlotWndAreaProp * plotWndArea)
{
	this->plotWndAreaProp = plotWndArea;
}

DebuggingProcessProp * PlotWndProp::GetProcess()
{
	if (this->seriesInPlotWndProp.size() == 0)
		return 0;
	return this->seriesInPlotWndProp[0]->GetProcess();
}

void PlotWndProp::NotifyClosed()
{
	std::vector<SeriesProp *>::iterator iterSeries;
	for (iterSeries = this->seriesInPlotWndProp.begin();
		iterSeries != this->seriesInPlotWndProp.end();
		iterSeries++ )
	{
		SeriesProp * seriesProp = *iterSeries;
		seriesProp->NotifyClosed();
		//seriesProp->DecRefCount();	//TODO
		seriesProp->SetPlotWndProp(0);
	}
	//this->series.clear();

	std::vector<PlotWndProp *>::iterator iter;
	PlotWndAreaProp * plotWndAreaProp = this->plotWndAreaProp;
	for (iter = plotWndAreaProp->plotWndsInPlotWndAreaProp.begin();
		iter != plotWndAreaProp->plotWndsInPlotWndAreaProp.end();
		iter++)
	{
		if ( (*iter) == this )
		{
			::WaitForSingleObject(hMutexTree, INFINITE);
			plotWndAreaProp->plotWndsInPlotWndAreaProp.erase(iter);
			::ReleaseMutex(hMutexTree);
			break;
		}
	}
}

CPlotWnd * PlotWndProp::GetPlotWnd(CWnd * parent)
{
	CPlotWnd * plotWnd = this->CreatePlotWnd(parent);
	this->AddPlotWndStrategy(plotWnd);



	plotWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, this->rect, parent, 0, this);
	this->targetWnd = plotWnd;

	return plotWnd;
}

void PlotWndProp::AddPlotWndStrategy(CPlotWnd * plotWnd)
{
	plotWnd->StrategyGetGridSize.Set([this](const void * sender, CPlotWnd::NullParam, int & gridSize){
		gridSize = this->GetPlotWndArea()->GetGridSize();
	});

	plotWnd->StrategyGetCaption.Set([this](const void * sender, CPlotWnd::NullParam, CString & str){
		if (this->nameIsSet)
		{
			str = this->GetName();
		}
		else
		{
			CString name;

			if (this->seriesInPlotWndProp.size() > 0)
			{
				DebuggingProcessProp * process = this->seriesInPlotWndProp[0]->GetProcess();
				name.Format(L"%s(%d):", process->moduleName, process->pid);
			}
			std::for_each(this->seriesInPlotWndProp.begin(), this->seriesInPlotWndProp.end(), [&name] (SeriesProp * series) {
				name.Append(L"[");
				name.Append(series->name);
				name.Append(L"]");
			});

			str = name;
		}
	});

	plotWnd->EventBringToFront.Subscribe([this](const void * sender, const CPlotWnd::BringToFrontEvent & e) {
		this->UpdatePropertyPage();
	});

	plotWnd->EventMove.Subscribe([this](const void * sender, const CPlotWnd::MoveEvent & e){
		this->rect = e.rect;
	});

	plotWnd->EventClosed.Subscribe([this](const void * sender, const CPlotWnd::NullEvent & e){
		this->NotifyClosed();
		//delete this->targetWnd;
		this->targetWnd = 0;
		delete this;
	});

	plotWnd->StragegyTestTarget.Set([this](const void * sender, const CPlotWnd::TestTargetParam & p, bool & ret){
		auto series = (SeriesProp *)p.obj;
		ret = this->Accept(series);
	});
}

void PlotWndProp::EnableDataUpdate()
{
	_isDataUpdateEnabled = true;

	for_each(this->seriesInPlotWndProp.begin(),
		this->seriesInPlotWndProp.end(), [](SeriesProp * series){
			series->EnableWrite(true);
	});
}

void PlotWndProp::DisableDataUpdate()
{
	_isDataUpdateEnabled = false;

	for_each(this->seriesInPlotWndProp.begin(),
		this->seriesInPlotWndProp.end(), [](SeriesProp * series){
			series->EnableWrite(false);
	});
}

bool PlotWndProp::IsDataUpdateEnabled()
{
	return _isDataUpdateEnabled;
}

Bitmap * PlotWndProp::CreateBitmap()
{
	return 0;
}

void PlotWndProp::PlotGraphToScreen()
{
	this->ForceUpdateGraph();
}

void PlotWndProp::ForceUpdateGraph()
{
	_taskQueue.QueueTask([this](){
		this->UpdatePlotWnd();
	});
}

