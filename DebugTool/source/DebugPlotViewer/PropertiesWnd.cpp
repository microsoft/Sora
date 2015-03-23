
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "DebugPlotViewer.h"
#include "CustomProperties.h"
#include "PlotWndArea.h"

//#include "DebugPlotViewerDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
	currentPropertyGrid = 0;
}

CPropertiesWnd::~CPropertiesWnd()
{
	if (currentPropertyGrid)
		delete currentPropertyGrid;
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	//int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	//m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	if (this->currentPropertyGrid)
		this->currentPropertyGrid->SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

ObjProp * CPropertiesWnd::GetCurrentObj()
{
	if (!this->currentPropertyGrid)
		return 0;

	return (ObjProp *)this->currentPropertyGrid->GetTarget();
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	//if (!plotWndAreaProperty.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	//{
	//	TRACE0("Failed to create Properties Grid \n");
	//	return -1;      // fail to create
	//}

	//currentPropertyGrid = &plotWndAreaProperty;

	InitPropList();

	AdjustLayout();

	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	if (this->currentPropertyGrid)
		this->currentPropertyGrid->ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	if (this->currentPropertyGrid)
		this->currentPropertyGrid->SetAlphabeticMode(!this->currentPropertyGrid->IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	if (this->currentPropertyGrid)
		pCmdUI->SetCheck(this->currentPropertyGrid->IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::OnProperties2()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	//plotWndAreaProperty.EnableHeaderCtrl(FALSE);
	//plotWndAreaProperty.EnableDescriptionArea();
	//plotWndAreaProperty.SetVSDotNetLook();
	//plotWndAreaProperty.MarkModifiedProperties();


}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	if (this->currentPropertyGrid)
		this->currentPropertyGrid->SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	if (this->currentPropertyGrid)
		this->currentPropertyGrid->SetFont(&m_fntPropList);
}

LRESULT CPropertiesWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case CMD_CHANGE_PROPERTY:
		this->ChangeProperty((BaseProperty *)lParam);
		break;
	case CMD_CHANGE_PROPERTY_VALUE:
		if (((BaseProperty *)lParam)->GetTarget() == currentPropertyGrid->GetTarget())
			this->ChangeProperty((BaseProperty *)lParam);
		else
			delete (BaseProperty *)lParam;
		break;
	case CMD_UPDATE_PROPERTY_PANEL:
		if (this->currentPropertyGrid)
			this->currentPropertyGrid->PostMessage(WM_APP, wParam, lParam);
			//this->currentPropertyGrid->UpdatePropertyValue((wchar_t *)lParam);
		break;
	}
	return 0;
}

void CPropertiesWnd::ChangeProperty(BaseProperty * property)
{
	if (this->currentPropertyGrid)
	{
		this->currentPropertyGrid->SendMessage(WM_CLOSE);
		this->currentPropertyGrid->DestroyWindow();
		delete this->currentPropertyGrid;
	}

	this->currentPropertyGrid = property;

	if (this->currentPropertyGrid)
	{
		//this->currentPropertyGrid->SetTarget(obj);
		CRect rectDummy;
		rectDummy.SetRectEmpty();
		this->currentPropertyGrid->Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2);
		this->AdjustLayout();
	}

	this->InvalidateRgn(NULL, 1);
}


void CPropertiesWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDockablePane::OnPaint() for painting messages

	CRect rect;
	GetClientRect(&rect);

	CBrush brush(RGB(255, 255, 255));
	dc.FillRect(&rect, &brush);
}
