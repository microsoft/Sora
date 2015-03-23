#include "stdafx.h"
#include "ChannelExplorer.h"
#include "Resource.h"
#include "Logger.h"
#include "Debug.h"

CChannelExplorerWnd::CChannelExplorerWnd()
{
}

CChannelExplorerWnd::~CChannelExplorerWnd()
{
}

BEGIN_MESSAGE_MAP(CChannelExplorerWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CAPTURECHANGED()
	ON_MESSAGE(WM_APP, OnApp)
END_MESSAGE_MAP()


int CChannelExplorerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	m_channelViewImages.Create(IDB_CHANNEL, 16, 0, RGB(255, 0, 255));
	DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS;
	if (!m_channelView.Create(dwViewStyle, rectDummy, this, TREECTRL_ID))
	{
		return -1;
	}
	m_channelView.SetImageList(&m_channelViewImages, TVSIL_NORMAL);
	m_channelView.BringWindowToTop();

	AdjustLayout();

	return 0;
}

void CChannelExplorerWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CChannelExplorerWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	m_channelView.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CChannelExplorerWnd::OnCaptureChanged(CWnd *pWnd)
{
	// TODO: Add your message handler code here
	CString str("wnd");
	HWND wnd = pWnd->GetSafeHwnd();

	HighlightWnd(wnd);

	Logger::Print(L"capture changed, %s", str.GetBuffer());
	CDockablePane::OnCaptureChanged(pWnd);
}

LRESULT CChannelExplorerWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case CMD_NEW_DOCUMENT:
		this->doc = (CDebugPlotViewerDoc *)lParam;
		m_channelView.PostMessage(WM_APP, wParam, lParam);
		//m_channelView.PostMessage(WM_APP, wParam, lParam);
		return 0;
	case CMD_UPDATE_TREE_TIMER:
		m_channelView.SendMessage(WM_APP, wParam, lParam);
		return 0;
	}
	return 0;
}

BOOL CChannelExplorerWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= WS_CLIPCHILDREN;

	return CDockablePane::PreCreateWindow(cs);
}
