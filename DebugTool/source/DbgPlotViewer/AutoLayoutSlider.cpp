// AutoLayoutSlider.cpp : implementation file
//

#include "stdafx.h"
#include "DbgPlotViewer.h"
#include "AutoLayoutSlider.h"


// AutoLayoutSlider

IMPLEMENT_DYNAMIC(AutoLayoutSlider, CWnd)

AutoLayoutSlider::AutoLayoutSlider()
{

}

AutoLayoutSlider::~AutoLayoutSlider()
{
}


BEGIN_MESSAGE_MAP(AutoLayoutSlider, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()



// AutoLayoutSlider message handlers




int AutoLayoutSlider::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	this->_sliderCtrl.Create(WS_CHILD | WS_VISIBLE, rectDummy, this, 1);
	this->_sliderCtrl.SetRange(0, 80, 1);	// hard code!!!

	return 0;
}


void AutoLayoutSlider::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	_sliderCtrl.MoveWindow(0, 0, cx, cy, 1);
}


void AutoLayoutSlider::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	switch (nSBCode)
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			int pos = nPos*5 + 100; // hard code!!!
			EventAutoScale.Raise(this, pos);
		}
		break;
	case SB_ENDSCROLL:
		{
			int pos = this->_sliderCtrl.GetPos()*5 + 100;	// hard code!!!
			EventAutoScale.Raise(this, pos);
		}
		break;
	}


	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}


void AutoLayoutSlider::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	EventAutoScale.Reset();

	CWnd::PostNcDestroy();
}
