#pragma once

#include "PlotWindowOpened.h"
#include "TaskQueue.h"
#include "Event.h"

class PlotWindowOpenedBitmapType : public PlotWindowOpened
{
public:
	PlotWindowOpenedBitmapType(std::shared_ptr<ProcessOpened>);
	~PlotWindowOpenedBitmapType();

	SoraDbgPlot::Event::Event<double> EventSeek;
	SoraDbgPlot::Event::Event<bool> EventWheel;

	struct RangeChangedEvent
	{
		size_t Index;
		size_t Range;
	};

	SoraDbgPlot::Event::Event<RangeChangedEvent> EventRangeChanged;
	SoraDbgPlot::Event::Event<double> EventSpeedChanged;
	SoraDbgPlot::Event::Event<std::wstring> EventXLabelChanged;
	SoraDbgPlot::Event::Event<std::wstring> EventYLableChanged;
	SoraDbgPlot::Event::Event<bool> EventShowGrid;
	SoraDbgPlot::Event::Event<bool> EventAutoScale;

	void SetPlayRange(size_t latestIdx, size_t range);
	void SetPlaySpeed(double playSpeed);
	void SetXLabel(const std::wstring &);
	void SetYLabel(const std::wstring &);
	void SetShowGrid(bool);
	void SetAutoScale(bool);
	void Seek(double pos);
	void SetClientRect(const CRect & rect);
	void ChangeSpeed(bool isUp);
	void ZoomInOut(bool isUp);
	void SetLog(bool isLog);
	void SetDataCount(size_t dataCount);

protected:
	size_t _latestIdx;
	size_t _range;
	double _playSpeed;
	double _playSpeedAccumulate;
	std::wstring _xLabel;
	std::wstring _yLabel;
	bool _bShowGrid;
	bool _bAutoScale;
	CRect _bitmapRect;
	bool _isLog;
	double _seekPos;
	bool _seekFlag;

protected:
	virtual void SeekToLatest();
	virtual void PlayPauseToLatest(bool);
	virtual void OnChannelAdded(std::shared_ptr<ChannelOpened>, const CRect &);
	virtual void OnChannelClosed(std::shared_ptr<ChannelOpened>);
	virtual CPlotWnd * CreatePlotWnd();
	void DrawBackground(Graphics * g, const CRect & clientRect);
	void DrawLabel(Graphics * g, const CRect & clientRect);
	void DrawXAxis(Graphics * g, const CRect & rectClient, size_t rangeDisp);	
	CRect GetBitmapRect();
	virtual HRESULT AppendXmlProperty(IXMLDOMDocument *pDom, IXMLDOMElement *pParent);
	virtual HRESULT LoadXmlElement(MSXML2::IXMLDOMNodePtr pElement);
	virtual void OnCoordinateChanged();
	virtual void OnMouseWheel(bool bIsUp);

	void UpdateTrackBarRange();
	size_t MaxDataSize();
	virtual void ModifyDataRange(size_t & range);
	virtual void OnProcessAttatched();

private:
	static const int LABEL_HEIGHT = 20;
	//CPlotWnd * _plotWnd;
};
