// ControlPanelList.cpp : implementation file
//

#include "stdafx.h"
#include <algorithm>
#include "DbgPlotViewer.h"
#include "ControlPanelList.h"


// ControlPanelList

IMPLEMENT_DYNAMIC(ControlPanelList, Invokable)

ControlPanelList::ControlPanelList()
{

}

ControlPanelList::~ControlPanelList()
{
}

void ControlPanelList::AddControlPanelWnd(std::shared_ptr<ControlPanelWnd> wnd)
{
	this->Invoke([this, wnd](bool close){

		if (close)
			return;

		if (m_hWnd == 0)
			return;

		this->_controlPanelList.push_back(wnd);
		CRect rectDummy;
		rectDummy.SetRectEmpty();
		wnd->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rectDummy, this, 0);
		if (this->m_hWnd)
			this->AdjustLayout();
	});
}

void ControlPanelList::RemoveControlPanelWnd(std::shared_ptr<ControlPanelWnd> wnd)
{
	this->Invoke([this, wnd](bool close){

		if (close)
			return;

		auto iter = std::find(this->_controlPanelList.begin(), this->_controlPanelList.end(), wnd);
		assert(iter != this->_controlPanelList.end());
		this->_controlPanelList.erase(iter);
		if (this->m_hWnd)
			this->AdjustLayout();
	});
}

void ControlPanelList::AdjustLayout()
{
	CRect rect;
	this->GetClientRect(&rect);
	int width = rect.Width();

	int y = 0;
	const int WND_HEIGHT= ControlPanelWnd::Height();
	const int MARGIN = 1;

	for (
		auto iterWnd = _controlPanelList.begin();
		iterWnd != _controlPanelList.end();
		++iterWnd)
		{
			auto& wnd = *iterWnd;
			if (wnd->m_hWnd)
			{
				wnd->MoveWindow(0, y, width, WND_HEIGHT, 1);
				y += WND_HEIGHT + MARGIN;
			}
		}
}

BEGIN_MESSAGE_MAP(ControlPanelList, Invokable)
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// ControlPanelList message handlers

void ControlPanelList::OnSize(UINT nType, int cx, int cy)
{
	Invokable::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	this->Invoke([this, cx](bool close){
		if (close)
			return;

		if (this->m_hWnd)
			this->AdjustLayout();
	});
}


void ControlPanelList::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call Invokable::OnPaint() for painting messages
	CBrush brush(RGB(0, 0, 0));
	CRect rect;
	GetClientRect(&rect);
	dc.FillRect(rect,&brush);
}


BOOL ControlPanelList::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	
	cs.style |= WS_CLIPCHILDREN;

	return Invokable::PreCreateWindow(cs);
}


void ControlPanelList::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	Invokable::PostNcDestroy();
}
