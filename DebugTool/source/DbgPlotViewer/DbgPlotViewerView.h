
// DbgPlotViewerView.h : interface of the CDbgPlotViewerView class
//

#pragma once


#include "PlotWndContainer.h"

class CDbgPlotViewerView : public CView
{
protected: // create from serialization only
	CDbgPlotViewerView();
	DECLARE_DYNCREATE(CDbgPlotViewerView)

// Attributes
public:
	CDbgPlotViewerDoc* GetDocument() const;

// Operations
public:

private:
	CWnd * _wnd;

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
	virtual ~CDbgPlotViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // debug version in DbgPlotViewerView.cpp
inline CDbgPlotViewerDoc* CDbgPlotViewerView::GetDocument() const
   { return reinterpret_cast<CDbgPlotViewerDoc*>(m_pDocument); }
#endif

