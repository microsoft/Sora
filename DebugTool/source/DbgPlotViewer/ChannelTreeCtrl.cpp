// ChannelTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include <memory>
#include "DbgPlotViewer.h"
#include "ChannelTreeCtrl.h"
#include "ChannelOpened.h"
#include "ChannelOpenedLine.h"
#include "ChannelOpenedDots.h"
#include "ChannelOpenedLog.h"
#include "ChannelOpenedText.h"
#include "ChannelOpenedSpectrum.h"
#include "PlotWnd.h"
#include "ChannelAddable.h"
#include "Targetable.h"

using namespace std;

// ChannelTreeCtrl

IMPLEMENT_DYNAMIC(ChannelTreeCtrl, CTreeCtrl)

ChannelTreeCtrl::ChannelTreeCtrl()
{
	_bDragging = false;
}

ChannelTreeCtrl::~ChannelTreeCtrl()
{
	_pvTaskQueue.Execute(true);
}

void ChannelTreeCtrl::UpdateTreeData(const std::map<std::shared_ptr<ProcessOpened>, std::set<std::shared_ptr<ChannelOpened> > > & tree)
{
	_pvTaskQueue.Queue([this, tree](bool bClose){
		if (bClose)
			return;

		this->_processChannelTree = tree;
		this->UpdateTreeView();
	});

	HWND hWnd = m_hWnd;
	if (hWnd)
		::PostMessage(hWnd, WMME_EXE_TASKQUEUE, 0, 0);
}

void ChannelTreeCtrl::UpdateTreeView()
{
	this->SetRedraw(FALSE);
	this->DeleteAllItems();

	for (auto iterPsChMap = _processChannelTree.begin(); iterPsChMap != _processChannelTree.end(); ++iterPsChMap)
	{
		auto process = iterPsChMap->first;
		CString str;
		str.Format(L"%s(%d)", process->Name().c_str(), process->Pid());

		int imgIdx = true ? 10 : 11;

		HTREEITEM hProcess = this->InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE| TVIF_SELECTEDIMAGE | TVIF_STATE, str.GetBuffer(), imgIdx, imgIdx, 0, 0, (LPARAM)process.get(), 0, TVI_SORT);

		auto channelMap = iterPsChMap->second;
		for (auto iterChannel = channelMap.begin(); iterChannel != channelMap.end(); ++iterChannel)
		{
			auto channel = *iterChannel;
			CString str;
			str.Format(L"%s", channel->Name().c_str());

			int imgIdx = 0;
			if (typeid(*channel) == typeid(ChannelOpenedLine))
				imgIdx = 0;
			else if (typeid(*channel) == typeid(ChannelOpenedDots))
				imgIdx = 2;
			else if (typeid(*channel) == typeid(ChannelOpenedSpectrum))
				imgIdx = 4;
			else if (typeid(*channel) == typeid(ChannelOpenedLog))
				imgIdx = 6;
			else if (typeid(*channel) == typeid(ChannelOpenedText))
				imgIdx = 8;

			if (! channel->IsAttatched())
				imgIdx++;

			HTREEITEM hItem = this->InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, str.GetBuffer(), imgIdx, imgIdx, 0, 0, (LPARAM)channel.get(), hProcess, TVI_SORT);
		}

		this->Expand(hProcess, TVE_EXPAND);
	}

	this->SetRedraw(TRUE);
}

BEGIN_MESSAGE_MAP(ChannelTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &ChannelTreeCtrl::OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_MESSAGE(WMME_EXE_TASKQUEUE, OnExeTaskQueue)
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_CLICK, &ChannelTreeCtrl::OnNMClick)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_RCLICK, &ChannelTreeCtrl::OnNMRClick)
	ON_COMMAND(ID_CLOSE_CHANNEL, OnCloseChannel)
	ON_COMMAND(ID_CHANNEL_SAVESELECTION, OnChannelSaveSelection)
	ON_COMMAND(ID_CHANNEL_SAVEALL, OnChannelSaveAll)
	ON_COMMAND(ID_CHANNEL_CLOSE, OnTextChannelClose)
	ON_COMMAND(ID_CHANNEL_EXPORT, OnTextChannelExport)
END_MESSAGE_MAP()



// ChannelTreeCtrl message handlers




void ChannelTreeCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	auto obj = (PropObject *)pNMTreeView->itemNew.lParam;
	this->SelectItem(pNMTreeView->itemNew.hItem);
	auto spObj = obj->shared_from_this();
	auto channel = dynamic_pointer_cast<ChannelOpened, AsyncObject>(spObj);

	if (channel && channel->IsAttatched())
	{
		TRACE0("Channel dragged\n");
		SetCapture();
		_bDragging = true;
		_channelDragged = channel;
	}


	*pResult = 0;
}


void ChannelTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if (_bDragging)
	{
		CPoint pointParam;
		auto obj = FindTarget(pointParam);

		if (obj != _lastObj)
		{
			if (_lastObj)
				_lastObj->Highlight(false);

			_lastObj = obj;
		}

		if (obj)
		{
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			obj->Highlight(true);
			
		}
		else
		{
			::SetCursor(::LoadCursor(NULL, IDC_NO));
		}
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}


void ChannelTreeCtrl::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	//_pvTaskQueue.Execute(true);

	CTreeCtrl::OnClose();
}

LRESULT ChannelTreeCtrl::OnExeTaskQueue(WPARAM wParam, LPARAM lParam)
{
	_pvTaskQueue.Execute(false);

	return 0;
}

shared_ptr<ChannelAddable> ChannelTreeCtrl::FindTarget(CPoint & pointOut)
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

	auto targetable = dynamic_cast<Targetable *>(wnd);
	if (targetable)
	{
		auto channelAddable = (ChannelAddable *)targetable->UserData();
		ASSERT(channelAddable);

		if (channelAddable->Accept(_channelDragged, screenCursorPoint, pointOut))
			return dynamic_pointer_cast<ChannelAddable, AsyncObject>(channelAddable->shared_from_this());
		else
			return 0;
	}

	return 0;
}

void ChannelTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	if (_bDragging)
	{
		CPoint pointOut;

		auto obj = this->FindTarget(pointOut);

		if (obj)
		{
			obj->Highlight(false);

			if (_channelDragged->IsAttatched())
			{
				shared_ptr<ProcessOpened> processOpened;
				for (auto iterPs = _processChannelTree.begin(); iterPs != _processChannelTree.end(); ++iterPs)
				{
					auto process = iterPs->first;
					auto chMap = iterPs->second;
					auto iterCh = chMap.find(_channelDragged);
					if (iterCh != chMap.end())
					{
						processOpened = iterPs->first;
						break;
					}
				}

				ASSERT(processOpened != 0);

				obj->RequestAddChannel(processOpened, _channelDragged, pointOut);
			}
		}

		ReleaseCapture();
		_lastObj = 0;
		_bDragging = false;
	}

	CTreeCtrl::OnLButtonUp(nFlags, point);
}


void ChannelTreeCtrl::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	_pvTaskQueue.Execute(true);

	EventObjSelected.Reset();
	EventCloseChannel.Reset();
	EventClosed.Raise(this, true);

	CTreeCtrl::PostNcDestroy();
}


void ChannelTreeCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	DWORD dw = GetMessagePos(); // Mouse position 
	CPoint p(GET_X_LPARAM(dw), GET_Y_LPARAM(dw)); 
	this->ScreenToClient(&p); 

	UINT htFlags = 0; 
	HTREEITEM hHitItem= this->HitTest(p, &htFlags);

	if (htFlags & TVHT_ONITEM) 
	{
		auto obj = (PropObject *)this->GetItemData(hHitItem);
		auto spObj = dynamic_pointer_cast<PropObject, AsyncObject>(obj->shared_from_this());

		this->EventObjSelected.Raise(this, spObj);
	}

	*pResult = 0;
}

bool ChannelTreeCtrl::ShowFileDialog(CString & filename)
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


void ChannelTreeCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	bool bOpened = _contextMenuObj->GetOpenState();
	if (!bOpened)
		return;

	auto channelText = dynamic_pointer_cast<ChannelOpenedText, PropObject>(_contextMenuObj);
	if (channelText)
	{
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_TEXT_CHANNEL, point.x, point.y, this, TRUE);
		return;
	}
	
	auto channelLog = dynamic_pointer_cast<ChannelOpenedLog, PropObject>(_contextMenuObj);
	if (channelLog)
	{
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_TEXT_CHANNEL, point.x, point.y, this, TRUE);
		return;
	}

	auto channel = dynamic_pointer_cast<ChannelOpened, PropObject>(_contextMenuObj);
	if (channel)
	{
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_CHANNEL_EXPLORER, point.x, point.y, this, TRUE);		
	}
}


void ChannelTreeCtrl::OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	DWORD dw = GetMessagePos(); // Mouse position 
	CPoint p(GET_X_LPARAM(dw), GET_Y_LPARAM(dw)); 
	this->ScreenToClient(&p); 

	UINT htFlags = 0; 
	HTREEITEM hHitItem= this->HitTest(p, &htFlags);

	if (htFlags & TVHT_ONITEM) 
	{
		HTREEITEM hItemOld = this->GetSelectedItem();
		this->SelectItem(hHitItem);

		auto obj = (PropObject *)this->GetItemData(hHitItem);
		auto spObj = dynamic_pointer_cast<ChannelOpened, AsyncObject>(obj->shared_from_this());

		if (spObj != 0)
		{
			_contextMenuObj = spObj;
			SendMessage(WM_CONTEXTMENU, (WPARAM) m_hWnd, GetMessagePos());
		}
	}

	*pResult = 1;
}

void ChannelTreeCtrl::OnCloseChannel()
{
	this->EventCloseChannel.Raise(this, _contextMenuObj);
}

void ChannelTreeCtrl::OnChannelSaveSelection()
{
	CString fileName;
	bool succ = ShowFileDialog(fileName);
	if (succ)
		_contextMenuObj->Export(fileName, false);
}

void ChannelTreeCtrl::OnChannelSaveAll()
{
	CString fileName;
	bool succ = ShowFileDialog(fileName);
	if (succ)
		_contextMenuObj->Export(fileName, true);
}

void ChannelTreeCtrl::OnTextChannelExport()
{
	CString fileName;
	bool succ = ShowFileDialog(fileName);
	if (succ)
		_contextMenuObj->Export(fileName, true);
}

void ChannelTreeCtrl::OnTextChannelClose()
{
	this->EventCloseChannel.Raise(this, _contextMenuObj);
}
