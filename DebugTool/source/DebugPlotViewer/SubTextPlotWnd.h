#pragma once

#include "AppMessage.h"
#include "SeriesObj.h"
#include "PlotWndPropText.h"

// SubTextPlotWnd

class SubTextPlotWnd : public CRichEditCtrl
{
	DECLARE_DYNAMIC(SubTextPlotWnd)

public:
	SubTextPlotWnd();
	virtual ~SubTextPlotWnd();
	bool AcceptSeries();
	void SetReplaceMode(bool isReplace);
	PlotWndPropText * plotWndProp;

protected:
	DECLARE_MESSAGE_MAP()
	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	void SetColor(COLORREF color);
	void SetRichEditData(const wchar_t * string, bool replace, COLORREF color);
	void SetData(StringData * dataInfo);
	void RemoveData(void * sender);

private:
	//TextSeriesProp * seriesProp;
	//void * sender;
	virtual void PostNcDestroy();
	CString lastData;
	COLORREF lastColor;
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


