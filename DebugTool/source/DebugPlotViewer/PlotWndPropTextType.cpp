#include "stdafx.h"
#include "PlotWndPropTextType.h"
#include "HelperFunc.h"
#include "TextTypePlotWnd.h"

/***********************************

Text Plot Wnd

************************************/


PlotWndPropTextType::PlotWndPropTextType()
{
	_subPlotWnd = 0;
	_isPlaying = true;
	_latestIdx = 0;

	this->EventNameChanged.Subscribe([=](const void * sender, const CString & name){
		if (_subPlotWnd)
			_subPlotWnd->Invalidate();
	});
}

CPlotWnd * PlotWndPropTextType::CreatePlotWnd(CWnd * parent)
{
	_isPlaying = true;

	if (_subPlotWnd)
		delete _subPlotWnd;

	_subPlotWnd = new TextTypePlotWnd;

	_subPlotWnd->EventPlayClicked.Subscribe([this](const void * sender, const TextTypePlotWnd::PlayPauseEvent & e) {

		if (e.IsPlay)
		{
			this->_isPlaying = true;
			this->_subPlotWnd->SetButtonStatePause();
		}
		else
		{
			this->_isPlaying = false;
			this->_subPlotWnd->SetButtonStatePlay();
			this->DisableDataUpdate();
		}
	});

	_subPlotWnd->StrategyGetText.Set([this](const void * sender, size_t index, char * & str){
		char * text = this->GetText(index);
		str = text;
	});

	return _subPlotWnd;
}

void PlotWndPropTextType::PlayAFrame()
{
	if (!this->_isPlaying)
		return;

	_latestIdx = 0;
	this->EnableDataUpdate();
}

void PlotWndPropTextType::UpdatePlotWnd()
{
	::WaitForSingleObject(this->hMutexTree, INFINITE);

	if (this->IsDataUpdateEnabled())
	{
		for_each(this->seriesInPlotWndProp.begin(), this->seriesInPlotWndProp.end(), [](SeriesProp * series){

			series->UpdateFromDataQueue();
		});
	}

	this->PlayAFrame();

	if ( (this->seriesInPlotWndProp.size() > 0) )
	{
		auto seriesTextType = dynamic_cast<TextTypeSeriesProp *>(this->seriesInPlotWndProp[0]);

		_subPlotWnd->Invoke([this, seriesTextType](){
			seriesTextType->LockData();
			size_t size = seriesTextType->DataSize();
			_subPlotWnd->UpdateView(size);
			seriesTextType->UnlockData();
		});
	}

	::ReleaseMutex(this->hMutexTree);
}

bool PlotWndPropTextType::Accept(SeriesProp * series)
{
	return false;
}


void PlotWndPropTextType::PlayPauseProcess(bool bPlay)
{
	if (bPlay)
	{
		this->_isPlaying = true;
		this->_subPlotWnd->SetButtonStatePause();
	}
	else
	{
		this->_isPlaying = false;
		this->_subPlotWnd->SetButtonStatePlay();
		this->DisableDataUpdate();
	}
}
