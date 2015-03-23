// DockableWndContainer.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "DockableWndContainer.h"

// DockableWndContainer

IMPLEMENT_DYNAMIC(DockableWndContainer, CDockablePane)

DockableWndContainer::DockableWndContainer()
{
	this->StrategyLayout.Set([](const void * sender, const LayoutParam & param, bool ret){
		if ((*param._childs).size() > 0)
		{
			(*param._childs)[0]->MoveWindow(0, 0, param._cx, param._cy);
			(*param._childs)[0]->Invalidate(1);
		}
		ret = true;
	});

	_color = RGB(50, 50, 50);
}

DockableWndContainer::~DockableWndContainer()
{
}


void DockableWndContainer::AddChild(std::shared_ptr<CWnd> wnd)
{
	_childs.push_back(wnd);
}

void DockableWndContainer::RemoveAllChilds()
{
	for (auto iter = _childs.begin(); iter != _childs.end(); ++iter)
	{
		if ((*iter)->m_hWnd)
			(*iter)->SendMessage(WM_CLOSE, 0, 0);
	}

	_childs.clear();
}

void DockableWndContainer::UpdateLayout()
{
	CRect rect;
	GetClientRect(&rect);
	AutoLayout(rect.Width(), rect.Height());
}

void DockableWndContainer::Clear()
{
	_childs.clear();
}

BEGIN_MESSAGE_MAP(DockableWndContainer, CDockablePane)
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()



// DockableWndContainer message handlers


void DockableWndContainer::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	AutoLayout(cx, cy);
}

void DockableWndContainer::AutoLayout(int cx, int cy)
{
	LayoutParam param;
	param._cx = cx;
	param._cy = cy;
	param._childs = &_childs;
	bool ret = false;
	StrategyLayout.Call(this, param, ret);
}


void DockableWndContainer::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDockablePane::OnPaint() for painting messages

	CRect rect;
	GetClientRect(rect);

	CBrush brush(_color);
	dc.FillRect(rect, &brush);
}

void DockableWndContainer::SetBgColor(COLORREF color)
{
	_color = color;
}


BOOL DockableWndContainer::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	
	cs.style |= WS_CLIPCHILDREN;

	return CDockablePane::PreCreateWindow(cs);
}
