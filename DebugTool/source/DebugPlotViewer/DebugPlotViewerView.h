
// DebugPlotViewerView.h : interface of the CDebugPlotViewerView class
//

#pragma once

#include "PlotWnd.h"
#include "PlotWndArea.h"

class CDebugPlotViewerView : public CView
{
protected: // create from serialization only
	CDebugPlotViewerView();
	DECLARE_DYNCREATE(CDebugPlotViewerView)

// Attributes
public:
	CDebugPlotViewerDoc* GetDocument() const;

// Operations

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CDebugPlotViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	PlotWndArea plotWndArea;

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlag, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg LRESULT OnApp(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

public:
	void ResetView();
};

#ifndef _DEBUG  // debug version in DebugPlotViewerView.cpp
inline CDebugPlotViewerDoc* CDebugPlotViewerView::GetDocument() const
   { return reinterpret_cast<CDebugPlotViewerDoc*>(m_pDocument); }
#endif

