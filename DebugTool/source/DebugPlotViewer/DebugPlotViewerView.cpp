
// DebugPlotViewerView.cpp : implementation of the CDebugPlotViewerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DebugPlotViewer.h"
#endif

#include "DebugPlotViewerDoc.h"
#include "DebugPlotViewerView.h"

#include <algorithm>
#include "Logger.h"
#include "AppMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDebugPlotViewerView

IMPLEMENT_DYNCREATE(CDebugPlotViewerView, CView)

BEGIN_MESSAGE_MAP(CDebugPlotViewerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CDebugPlotViewerView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()

	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()

	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CDebugPlotViewerView construction/destruction

CDebugPlotViewerView::CDebugPlotViewerView()
{
	// TODO: add construction code here
}

CDebugPlotViewerView::~CDebugPlotViewerView()
{

}

BOOL CDebugPlotViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	//cs.style &=~(WS_THICKFRAME | WS_BORDER);

	return CView::PreCreateWindow(cs);
}

// CDebugPlotViewerView drawing

void CDebugPlotViewerView::OnDraw(CDC* /*pDC*/)
{
	CDebugPlotViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CDebugPlotViewerView printing


void CDebugPlotViewerView::OnFilePrintPreview()
{
//#ifndef SHARED_HANDLERS
//	AFXPrintPreview(this);
//#endif
}

BOOL CDebugPlotViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CDebugPlotViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CDebugPlotViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CDebugPlotViewerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CDebugPlotViewerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CDebugPlotViewerView diagnostics

#ifdef _DEBUG
void CDebugPlotViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CDebugPlotViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDebugPlotViewerDoc* CDebugPlotViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDebugPlotViewerDoc)));
	return (CDebugPlotViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CDebugPlotViewerView message handlers


int CDebugPlotViewerView::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CView::OnCreate(lpcs) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	plotWndArea.CreateEx(0, NULL, L"PlotWndArea", WS_CHILD | WS_VISIBLE, rectDummy, this, 0, NULL);

	return 0;
}

void CDebugPlotViewerView::OnMouseMove(UINT nFlag, CPoint point)
{
	CView::OnMouseMove(nFlag, point);
}

void CDebugPlotViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CView::OnLButtonUp(nFlags, point);
}


LRESULT CDebugPlotViewerView::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_PLAY_PAUSE_ALL:
		{
			for (auto iter = plotWndArea.plotWndAreaProp->plotWndsInPlotWndAreaProp.begin();
				iter != plotWndArea.plotWndAreaProp->plotWndsInPlotWndAreaProp.end();
				iter++)
			{
				PlotWndProp * plotWndProp = *iter;
				plotWndProp->ForceUpdateGraph();
				if (plotWndProp->seriesInPlotWndProp.size() > 0)
				{
					DebuggingProcessProp * process = (DebuggingProcessProp *)lParam;
					if (plotWndProp->seriesInPlotWndProp[0]->GetProcess() == process)
						plotWndProp->PlayPauseProcess(!process->ReplayIsPaused());
				}
			}
			return 0;
		}
	case CMD_NEW_DOCUMENT:
		{
		this->plotWndArea.PostMessage(WM_APP, wParam, lParam);
		//this->GetDocument()->AddObserver(&this->plotWndArea);
		CDebugPlotViewerDoc * doc = (CDebugPlotViewerDoc *)lParam;
		plotWndArea.plotWndAreaProp = &doc->plotWndAreaProp;
		doc->plotWndAreaProp.targetWnd = &this->plotWndArea;
		return 0;
		}
	case CMD_GET_PLOT_WND_AREA_WND:
		{
		CWnd * wnd = &plotWndArea;
		CWnd ** ppWnd = (CWnd **)lParam;
		*ppWnd = &plotWndArea;
		return 0;
		}
	case CMD_UPDATE_GRAPH_TIMER:
		this->plotWndArea.plotWndAreaProp->PlotGraphToScreen();
		return 0;
	case CMD_RESET_VIEW:
		ResetView();
		return 0;
	case CMD_UPDATE_PLOT_WND:
		{
			vector<PlotWndProp *>::iterator iter;
			for (iter = plotWndArea.plotWndAreaProp->plotWndsInPlotWndAreaProp.begin();
				iter != plotWndArea.plotWndAreaProp->plotWndsInPlotWndAreaProp.end();
				iter++)
			{
				PlotWndProp * plotWndProp = *iter;
				plotWndProp->ForceUpdateGraph();
			}
		}
		return 0;
	case CMD_AUTO_LAYOUT:
		this->plotWndArea.PostMessage(WM_APP, wParam, lParam);
		return 0;
	default:
		return 0;
	}	
}

void CDebugPlotViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CRect clientRect;
	GetClientRect(&clientRect);
	this->plotWndArea.MoveWindow(clientRect, 1);
}

void CDebugPlotViewerView::ResetView()
{
			std::vector<PlotWndProp *> plotWnds;

			std::vector<PlotWndProp *>::iterator iter;

			for (iter = this->plotWndArea.plotWndAreaProp->plotWndsInPlotWndAreaProp.begin();
				iter != this->plotWndArea.plotWndAreaProp->plotWndsInPlotWndAreaProp.end();
				iter++)
			{
				plotWnds.push_back(*iter);
			}

			for (iter = plotWnds.begin();
				iter != plotWnds.end();
				iter++)
			{
				(*iter)->targetWnd->SendMessage(WM_CLOSE, 0, 0);
			}

			ASSERT(this->plotWndArea.plotWndAreaProp->plotWndsInPlotWndAreaProp.size() == 0);	
}
