
#include "stdafx.h"
#include "ViewTree.h"
#include "Logger.h"
#include "AppMessage.h"
#include "PlotWnd.h"
#include "PlotWndArea.h"
#include "SharedStruct.h"
#include "SeriesObj.h"
#include "SeriesLine.h"
#include "SeriesText.h"
#include "SeriesDots.h"
#include "SeriesLog.h"
#include "SeriesSpectrum.h"
#include "PlotWndPropLine.h"
#include "PlotWndPropDots.h"
#include "PlotWndPropSpectrum.h"
#include "SharedObjManager.h"
#include "Debug.h"
#include "Resource.h"
#include "DebugPlotViewer.h"
#include "TargetTestable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewTree

CViewTree::CViewTree()
{
	this->isDragging = false;
	this->lastFoundWnd = 0;
	this->_lastTestable = 0;
}

CViewTree::~CViewTree()
{
}

BEGIN_MESSAGE_MAP(CViewTree, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CViewTree::OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_CLOSE_CHANNEL, OnCloseChannel)
	ON_COMMAND(ID_CHANNEL_SAVESELECTION, OnChannelSaveSelection)
	ON_COMMAND(ID_CHANNEL_SAVEALL, OnChannelSaveAll)
	ON_COMMAND(ID_CHANNEL_CLOSE, OnTextChannelClose)
	ON_COMMAND(ID_CHANNEL_EXPORT, OnTextChannelExport)
	ON_MESSAGE(WM_APP, OnApp)
	ON_NOTIFY_REFLECT(NM_CLICK, &CViewTree::OnNMClick)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_RCLICK, &CViewTree::OnNMRClick)
	ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewTree message handlers

BOOL CViewTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != NULL);

	if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL)
	{
		GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}


void CViewTree::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	ObjProp * prop = (ObjProp *)pNMTreeView->itemNew.lParam;
	this->SelectItem(pNMTreeView->itemNew.hItem);

	BaseProperty * property = prop->GetPropertyPage();
	::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);

	//if (prop->GetObjType() == TYPE_SERIES)
	if (dynamic_cast<SeriesProp *>(prop) != 0)
	{
		SeriesProp * seriesProp = (SeriesProp *)prop;
		if (seriesProp->isActive)
		{
			this->dragSeriesProp = seriesProp;
			SetCapture();
			isDragging = true;
			lastFoundWnd = 0;		
		}
	}

	*pResult = 0;
}


void CViewTree::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if (isDragging)
	{
		//TargetWnd targetWnd;

		TargetTestable * testable = FindTarget();
		if (testable != _lastTestable)
		{
			if (_lastTestable)
			{
				_lastTestable->HighLight(false);
			}

			if (testable)
			{
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));
				testable->HighLight(true);
			}
			else
			{
				::SetCursor(::LoadCursor(NULL, IDC_NO));
			}

			_lastTestable = testable;
		}

		//bool isTarget = FindTargetWnd(&targetWnd);

		//if (targetWnd.wnd != lastFoundWnd)
		//{
		//	if (lastFoundWnd)
		//	{
		//		HighlightWindow(lastFoundWnd, false);
		//	}
		//	if (isTarget)
		//	{
		//		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		//		lastFoundWnd = targetWnd.wnd;
		//		HighlightWindow(targetWnd.wnd, true);
		//	}
		//	else
		//	{
		//		::SetCursor(::LoadCursor(NULL, IDC_NO));
		//		lastFoundWnd = 0;
		//	}
		//}
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}


void CViewTree::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if (isDragging)
	{	
		//TargetWnd targetWnd;

		TargetTestable * testable = this->FindTarget();

		if (testable)
		{
			testable->HighLight(false);
			dragSeriesProp->Close();
			PlotWndProp * plotWndProp = dragSeriesProp->GetPlotWndProp();
			if (plotWndProp)
				plotWndProp->RemoveSeries(dragSeriesProp);

			// set rect of series object
			CPoint screenCursorPoint;
			GetCursorPos(&screenCursorPoint);
			dragSeriesProp->rect.left = screenCursorPoint.x;
			dragSeriesProp->rect.top = screenCursorPoint.y;
			dragSeriesProp->rect.right = dragSeriesProp->rect.left + 40;
			dragSeriesProp->rect.bottom = dragSeriesProp->rect.top + 40;

			testable->AddToTarget(dragSeriesProp);
		}

		//if ( FindTargetWnd(&targetWnd) )
		//{
		//	HighlightWindow(targetWnd.wnd, false);
		//	dragSeriesProp->Close();
		//	PlotWndProp * plotWndProp = dragSeriesProp->GetPlotWndProp();
		//	if (plotWndProp)
		//		plotWndProp->RemoveSeries(dragSeriesProp);

		//	// Add plot wnd to plot wnd area
		//	if (targetWnd.type == WND_TYPE_VIEW)		// Plot Window Area
		//	{
		//		PlotWndProp * plotWndProp = 0;

		//		if (typeid(*dragSeriesProp) == typeid(LineSeriesProp))
		//			plotWndProp = new PlotWndPropLine();
		//		else if (typeid(*dragSeriesProp) == typeid(DotsSeriesProp))
		//			plotWndProp = new PlotWndPropDots();
		//		else if (typeid(*dragSeriesProp) == typeid(TextSeriesProp))
		//			plotWndProp = new PlotWndPropText();
		//		else if (typeid(*dragSeriesProp) == typeid(LogSeriesProp))
		//			plotWndProp = new PlotWndPropLog();
		//		else if (typeid(*dragSeriesProp) == typeid(SpectrumSeriesProp))
		//			plotWndProp = new PlotWndPropSpectrum();

		//		ASSERT(plotWndProp);

		//		// name
		//		plotWndProp->name.SetString(this->dragSeriesProp->name);

		//		// rect
		//		CPoint screenCursorPoint;
		//		GetCursorPos(&screenCursorPoint);
		//		CPoint clientPoint = screenCursorPoint;
		//		targetWnd.wnd->ScreenToClient(&clientPoint);
		//		plotWndProp->rect.left = clientPoint.x;
		//		plotWndProp->rect.top = clientPoint.y;
		//		plotWndProp->rect.right = plotWndProp->rect.left + doc->plotWndAreaProp.initialPlotWndWidth;
		//		plotWndProp->rect.bottom = plotWndProp->rect.top + doc->plotWndAreaProp.initialPlotWndHeight;

		//		dragSeriesProp->rect.SetRectEmpty();

		//		// add plot wnd to plot wnd area
		//		PlotWndAreaProp * plotWndAreaProp = ((PlotWndArea *)targetWnd.wnd)->GetProp();
		//		plotWndProp->SetPlotWndArea(plotWndAreaProp);											// 1
		//		plotWndAreaProp->AddPlotWnd(plotWndProp);												// 2
		//		// Create CWnd object
		//		targetWnd.wnd->SendMessage(WM_APP, CMD_NEW_PLOT_WND, (LPARAM)plotWndProp);				// 3
		//		
		//		// add series to plot wnd
		//		plotWndProp->AddSeries(dragSeriesProp);													// 7
		//		dragSeriesProp->SetPlotWndProp(plotWndProp);											// 8

		//		// Create CWnd object
		//		//plotWndProp->targetWnd->SendMessage(WM_APP, CMD_ADD_SERIES, (LPARAM)dragSeriesProp);	// 9

		//	}
		//	else if (targetWnd.type == WND_TYPE_PLOT)
		//	{
		//		CPlotWnd * plotWnd = (CPlotWnd *)targetWnd.wnd;
		//		
		//		// add series to plotwnd obj
		//		auto plotWndProp = (PlotWndProp *)plotWnd->GetUserData();
		//		plotWndProp->AddSeries(dragSeriesProp);
		//		dragSeriesProp->SetPlotWndProp(plotWndProp);
		//		
		//		// set rect of series object
		//		CPoint screenCursorPoint;
		//		GetCursorPos(&screenCursorPoint);
		//		dragSeriesProp->rect.left = screenCursorPoint.x;
		//		dragSeriesProp->rect.top = screenCursorPoint.y;
		//		dragSeriesProp->rect.right = dragSeriesProp->rect.left + 40;
		//		dragSeriesProp->rect.bottom = dragSeriesProp->rect.top + 40;
		//		//plotWnd->graph->ScreenToClient(&dragSeriesProp->rect);

		//		//plotWnd->SendMessage(WM_APP, CMD_ADD_SERIES, (LPARAM)dragSeriesProp);
		//	}
		//}

		ReleaseCapture();
		lastFoundWnd = 0;
		_lastTestable = 0;
		isDragging = false;
	}

	CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CViewTree::HighlightWindow(CWnd * wnd, bool highlight)
{
	wnd->SendMessage(WM_APP, CMD_HIGHLIGHT_WND, (LPARAM)highlight);
	wnd->InvalidateRgn(NULL);
	wnd->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
}

UINT __cdecl UpdateFrameData( LPVOID pParam )
{
	CViewTree * viewTree = (CViewTree *)pParam;

	return 0;
}

LRESULT CViewTree::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case CMD_NEW_DOCUMENT:
		this->doc = (CDebugPlotViewerDoc *)lParam;
		UpdateTree();
		return 0;

	case CMD_UPDATE_CHANNEL_TREE:
		this->BuildTree();
		return 0;
	case CMD_UPDATE_TREE_TIMER:
		if (!isDragging)
			this->UpdateTree();
		return 0;
	}

	return 0;
}

void CViewTree::BuildTree()
{
	SharedObjManager * sharedObjManager = SharedObjManager::Instance();
	if (!sharedObjManager)
		return;

	sharedObjManager->Lock();

	this->SetRedraw(FALSE);
	this->DeleteAllItems();

	std::multimap<wstring, DebuggingProcessProp *> * processes = &sharedObjManager->allProcessObjs;
	std::multimap<wstring, DebuggingProcessProp *>::iterator i;

	for (	i = processes->begin(); 
		i != processes->end();
		i++ )
	{
		DebuggingProcessProp * prop = (*i).second;
		UpdateProcessTree(prop);
	}

	sharedObjManager->Unlock();

	this->SetRedraw(TRUE);
}

void CViewTree::UpdateProcessTree(DebuggingProcessProp * processProp)
{

	CString str;
	str.Format(L"%s(%d)", processProp->moduleName, processProp->pid);

	int imgIdx = processProp->isActive ? 10 : 11;

	HTREEITEM hProcess = this->InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE| TVIF_SELECTEDIMAGE | TVIF_STATE, str.GetBuffer(), imgIdx, imgIdx, 0, 0, (LPARAM)processProp, 0, TVI_SORT);

	std::vector<SeriesProp *>::iterator si;
	for (si = processProp->series.begin(); si != processProp->series.end(); si++)
	{
		SeriesProp * seriesProp = *si;
		UpdateChannel(hProcess, seriesProp);
	}
	this->Expand(hProcess, TVE_EXPAND);
}

void CViewTree::UpdateChannel(HTREEITEM item, SeriesProp * seriesProp)
{
	CString str;
	str.Format(L"%s", seriesProp->name);

	int imgIdx = 0;
	if (typeid(*seriesProp) == typeid(LineSeriesProp))
		imgIdx = 0;
	else if (typeid(*seriesProp) == typeid(DotsSeriesProp))
		imgIdx = 2;
	else if (typeid(*seriesProp) == typeid(SpectrumSeriesProp))
		imgIdx = 4;
	else if (typeid(*seriesProp) == typeid(LogSeriesProp))
		imgIdx = 6;
	else if (typeid(*seriesProp) == typeid(TextSeriesProp))
		imgIdx = 8;

	if (!seriesProp->isActive)
		imgIdx++;

	HTREEITEM hItem = this->InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, str.GetBuffer(), imgIdx, imgIdx, 0, 0, (LPARAM)seriesProp, item, TVI_SORT);
	//if (!seriesProp->isActive)
	//	this->SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
}

bool CViewTree::FindTargetWnd(TargetWnd * targetWnd)
{
	if (targetWnd == 0)
		return false;

	CPoint screenCursorPoint;
	GetCursorPos(&screenCursorPoint);
	CWnd * wnd = WindowFromPoint(screenCursorPoint);

	DWORD processId;
	::GetWindowThreadProcessId(wnd->GetSafeHwnd(), &processId);
	if ( processId != ::GetProcessId(::GetCurrentProcess()) )
	{
		return false;
	}

	targetWnd->type = WND_TYPE_DEFAULT;
	targetWnd->wnd = wnd;

	wnd->SendMessage(WM_APP, CMD_QUERY_TYPE, (LPARAM)targetWnd);

	targetWnd->wnd = wnd;

	if ( targetWnd->type == WND_TYPE_VIEW )
	{
		return true;
	}
	else
	{
		auto testable = dynamic_cast<TargetTestable *>(wnd);
		if (testable)
		{
			return testable->TestTarget(dragSeriesProp);
		}
		else
			return false;
	}

	//else if ( targetWnd->type == WND_TYPE_PLOT )
	//{
	//	CPlotWnd * plotWnd = dynamic_cast<CPlotWnd *>(targetWnd->wnd);
	//	ASSERT(plotWnd);
	//	auto plotWndProp = (PlotWndProp *)plotWnd->GetUserData();
	//	return plotWndProp->Accept(dragSeriesProp);
	//	//return ((CPlotWnd *)targetWnd->wnd)->Accept(dragSeriesProp);
	//}
	//else
	//	return false;
}

TargetTestable * CViewTree::FindTarget()
{
	CPoint screenCursorPoint;
	GetCursorPos(&screenCursorPoint);
	CWnd * wnd = WindowFromPoint(screenCursorPoint);

	DWORD processId;
	::GetWindowThreadProcessId(wnd->GetSafeHwnd(), &processId);
	if ( processId != ::GetProcessId(::GetCurrentProcess()) )
	{
		return false;
	}

	auto testable = dynamic_cast<TargetTestable *>(wnd);
	if (testable == 0)
		return 0;

	if (testable->TestTarget(dragSeriesProp))
		return testable;

	return 0;
}

void CViewTree::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here

	DWORD dw = GetMessagePos(); // Mouse position 
	CPoint p(GET_X_LPARAM(dw), GET_Y_LPARAM(dw)); 
	this->ScreenToClient(&p); 

	UINT htFlags = 0; 
	HTREEITEM hHitItem= this->HitTest(p, &htFlags);

	if (htFlags & TVHT_ONITEM) 
	{
		ObjProp * obj = (ObjProp *)this->GetItemData(hHitItem);
		if (dynamic_cast<SeriesProp *>(obj) != 0)
			//if (obj->GetObjType() == TYPE_SERIES)
		{
			BaseProperty * property = obj->GetPropertyPage();
			::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);
		}
	}

	*pResult = 0;
}


void CViewTree::UpdateTree()
{

	SharedObjManager * manager = SharedObjManager::Instance();

	manager->LockAndUpdate();

	if (manager->IsChanged())
	{
		this->PostMessage(WM_APP, CMD_UPDATE_CHANNEL_TREE, 0);
		::AfxGetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_CONTROLLER, 0);
		::AfxGetMainWnd()->PostMessage(WM_APP, CMD_UPDATE_PLOT_WND, 0);
	}
}


BOOL CViewTree::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= WS_CLIPSIBLINGS;

	return CTreeCtrl::PreCreateWindow(cs);
}

void CViewTree::OnContextMenu(CWnd* pWnd/*pWnd*/, CPoint point/*point*/)
{
	// TODO: Add your message handler code here
	TextTypeSeriesProp * series = dynamic_cast<TextTypeSeriesProp *>(this->contextMenuProp);
	if (series)
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_TEXT_CHANNEL, point.x, point.y, this, TRUE);
	else
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_CHANNEL_EXPLORER, point.x, point.y, this, TRUE);	
}


void CViewTree::OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here

	DWORD dw = GetMessagePos(); // Mouse position 
	CPoint p(GET_X_LPARAM(dw), GET_Y_LPARAM(dw)); 
	this->ScreenToClient(&p); 

	UINT htFlags = 0; 
	HTREEITEM hHitItem= this->HitTest(p, &htFlags);

	if (htFlags & TVHT_ONITEM) 
	{
		HTREEITEM hItemOld = this->GetSelectedItem();
		this->SelectItem(hHitItem);
		ObjProp * obj = (ObjProp *)this->GetItemData(hHitItem);
		SeriesProp * seriesProp = dynamic_cast<SeriesProp *>(obj);
		if (seriesProp != 0)
		{
			contextMenuProp = seriesProp;
			SendMessage(WM_CONTEXTMENU, (WPARAM) m_hWnd, GetMessagePos());
		}
	}

	*pResult = 1;
}


int CViewTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

void CViewTree::OnCloseChannel()
{
	contextMenuProp->Close();
	PlotWndProp * plotWndProp = contextMenuProp->GetPlotWndProp();
	if (plotWndProp)
		plotWndProp->RemoveSeries(contextMenuProp);
}

void CViewTree::OnChannelSaveSelection()
{
	CString fileName;
	bool succ = ShowFileDialog(fileName);
	if (succ)
		contextMenuProp->Export(fileName, false);
}

void CViewTree::OnChannelSaveAll()
{
	CString fileName;
	bool succ = ShowFileDialog(fileName);
	if (succ)
		contextMenuProp->Export(fileName, true);
}

bool CViewTree::ShowFileDialog(CString & filename)
{
	CFileDialog dlgFile(FALSE, L"txt", 0, 6UL, L"Text file(*.txt)\0*.txt\0All file(*.*)\0*.*\0\0");
	CString fileName;
	const int c_cMaxFiles = 100;
	const int c_cbBuffSize = (c_cMaxFiles * (MAX_PATH + 1)) + 1;
	dlgFile.GetOFN().lpstrFile = fileName.GetBuffer(c_cbBuffSize);
	dlgFile.GetOFN().nMaxFile = c_cbBuffSize;

	if (IDOK == dlgFile.DoModal())
	{

		FILE * fp;
		errno_t ret = _wfopen_s(&fp, fileName, L"wb");

		bool completeBlock = true;
		if (ret == 0)
		{
			fclose(fp);
			filename = fileName;
			return true;
		}
		else
		{
			CString errMsg;
			errMsg.Format(L"Open file error: %x\n", ret);
			::AfxMessageBox(errMsg);
			return false;
		}
	}

	return false;
}


void CViewTree::OnTextChannelExport()
{
	CString fileName;
	bool succ = ShowFileDialog(fileName);
	if (succ)
		contextMenuProp->Export(fileName, true);
}

void CViewTree::OnTextChannelClose()
{
	contextMenuProp->Close();
	PlotWndProp * plotWndProp = contextMenuProp->GetPlotWndProp();
	if (plotWndProp)
		plotWndProp->RemoveSeries(contextMenuProp);
}