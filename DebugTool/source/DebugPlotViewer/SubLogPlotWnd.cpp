// SubLogPlotWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SubLogPlotWnd.h"
#include "AppMessage.h"
#include "SeriesLog.h"

// SubLogPlotWnd

IMPLEMENT_DYNAMIC(SubLogPlotWnd, CListCtrl)

SubLogPlotWnd::SubLogPlotWnd() :
	plotWndProp(0)
{
}

SubLogPlotWnd::~SubLogPlotWnd()
{
}


BEGIN_MESSAGE_MAP(SubLogPlotWnd, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &SubLogPlotWnd::OnLvnGetdispinfo)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &SubLogPlotWnd::OnNMCustomdraw)
	ON_MESSAGE(WM_APP, OnApp)
END_MESSAGE_MAP()



// SubLogPlotWnd message handlers


BOOL SubLogPlotWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= LVS_OWNERDATA | LVS_NOCOLUMNHEADER | LVS_REPORT;

	return CListCtrl::PreCreateWindow(cs);
}

void SubLogPlotWnd::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

    //Create a pointer to the item
    LV_ITEM* pItem= &(pDispInfo)->item;

    //Which item number?
    int itemid = pItem->iItem;

    //Do the list need text information?
    if (pItem->mask & LVIF_TEXT)
    {
        CString text;

        if(pItem->iSubItem == 0)
        {
			ASSERT(this->plotWndProp);
			ASSERT(this->plotWndProp->seriesInPlotWndProp.size() == 1);
			LogSeriesProp * seriesProp = (LogSeriesProp *)this->plotWndProp->seriesInPlotWndProp[0];
			char * strRecord = seriesProp->Record(itemid);
			text = strRecord;
			lstrcpyn(pItem->pszText, text, pItem->cchTextMax);
			delete [] strRecord;
        }
    }

    *pResult = 0;

}

int SubLogPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	this->InsertColumn(0, L"log", LVCFMT_LEFT);
	this->SetColumnWidth(0, LVSCW_AUTOSIZE);
	this->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_DOUBLEBUFFER | LVS_EX_FLATSB);
	this->SetBkColor(RGB(0, 0, 0));

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_REGISTER_WND, (LPARAM)this);

	return 0;
}


void SubLogPlotWnd::OnSize(UINT nType, int cx, int cy)
{
	CListCtrl::OnSize(nType, cx, cy);

	this->SetColumnWidth(0, cx);
	this->Invalidate(1);
}


void SubLogPlotWnd::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here

	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

	*pResult = CDRF_DODEFAULT;

	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		pLVCD->clrText = RGB(0, 255, 0);
		pLVCD->clrTextBk = RGB(0, 0, 0);
		*pResult = CDRF_DODEFAULT;
	}
}

LRESULT SubLogPlotWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_SERIES_CLOSED:
		return 0;
	case CMD_DATA_CLEAR:
		//ASSERT(FALSE);
		;
		return 0;
	case CMD_QUERY_TYPE:
		this->GetParent()->SendMessage(WM_APP, wParam, lParam);
		return 0;
	case CMD_SET_REPLACE_MODE:
		ASSERT(FALSE);
		return 0;
	//case CMD_UPDATE_VIEW:
	//	this->OnDataUpdate();
	//	return 0;
	}
	return 0;
}

void SubLogPlotWnd::OnDataUpdate(void)
{
	ASSERT(this->plotWndProp->seriesInPlotWndProp.size() == 1);

	LogSeriesProp * seriesProp = (LogSeriesProp *)this->plotWndProp->seriesInPlotWndProp[0];

	int itemCount;
	
	seriesProp->Lock();
	itemCount = seriesProp->RecordCount();
	seriesProp->Unlock();

	this->SetItemCount(itemCount);

	this->EnsureVisible(this->GetItemCount() - 1, FALSE);
}

void SubLogPlotWnd::PostNcDestroy()
{
	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_UNREGISTER_WND, (LPARAM)this);
	delete this;

	CListCtrl::PostNcDestroy();
}
