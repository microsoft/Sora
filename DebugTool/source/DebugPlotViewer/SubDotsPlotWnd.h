#pragma once

#include "DebugPlotViewerDoc.h"
#include "AppType.h"

// SubDotsPlotWnd

class SubDotsPlotWnd : public CWnd
{
	DECLARE_DYNAMIC(SubDotsPlotWnd)

public:
	SubDotsPlotWnd();
	virtual ~SubDotsPlotWnd();

protected:
	DECLARE_MESSAGE_MAP()

	SeriesProp * seriesProp;
	CDebugPlotViewerDoc * doc;
	afx_msg void OnPaint();
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);

public:
	void DrawGrid(Graphics * g);
	void DrawBackground(Graphics * g);
	void DrawData(Graphics * g, COMPLEX16 * data, int len, Color& color);
	void DrawData(Graphics * g, DotsSeriesProp * seriesProp, bool testAutoScaleOnly);
	COMPLEX16 GetClientDot(COMPLEX16 data);
	void DrawDot(Graphics * g, COMPLEX16 data, Color& color);

protected:
	//std::map<void *, NewData *> dataMap;

public:
	PlotWndPropDots * plotWndProp;
	std::vector<SeriesProp *> seriesProps;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void SetPlotWndProp(PlotWndProp * prop);
	//void SetData(NewData * dataInfo);
	//void RemoveData(void * sender);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void PostNcDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:

	Bitmap * bmp;
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


