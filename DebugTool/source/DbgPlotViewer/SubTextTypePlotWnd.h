#pragma once

#include "Strategy.h"
// SubTextTypePlotWnd

class SubTextTypePlotWnd : public CListCtrl
{
	DECLARE_DYNAMIC(SubTextTypePlotWnd)

public:
	SubTextTypePlotWnd();
	virtual ~SubTextTypePlotWnd();

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);

public:
	SoraDbgPlot::Strategy::Strategy<size_t, char *> StrategyGetText;
	virtual void PostNcDestroy();

public:
	void SetColor(COLORREF color);
private:
	COLORREF _color;
public:
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
};


