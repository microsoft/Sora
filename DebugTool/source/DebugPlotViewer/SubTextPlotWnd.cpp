// SubTextPlotWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DebugPlotViewer.h"
#include "SubTextPlotWnd.h"
#include "AppMessage.h"
#include "SeriesText.h"

// SubTextPlotWnd

IMPLEMENT_DYNAMIC(SubTextPlotWnd, CRichEditCtrl)

SubTextPlotWnd::SubTextPlotWnd()
{
	//this->sender = 0;
	this->plotWndProp = 0;
	lastColor = RGB(0,0,0);
}

SubTextPlotWnd::~SubTextPlotWnd()
{
}


BEGIN_MESSAGE_MAP(SubTextPlotWnd, CRichEditCtrl)
	ON_WM_CREATE()
	ON_MESSAGE(WM_APP, OnApp)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// SubTextPlotWnd message handlers

void SubTextPlotWnd::SetRichEditData(const wchar_t * string, bool replace, COLORREF color)
{
	this->SetRedraw(FALSE);

	CHARRANGE range;

	if (replace)
		range.cpMin = 0;
	else
		range.cpMin = -1;
		
	range.cpMax = -1;
	this->SetSel(range);
	this->GetSel(range);
	this->ReplaceSel(string, 0);
	range.cpMax = -1;
	this->SetSel(range);

	// set color
	CHARFORMAT cf;
	this->GetSelectionCharFormat(cf);
	cf.crTextColor = color; //RGB
	cf.dwEffects &= ~CFE_AUTOCOLOR;
	cf.dwMask = CFM_COLOR;
	this->SetSelectionCharFormat(cf);
	
	if (1)		// clean up, hard coded
	{
		range.cpMin = 0;
		range.cpMax = -1;
		this->SetSel(range);
		this->GetSel(range);
		if (range.cpMax > 32*1024*1024)
		{
			range.cpMax = 16*1024*1024;
			this->SetSel(range);
			this->ReplaceSel(L"");
		}
	}

	
	range.cpMax = range.cpMin = -1;
	this->SetSel(range);

	this->SetRedraw(TRUE);
	this->InvalidateRgn(NULL, 1);

	this->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

void SubTextPlotWnd::SetColor(COLORREF color)
{
	this->SetRedraw(FALSE);

	CHARRANGE range;

	range.cpMin = 0;
	range.cpMax = -1;
	this->SetSel(range);

	// set color
	CHARFORMAT cf;
	this->GetSelectionCharFormat(cf);
	cf.crTextColor = color; //RGB
	cf.dwEffects &= ~CFE_AUTOCOLOR;
	cf.dwMask = CFM_COLOR;
	this->SetSelectionCharFormat(cf);

	this->SetRedraw(TRUE);
	this->InvalidateRgn(NULL, 1);
}

int SubTextPlotWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CRichEditCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	
	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_REGISTER_WND, (LPARAM)this);

	this->SetOptions(ECOOP_SET, ECO_READONLY);
	ASSERT(this->GetStyle() & ES_READONLY);
	this->SetBackgroundColor(false, RGB(0,0,0));

	return 0;
}


BOOL SubTextPlotWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= ES_MULTILINE;

	return CRichEditCtrl::PreCreateWindow(cs);
}

LRESULT SubTextPlotWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_SERIES_CLOSED:
		//this->DestroyWindow();
		return 0;
	case CMD_DATA_CLEAR:
		RemoveData((void *)lParam);
		return 0;
	//case CMD_NEW_DATA:
	//	SetData((StringData *)lParam);
	//	return 0;
	case CMD_QUERY_TYPE:
		this->GetParent()->SendMessage(WM_APP, wParam, lParam);
		return 0;
	case CMD_SET_REPLACE_MODE:
		this->SetReplaceMode(lParam != 0);
		return 0;
	//case CMD_UPDATE_VIEW:
	//	this->SetData(0);
	//	return 0;
	}
	return 0;
}

void SubTextPlotWnd::SetData(StringData * dataInfo)
{
	ASSERT(this->plotWndProp->seriesInPlotWndProp.size() == 1);

	TextSeriesProp * seriesProp = (TextSeriesProp *)this->plotWndProp->seriesInPlotWndProp[0];

	COLORREF color;
	color = seriesProp->color;

	//if (seriesProp->colorIsSet)
	//	color = seriesProp->color;
	//else
	//	color = RGB(0, 255, 0);

	if (!seriesProp->isLog)
	{
		int frameCount = seriesProp->GetPlotWndProp()->frameCount;
		if (seriesProp->replaceMode)
			frameCount = 1;

		int stringLen = 0;

		int validFrameCount = 0;
		int frameIdx = plotWndProp->replayReadPos;

		while( (frameIdx > 0) && (validFrameCount < frameCount) )
		{
			frameIdx--;
			int frameDataCount = seriesProp->replayBuffer[frameIdx].len / sizeof(char);
			if (frameDataCount > 0)
			{
				validFrameCount++;
			}
			stringLen += frameDataCount;
			if (stringLen > 4*1024)
				break;
		}

		char * string = new char[stringLen+1];
		char * dstPtr = string;

		while( frameIdx < plotWndProp->replayReadPos )
		{
			int frameDataCount = seriesProp->replayBuffer[frameIdx].len / sizeof(char);
			if (frameDataCount > 0)
			{
				memcpy(dstPtr, seriesProp->replayBuffer[frameIdx].data, frameDataCount * sizeof(char));
				dstPtr += frameDataCount;
			}
			frameIdx++;
		}

		//int testLen = dstPtr - string;
		//ASSERT(testLen == stringLen);	//TODO
		*dstPtr = 0;

		CString lastDataString(string);
		if ( (color != lastColor) || (lastData.Compare(lastDataString) != 0) )
		{
			this->SetRichEditData(lastDataString, 1, color);
			lastData.SetString(lastDataString);
			lastColor = color;
		}

		delete [] string;
	}
	else
	{
		::EnterCriticalSection(&seriesProp->csLogBuf);
		if (seriesProp->logBuf.GetLength() > 0)
			this->SetRichEditData(seriesProp->logBuf, 0, color);
		seriesProp->logBuf.Empty();
		::LeaveCriticalSection(&seriesProp->csLogBuf);

		if (color != lastColor)
		{
			this->SetColor(color);
			lastColor = color;
		}
	}
}

void SubTextPlotWnd::RemoveData(void * sender)
{	
	COLORREF color = RGB(0,0,0);
	this->SetRichEditData(L"", 1, color);
}

bool SubTextPlotWnd::AcceptSeries()
{
	return this->plotWndProp->seriesInPlotWndProp.size() == 0;
}

void SubTextPlotWnd::SetReplaceMode(bool isReplace)
{
	if ( isReplace &&
		 (this->GetStyle() & WS_VSCROLL) )
	{
			this->ModifyStyle(WS_VSCROLL, 0);
	}
	else if ( !isReplace &&
		!(this->GetStyle() & WS_VSCROLL) )
	{
		this->ModifyStyle(0, WS_VSCROLL);
	}
}


void SubTextPlotWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	::AfxGetMainWnd()->SendMessage(WM_APP, CMD_UNREGISTER_WND, (LPARAM)this);
	delete this;

	CRichEditCtrl::PostNcDestroy();
}



void SubTextPlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	LPARAM lParam = MAKELONG(point.x, point.y);

	GetParent()->SendMessage(WM_LBUTTONDOWN, nFlags, lParam);

	CRichEditCtrl::OnLButtonDown(nFlags, point);
}
