
// DbgPlotViewerView.cpp : implementation of the CDbgPlotViewerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DbgPlotViewer.h"
#endif

#include "DbgPlotViewerDoc.h"
#include "DbgPlotViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDbgPlotViewerView

IMPLEMENT_DYNCREATE(CDbgPlotViewerView, CView)

BEGIN_MESSAGE_MAP(CDbgPlotViewerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CDbgPlotViewerView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CDbgPlotViewerView construction/destruction

CDbgPlotViewerView::CDbgPlotViewerView()
{
	// TODO: add construction code here

}

CDbgPlotViewerView::~CDbgPlotViewerView()
{
}

BOOL CDbgPlotViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CDbgPlotViewerView drawing

void CDbgPlotViewerView::OnDraw(CDC* /*pDC*/)
{
	CDbgPlotViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CDbgPlotViewerView printing


void CDbgPlotViewerView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	// must be __cdecl?? ops~~
	//AFXPrintPreview(this);
#endif
}

BOOL CDbgPlotViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CDbgPlotViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CDbgPlotViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CDbgPlotViewerView diagnostics

#ifdef _DEBUG
void CDbgPlotViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CDbgPlotViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDbgPlotViewerDoc* CDbgPlotViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDbgPlotViewerDoc)));
	return (CDbgPlotViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CDbgPlotViewerView message handlers


int CDbgPlotViewerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	_wnd = theApp.plotWindowOpenedContainer->GetWnd();

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	_wnd->CreateEx(0, NULL, L"Plot Window Container", WS_CHILD | WS_VISIBLE, rectDummy, this, 0, NULL);

	return 0;
}


void CDbgPlotViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (_wnd)
		_wnd->MoveWindow(0, 0, cx, cy);
}
