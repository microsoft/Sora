// AutoLayoutPanel.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "AutoLayoutPanel.h"
#include "AppMessage.h"

// AutoLayoutPanel

IMPLEMENT_DYNAMIC(AutoLayoutPanel, CDockablePane)

AutoLayoutPanel::AutoLayoutPanel()
{

}

AutoLayoutPanel::~AutoLayoutPanel()
{
}


BEGIN_MESSAGE_MAP(AutoLayoutPanel, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()



// AutoLayoutPanel message handlers




int AutoLayoutPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	this->slider.Create(WS_CHILD | WS_VISIBLE, rectDummy, this, ID_SLIDER);
	this->slider.SetRange(0, 80, 1);	// hard code!!!

	return 0;
}


void AutoLayoutPanel::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	this->slider.MoveWindow(0, 0, cx, cy, 1);
}

void AutoLayoutPanel::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	switch (nSBCode)
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			int pos = nPos*5 + 100; // hard code!!!
			::AfxGetMainWnd()->PostMessage(WM_APP, CMD_AUTO_LAYOUT, pos);
		}
		break;
	case SB_ENDSCROLL:
		{
			int pos = this->slider.GetPos()*5 + 100;	// hard code!!!
			::AfxGetMainWnd()->PostMessage(WM_APP, CMD_AUTO_LAYOUT, pos);
		}
		break;
	}

	CDockablePane::OnHScroll(nSBCode, nPos, pScrollBar);
}
