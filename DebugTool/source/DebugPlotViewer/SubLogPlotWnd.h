#pragma once

#include "SeriesObj.h"
#include "PlotWndPropLog.h"

// SubLogPlotWnd

class SubLogPlotWnd : public CListCtrl
{
	DECLARE_DYNAMIC(SubLogPlotWnd)

public:
	SubLogPlotWnd();
	virtual ~SubLogPlotWnd();

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnApp(WPARAM wParam, LPARAM lParam);
private:
	void OnDataUpdate(void);

public:
	PlotWndPropLog * plotWndProp;
	virtual void PostNcDestroy();
};



