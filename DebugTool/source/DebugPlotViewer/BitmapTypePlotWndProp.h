#pragma once

#include "PlotWndProp.h"

class BitmapPlotWnd;

class BitmapTypePlotWndProp : public PlotWndProp
{
private:
	BitmapPlotWnd * _subPlotWnd;

public:
	BitmapTypePlotWndProp();
	virtual ~BitmapTypePlotWndProp();
	virtual HRESULT CreateElementPlot(IXMLDOMDocument *pDom,IXMLDOMElement *pe) = 0;
	virtual BaseProperty * GetPropertyPage() = 0;
	virtual Bitmap * CreateBitmap() = 0;
	
	void UpdateTrackBarRange();
	void SetDataRange(size_t range);
	size_t GetRangeSize();

protected:
	virtual CPlotWnd * CreatePlotWnd(CWnd * parent);

private:
	void SeekTextWnd();

protected:
	virtual size_t MaxDataSize() = 0;

public:
	size_t LatestIdx();
	size_t RangeSize();

private:
	void PlayAFrame();
	bool _isPlaying;

	size_t _playSpeed;
	size_t _latestIdx;
	size_t _rangeSize;
public:
	void Seek(double pos);
	void SpeedUpDown(bool isUp);
	virtual void ModifyDataRange(size_t & range);
	virtual void ModifyTrackBarDispRange(double & range);
private:
	void ZoomInOut(bool isIn);

public:
	int ClientWidth();
	int ClientHeight();

private:
	int _clientWidth;
	int _clientHeight;

// update Task
protected:
	virtual void UpdatePlotWnd();

public:
	void DrawXAxis(Graphics * g, const CRect & rectClient);

protected:
	virtual void ModifySeekedPos(size_t & idx);

public:
	void AddTextPlotWnd(TextSeriesProp * series);

public:
	void XLabel(const CString & label);
	void YLabel(const CString & label);
	CString GetXLabel();
	CString GetYLabel();
private:
	CString _xLabel;
	CString _yLabel;
	SoraDbgPlot::Lock::CSLock _lockYLabel;

protected:
	void GetCanvasRect(CRect & out);
	void DrawLabel(Graphics * g);

private:
	static const int LABEL_HEIGHT = 20;

public:
	virtual void PlayPauseProcess(bool bPlay);
};


