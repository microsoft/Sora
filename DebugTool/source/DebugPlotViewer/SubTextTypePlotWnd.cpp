// SubTextTypePlotWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SubTextTypePlotWnd.h"
#include "AppMessage.h"

// SubTextTypePlotWnd

IMPLEMENT_DYNAMIC(SubTextTypePlotWnd, CListCtrl)

SubTextTypePlotWnd::SubTextTypePlotWnd()
{
	_color = RGB(0, 255, 0);
}

SubTextTypePlotWnd::~SubTextTypePlotWnd()
{
}


BEGIN_MESSAGE_MAP(SubTextTypePlotWnd, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &SubTextTypePlotWnd::OnLvnGetdispinfo)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &SubTextTypePlotWnd::OnNMCustomdraw)
END_MESSAGE_MAP()



// SubTextTypePlotWnd message handlers




BOOL SubTextTypePlotWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= LVS_OWNERDATA | LVS_NOCOLUMNHEADER | LVS_REPORT;

	return CListCtrl::PreCreateWindow(cs);
}


void SubTextTypePlotWnd::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			//ASSERT(this->plotWndProp);
			//ASSERT(this->plotWndProp->seriesInPlotWndProp.size() == 1);
			//LogSeriesProp * seriesProp = (LogSeriesProp *)this->plotWndProp->seriesInPlotWndProp[0];
			//char * strRecord = seriesProp->Record(itemid);
			//text = strRecord;
			//lstrcpyn(pItem->pszText, text, pItem->cchTextMax);
			//delete [] strRecord;

			char * text;
			bool succ = this->StrategyGetText.Call(this, itemid, text);
			if (succ)
			{
				if (text != 0)
				{
					CString str(text);
					lstrcpyn(pItem->pszText, str, pItem->cchTextMax);
					delete [] text;
				}
				else
				{
					lstrcpyn(pItem->pszText, L"", pItem->cchTextMax);
				}
			}
        }
    }

    *pResult = 0;
}

int SubTextTypePlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
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


void SubTextTypePlotWnd::OnSize(UINT nType, int cx, int cy)
{
	CListCtrl::OnSize(nType, cx, cy);

	this->SetColumnWidth(0, cx);
	this->Invalidate(1);
}


void SubTextTypePlotWnd::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
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
		pLVCD->clrText = _color;
		pLVCD->clrTextBk = RGB(0, 0, 0);
		*pResult = CDRF_DODEFAULT;
	}
}


void SubTextTypePlotWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_UNREGISTER_WND, (LPARAM)this);

	CListCtrl::PostNcDestroy();
}

void SubTextTypePlotWnd::SetColor(COLORREF color)
{
	_color = color;
}
