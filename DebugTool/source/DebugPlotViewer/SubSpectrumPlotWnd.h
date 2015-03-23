#pragma once

#include <map>

#include "DebugPlotViewerDoc.h"
#include "AppType.h"

// SubSpectrumPlotWnd

class SubSpectrumPlotWnd : public CWnd
{
	DECLARE_DYNAMIC(SubSpectrumPlotWnd)

public:
	SubSpectrumPlotWnd();
	virtual ~SubSpectrumPlotWnd();

protected:
	DECLARE_MESSAGE_MAP()

	//SeriesProp * seriesProp;
	CDebugPlotViewerDoc * doc;
	afx_msg void OnPaint();
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);


	//std::map<void *, NewData *> dataMap;

public:
	PlotWndPropSpectrum * plotWndProp;
	std::vector<SeriesProp *> seriesProps;
	void DrawData(Graphics * g, int * data, int len, Color& color);
	void DrawData(Graphics * g, SpectrumSeriesProp * prop, bool testAutoScaleOnly);
	void DrawDataFrame(Graphics * g, int totalFrame, int frameIndex, int frameDispIndex, SpectrumSeriesProp * prop);
	void DrawXAxis(Graphics * g);
	void DrawGrid(Graphics * g);
	void DrawGridLine(Graphics * g, __int64 yData);
	void DrawBackground(Graphics * g);

	Gdiplus::REAL GetClientY(double y);
	int GetClientX(int x, int count);
	int GetClientX(int x, int count, int frameIdx, int frameCount);
	int CalcOptimizedGridSize();

	int * dataBuf;
	int dataBufLen;

	virtual void SetPlotWndProp(PlotWndProp * prop);

	//void SetData(NewData * dataInfo);
	//void RemoveData(void * sender);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void PostNcDestroy();

private:
	Bitmap * bmp;
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


