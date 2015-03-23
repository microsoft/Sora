#pragma once

#include "PlotWndProp.h"
#include "PlotWnd.h"

class TextTypePlotWnd;

class PlotWndPropTextType : public PlotWndProp
{
public:
	PlotWndPropTextType();
protected:
	CPlotWnd * CreatePlotWnd(CWnd * parent);
private:
	TextTypePlotWnd * _subPlotWnd;

private:
	void PlayAFrame();
	bool _isPlaying;
	size_t _latestIdx;

protected:
	virtual void UpdatePlotWnd();

protected:
	virtual char * GetText(size_t index) = 0;

public:
	virtual bool Accept(SeriesProp * series);

public:
	virtual void PlayPauseProcess(bool bPlay);
};

