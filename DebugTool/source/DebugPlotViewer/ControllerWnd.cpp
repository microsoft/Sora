// ControllerWnd.cpp : implementation file
//

#include "stdafx.h"
#include <algorithm>
#include "AppSettings.h"
#include "DebugPlotViewer.h"
#include "ControllerWnd.h"
#include "SharedObjManager.h"
//#include "AppType.h"
#include "ControllerProperty.h"
#include "sora.h"

class ObjInRect
{
public:
	void SetRect(CRect * rect);
	void Draw(Graphics * g);
	void OnMouseOver();
	void OnMouseLeave();
	void OnLButtonDown();
	void OnLButtonUp();
private:
	CRect rect;
	enum MouseState
	{
		MOUSE_LEAVE,
		MOUSE_OVER,
		MOUSE_L_DOWN,
		MOUSE_R_DOWN,
	} state;
};

// ControllerWnd

IMPLEMENT_DYNAMIC(ControllerWnd, CWnd)

ControllerWnd::ControllerWnd()
{
	process = 0;
	overviewDispData = 0;
	overviewDispDataLen = 0;
	this->buttonPlay.SetType(TYPE_PLAY);
	//this->buttonAutoPlay.SetType(TYPE_AUTO_REPLAY);
	//this->buttonSpeedUp.SetType(TYPE_PLUS);
	//this->buttonSpeedDown.SetType(TYPE_MINUS);
}

ControllerWnd::~ControllerWnd()
{
	if (overviewDispData)
		delete overviewDispData;
}


BEGIN_MESSAGE_MAP(ControllerWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	//ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_APP, OnApp)
	//ON_WM_MBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// ControllerWnd message handlers

// Ops...
void ControllerWnd::SetLayout(
	int pidLeft,
	int pidRight,
	int barLeft,
	int barRight,
	//int speedUpLeft,
	//int speedUpRight,
	//int speedDownLeft,
	//int speedDownRight,
	int playLeft,
	int playRight
	//int autoPlayLeft,
	//int autoPlayRight
	)
{
	this->pidLeft = pidLeft;
	this->pidRight = pidRight;
	this->barLeft = barLeft;
	this->barRight = barRight;
	//this->speedUpLeft = speedUpLeft;
	//this->speedUpRight = speedUpRight;
	//this->speedDownLeft = speedDownLeft;
	//this->speedDownRight = speedDownRight;
	this->playLeft = playLeft;
	this->playRight = playRight;
	//this->autoPlayLeft = autoPlayLeft;
	//this->autoPlayRight = autoPlayRight;
}

void ControllerWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);
	Bitmap bmp(rect.right,rect.bottom);

	Graphics* memGraph = Graphics::FromImage(&bmp);

	if (process->traceMode == DebuggingProcessProp::MODE_TRACE_GRAPH)
		DrawGraphTrackbar(memGraph);
	else
		DrawSourceTrackbar(memGraph);

	DrawCaption(memGraph);

	graphics.DrawImage(&bmp,rect.left,rect.top,rect.right,rect.bottom);
	
	delete memGraph;
}

void ControllerWnd::DrawGraphTrackbar(Graphics * g)
{
	CRect rect;
	GetClientRect(&rect);
	rect.left = this->barLeft;
	rect.right = this->barRight;

	float readRatio = (float)this->process->replayReadPosInProcess / this->process->replayCapacity;
	float readPos = (rect.right - rect.left) * readRatio + rect.left;
	Color colorReadDataPart(28,48,69);
	SolidBrush brushReadDataPart(colorReadDataPart);
	g->FillRectangle(
		&brushReadDataPart, 
		float(rect.left),
		float(rect.top), 
		readPos, 
		float(rect.bottom));

	float emptyStartPos = readPos;

	if (this->process->replayReadPosInProcess < this->process->replayWritePos)
	{
		float writeRatio = (float)this->process->replayWritePos / this->process->replayCapacity;
		float writePos = float((rect.right - rect.left) * writeRatio + rect.left);
		Color colorWriteDataPart(100, 100, 100);
		SolidBrush brushWriteDataPart(colorWriteDataPart);
		g->FillRectangle(
			&brushWriteDataPart,
			readPos,
			float(rect.top), 
			writePos, 
			float(rect.bottom));
		emptyStartPos = writePos;
	}

	SolidBrush brush(Color::Black);
	g->FillRectangle(
		&brush,
		emptyStartPos,
		float(rect.top),
		float(rect.right),
		float(rect.bottom));
}

float CalcOverview(int re, int im)
{
	float overview = log((float)re*re + (float)im*im) - 10;
	if (overview < 0.0)
		overview = 0.0;
	return overview;
}

void ControllerWnd::DrawSourceTrackbar(Graphics * g)
{
	if (!(process->smSourceInfo && process->smSourceData))
	{
		CRect rect;
		GetClientRect(&rect);

		// draw background
		SolidBrush brushBk(Color::Black);
		g->FillRectangle(
			&brushBk, 
			rect.left,
			rect.top,
			rect.Width(),
			rect.Height());

		return;
	}

	CRect rect;
	GetClientRect(&rect);
	rect.left = this->barLeft;
	rect.right = this->barRight;

	// draw read data part
	SolidBrush brushReadDataPart(Color::Black);
	g->FillRectangle(
		&brushReadDataPart, 
		rect.left,
		rect.top,
		rect.Width(),
		rect.Height());

	SharedSourceInfo * sourceInfo = (SharedSourceInfo *)process->smSourceInfo->GetAddress();
	char * sourceData = (char *)process->smSourceData->GetAddress();

	// draw progress
	unsigned int rIdx = sourceInfo->rIdx;
	unsigned int wIdx = sourceInfo->wIdx;
	int viewWndSize = process->viewWndSize;

	if (rIdx != wIdx)
	{
		//int wndLeft, wndRight;
		int wndLeft = rIdx - rIdx % viewWndSize;
		int wndRight = wndLeft + viewWndSize;

		// draw progress wnd
		float wndLeftF = (float)wndLeft / wIdx * rect.Width() + rect.left;
		float wndRightF = (float)wndRight / wIdx * rect.Width() + rect.left;
		float wndWidthF = max(1.0f, wndRightF - wndLeftF);

		Color color(48,88,129);
		SolidBrush brushReadDataPart(color);
		g->FillRectangle(
			&brushReadDataPart, 
			wndLeftF,
			float(rect.top),
			wndWidthF,
			float(OVERVIEW_MARGIN_TOP - 2));

		// draw overview
		COMPLEX16 * overviewStart = (COMPLEX16*)sourceData + wndLeft/sizeof(COMPLEX16);
		COMPLEX16 * overviewEnd = (COMPLEX16*)sourceData + wndRight/sizeof(COMPLEX16);

		unsigned int overviewDataCount = overviewEnd - overviewStart;
		unsigned int wndDispCount = rect.Width();
		
		unsigned int dispCount = min(overviewDataCount, wndDispCount);

		static const float ENERGY_MAX = ::CalcOverview(32768, 32768);

		Color colorOverview(0, 255, 0);
		Pen penOverview(colorOverview);
		float lastDispX, lastDispY;

		float drawRectHeight = float(rect.Height() - OVERVIEW_MARGIN_TOP - OVERVIEW_MARGIN_BOTTOM);
		float drarRectBottom = float(rect.bottom - OVERVIEW_MARGIN_BOTTOM);

		if (overviewDataCount > dispCount)	// normal case
		{
			for (unsigned int i = 0; i < dispCount; i++)
			{
				unsigned int dataIdx = (__int64)i * overviewDataCount / dispCount;
				if (dataIdx >= wIdx/sizeof(COMPLEX16))
					break;

				int re = overviewStart[dataIdx].re;
				int im = overviewStart[dataIdx].im;

				float energy = ::CalcOverview(re, im);

				float dispX = float(rect.left + i);
				float dispY = float(drarRectBottom - energy * drawRectHeight / ENERGY_MAX - 1);
				if (i == 0)
				{
					lastDispX = dispX;
					lastDispY = dispY;
				}
				else
				{
					g->DrawLine(&penOverview, lastDispX, lastDispY, dispX, dispY);
					lastDispX = dispX;
					lastDispY = dispY;
				}
			}
		}
		else
		{
			if (overviewDataCount >= 2)		// minimum number of dots to draw line
			{
				for (unsigned int i = 0; i < overviewDataCount; i++)
				{
					if (i >= wIdx/sizeof(COMPLEX16))
						break;

					int re = overviewStart[i].re;
					int im = overviewStart[i].im;

					float energy = ::CalcOverview(re, im);

					float dispX = rect.left + (float)i * rect.Width() / (overviewDataCount - 1);
					float dispY = drarRectBottom - energy * drawRectHeight / ENERGY_MAX - 1;
					if (i == 0)
					{
						lastDispX = dispX;
						lastDispY = dispY;
					}
					else
					{
						g->DrawLine(&penOverview, lastDispX, lastDispY, dispX, dispY);
						lastDispX = dispX;
						lastDispY = dispY;
					}
				}
			}
		}

		// draw current reading point
		float headRatio = (float)(rIdx % viewWndSize) / viewWndSize;
		float headX =  rect.Width() * headRatio + rect.left;
		Pen pen(Color::White);
		g->DrawLine(&pen, headX, (float)rect.top, headX, (float)rect.bottom);

	}
}

void ControllerWnd::DrawSourceTrackbarTop(Graphics * g, CRect * rect)
{
	Color color(100,100,100);
	SolidBrush brushReadDataPart(color);
	g->FillRectangle(
		&brushReadDataPart, 
		rect->left,
		rect->top, 
		rect->right, 
		rect->bottom);
}

void ControllerWnd::DrawSourceTrackbarBottom(Graphics * g, CRect * rect)
{
	Color color(28,48,69);
	SolidBrush brushReadDataPart(color);
	g->FillRectangle(
		&brushReadDataPart, 
		rect->left,
		rect->top, 
		rect->right, 
		rect->bottom);
}

void ControllerWnd::DrawCaption(Graphics * g)
{
	ASSERT(this->process);

	SolidBrush fontBrush(Color(255, 150, 150, 150));
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	format.SetFormatFlags(StringFormatFlagsNoWrap);
	format.SetTrimming(StringTrimmingEllipsisCharacter);
	Gdiplus::Font captionFont(L"Arial", 10);
	PointF pointF(5, 2);

	CRect rectCaption;
	GetClientRect(&rectCaption);
	int top = rectCaption.top;
	int bottom = rectCaption.bottom;
	int middle = (top + bottom) / 2;

	rectCaption.left = this->pidLeft;
	rectCaption.right = this->pidRight;

	SolidBrush brushBk(Color::Black);
	g->FillRectangle(&brushBk, rectCaption.left, rectCaption.top, rectCaption.right,rectCaption.bottom);

	RectF rectName(
		(Gdiplus::REAL)this->pidLeft,
		(Gdiplus::REAL)top,
		(Gdiplus::REAL)this->pidRight,
		(Gdiplus::REAL)middle );
	g->DrawString(this->process->moduleName, -1, &captionFont, rectName, &format, &fontBrush);

	CString strPid;
	strPid.Format(L"(%d)", this->process->pid);
	RectF rectPid(
		(Gdiplus::REAL)this->pidLeft,
		(Gdiplus::REAL)middle,
		(Gdiplus::REAL)this->pidRight,
		(Gdiplus::REAL)bottom );
	g->DrawString(strPid.GetBuffer(), -1, &captionFont, rectPid, &format, &fontBrush);

}

void ControllerWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CRect rect;
	GetClientRect(&rect);
	rect.left = this->playLeft;
	rect.right = this->playRight;
	buttonPlay.MoveWindow(&rect, 1);
	buttonPlay.InvalidateRgn(NULL, 1);

	//rect.left = this->speedUpLeft;
	//rect.right = this->speedUpRight;
	//buttonSpeedUp.MoveWindow(&rect, 1);
	//buttonSpeedUp.InvalidateRgn(NULL, 1);

	//rect.left = this->speedDownLeft;
	//rect.right = this->speedDownRight;
	//buttonSpeedDown.MoveWindow(&rect, 1);	
	//buttonSpeedDown.InvalidateRgn(NULL, 1);

	//rect.left = this->autoPlayLeft;
	//rect.right = this->autoPlayRight;
	//buttonAutoPlay.MoveWindow(&rect, 1);	
	//buttonAutoPlay.InvalidateRgn(NULL, 1);
}


void ControllerWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetFocus();
	CWnd::OnMouseMove(nFlags, point);
}


void ControllerWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CRect rect;
	GetClientRect(&rect);

	//if (point.x < this->pidRight)
	//{
	//	ControllerProperty * property = new ControllerProperty;
	//	property->SetTarget(process);
	//	::AfxGetMainWnd()->PostMessage(WM_APP, CMD_CHANGE_PROPERTY, (LPARAM)property);
	//	return;
	//}

	int left = this->barLeft;
	int right = this->barRight;
	int length = right - left;
	ASSERT(length != 0);
	
	if ( (point.x > left) && (point.x < right) )
	{
		if (process->traceMode == DebuggingProcessProp::MODE_TRACE_GRAPH)
		{
			float readRatio = (float)(point.x - left) / length;
			int readPos = (int)(readRatio * process->replayCapacity);
			if (readPos <= this->process->replayWritePos)
				this->process->replayReadPosInProcess = readPos;
		}
		else
		{
			if (process->smSourceInfo != 0)
			{
				process->replayReadPosInProcess = 0;
				process->replayWritePos = 0;

				float readRatio = (float)(point.x - left) / length;
				SharedSourceInfo * sourceInfo = (SharedSourceInfo *)process->smSourceInfo->GetAddress();
				int rIdx = sourceInfo->rIdx;
				int wIdx = sourceInfo->wIdx;
				int viewWndSize = process->viewWndSize;
				int wndStart = rIdx - rIdx % viewWndSize;
				int readPos = int(wndStart + readRatio * viewWndSize);

				readPos = min(sourceInfo->wIdx, readPos);
				readPos -= readPos % sizeof(COMPLEX16);
				sourceInfo->rIdx = readPos;
			}

		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

LRESULT ControllerWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case CMD_CONTROLLER_UPDATE_BUTTON:
		if (process) {
			if (process->ReplayIsPaused())
				//this->buttonPlay.SetWindowText(L">");
				this->buttonPlay.SetType(TYPE_PLAY);
			else
				//this->buttonPlay.SetWindowText(L"||");
				this->buttonPlay.SetType(TYPE_PAUSE);
		} while(0);
		break;
	case CMD_TRACE_TYPE_CHANGED:
		//if (process) {
		//	if (process->traceMode == DebuggingProcessProp::MODE_TRACE_GRAPH)
		//	{
		//		process->replayAutoPlay = TRUE;
		//		//buttonAutoPlay.SetWindowText(L">|");
		//		//buttonAutoPlay.SetType(TYPE_AUTO_REPLAY);
		//		//buttonAutoPlay.EnableWindow(TRUE);

		//	}
		//	else
		//	{
		//		process->replayAutoPlay = TRUE;
		//		//buttonAutoPlay.SetWindowText(L">|");
		//		//buttonAutoPlay.SetType(TYPE_AUTO_REPLAY);
		//		//buttonAutoPlay.EnableWindow(FALSE);
		//	}
		//	this->InvalidateRgn(NULL, 0);
		//} while(0);
		break;
	}
	return 0;
}

int ControllerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rect;
	GetClientRect(&rect);
	rect.left = this->playLeft;
	rect.right = this->playRight;
	CString pauseStr;
	if (process)
	{
		if (process->ReplayIsPaused())
			pauseStr.SetString(L">");
		else
			pauseStr.SetString(L"||");
	}
	else
		pauseStr.SetString(L"||");

	buttonPlay.Create(pauseStr, BS_PUSHBUTTON | BS_OWNERDRAW | WS_VISIBLE | WS_CHILD,
        rect, this, ID_PLAY_BUTTON);

	//rect.left = this->speedUpLeft;
	//rect.right = this->speedUpRight;
	//buttonSpeedUp.Create(L"+", BS_PUSHBUTTON | BS_OWNERDRAW | WS_VISIBLE | WS_CHILD,
 //       rect, this, ID_SPEED_UP_BUTTON);

	//rect.left = this->speedDownLeft;
	//rect.right = this->speedDownRight;
	//buttonSpeedDown.Create(L"-", BS_PUSHBUTTON | BS_OWNERDRAW | WS_VISIBLE | WS_CHILD,
 //       rect, this, ID_SPEED_DOWN_BUTTON);

	//rect.left = this->autoPlayLeft;
	//rect.right = this->autoPlayRight;
	//buttonAutoPlay.Create(L">|", BS_PUSHBUTTON | BS_OWNERDRAW | WS_VISIBLE | WS_CHILD,
 //       rect, this, ID_AUTO_PLAY_BUTTON);

	return 0;
}

void ControllerWnd::SetProcess(DebuggingProcessProp * process)
{
	if (this->process != process)
	{
		ReleaseProcess();
		process->IncRefCount();
		this->process = process;
	}

	if (process->ReplayIsPaused())
		//this->buttonPlay.SetWindowText(L">");
		buttonPlay.SetType(TYPE_PLAY);
	else
		//this->buttonPlay.SetWindowText(L"||");
		buttonPlay.SetType(TYPE_PAUSE);
}

void ControllerWnd::ReleaseProcess()
{
	if (this->process)
	{
		this->process->DecRefCount();
		this->process = 0;
	}
}

void ControllerWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	ReleaseProcess();

	CWnd::PostNcDestroy();
}

//
//void ControllerWnd::OnTimer(UINT_PTR nIDEvent)
//{
//	// TODO: Add your message handler code here and/or call default
//
//	if (nIDEvent == 0)
//	{
//		this->InvalidateRgn(NULL, 0);
//	}
//
//	CWnd::OnTimer(nIDEvent);
//}


BOOL ControllerWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style |= WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);
}

BOOL ControllerWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	switch(LOWORD(wParam))
	{
	case ID_PLAY_BUTTON:
		{
			// for both trace modes
			if ( 1/*process->traceMode == DebuggingProcessProp::MODE_TRACE_GRAPH*/)
			{
				if (process->ReplayIsPaused())
				{
					//buttonPlay.SetWindowText(L"||");
					buttonPlay.SetType(TYPE_PAUSE);
					process->EventPlayPauseAll.Raise(process, true);
					process->ReplayPlay();
					::AfxGetMainWnd()->PostMessage(WM_APP, CMD_PLAY_PAUSE_ALL, (LPARAM)process);
				}
				else
				{
					//buttonPlay.SetWindowText(L">");
					buttonPlay.SetType(TYPE_PLAY);
					process->EventPlayPauseAll.Raise(process, false);
					process->ReplayPause();
					::AfxGetMainWnd()->PostMessage(WM_APP, CMD_PLAY_PAUSE_ALL, (LPARAM)process);
				}
			}
		}
		break;
	//case ID_SPEED_UP_BUTTON:
	//	process->replayStep *= 2;
	//	break;
	//case ID_SPEED_DOWN_BUTTON:
	//	process->replayStep /= 2;
	//	break;
	//case ID_AUTO_PLAY_BUTTON:
	//	{
	//		if (process->traceMode == DebuggingProcessProp::MODE_TRACE_GRAPH)
	//		{
	//			if (process->replayAutoPlay)
	//			{
	//				process->replayAutoPlay = false;
	//				//buttonAutoPlay.SetWindowText(L">>");
	//				buttonAutoPlay.SetType(TYPE_AUTO_PAUSE);
	//			}
	//			else
	//			{
	//				process->replayAutoPlay = true;
	//				//buttonAutoPlay.SetWindowText(L">|");
	//				buttonAutoPlay.SetType(TYPE_AUTO_REPLAY);
	//			}
	//		}
	//		else
	//		{
	//		
	//		}
	//	}
	}

	return CWnd::OnCommand(wParam, lParam);
}


BOOL ControllerWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect(rect);
	ClientToScreen(rect);
	if (!rect.PtInRect(pt))
		goto CALL_DEFAULT;

	if (process->traceMode == DebuggingProcessProp::MODE_TRACE_GRAPH)
	{
		if (zDelta > 0)
		{
			if (process->replayReadPosInProcess > 0)
				process->replayReadPosInProcess--;
		}
		else
		{
			if (process->replayReadPosInProcess < process->replayWritePos)
				process->replayReadPosInProcess++;
		}
	}
	else
	{
		if (zDelta > 0)
		{
			int viewWndSize = process->viewWndSize / 2;
			viewWndSize = max(2*sizeof(COMPLEX16), viewWndSize);
			process->viewWndSize = viewWndSize;
		}
		else
		{
			int viewWndSize = process->viewWndSize * 2;
			viewWndSize = min(viewWndSize, ::SettingGetSourceBufferSize());
			process->viewWndSize = viewWndSize;
		}
	}

CALL_DEFAULT:
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


void ControllerWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: Add your message handler code here
}

void ControllerWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect(&rect);

	if ( (point.x < this->pidRight) &&
		process->traceMode == DebuggingProcessProp::MODE_TRACE_SOURCE)
	{
		CFileDialog dlgFile(FALSE, L"dmp", 0, 6UL, L"Dump file(*.dmp)\0*.dmp\0All file(*.*)\0*.*\0\0");
		CString fileName;
		const int c_cMaxFiles = 100;
		const int c_cbBuffSize = (c_cMaxFiles * (MAX_PATH + 1)) + 1;
		dlgFile.GetOFN().lpstrFile = fileName.GetBuffer(c_cbBuffSize);
		dlgFile.GetOFN().nMaxFile = c_cbBuffSize;

		if (IDOK == dlgFile.DoModal())
		{
			process->smSourceInfo->Lock(INFINITE);

			SharedSourceInfo * sharedSourceInfo = (SharedSourceInfo *)process->smSourceInfo->GetAddress();
			char * sharedSourceData = (char *)process->smSourceData->GetAddress();

			ASSERT(sharedSourceInfo->wIdx % sizeof(COMPLEX16) == 0);
			int index = 0;

			FILE * fp;
			errno_t ret = _wfopen_s(&fp, fileName, L"wb");

			bool completeBlock = true;
			if (ret == 0)
			{
				const int RX_BLOCK_BUF_SIZE = 1024 * 1024 / sizeof(RX_BLOCK);
				RX_BLOCK * rxBlockBuf = new RX_BLOCK[RX_BLOCK_BUF_SIZE];
				int rxBlockBufIdx = 0;

				while(index < sharedSourceInfo->wIdx)
				{
					//TODO Set the valid bit??
					RX_BLOCK * pBlock = rxBlockBuf + rxBlockBufIdx;
					pBlock->u.Desc.VStreamBits = -1;
					int i;
					for (i = 0; i < 28; i++)
					{
						PCOMPLEX16 pSample = (PCOMPLEX16)&(pBlock->u.SampleBlock);
						pSample[i] = *((COMPLEX16*)(sharedSourceData + index));
						index += sizeof(COMPLEX16);
						if (index >= sharedSourceInfo->wIdx)
							break;
					}
					if (i == 28)	// a full block
					{
						rxBlockBufIdx++;
						if (rxBlockBufIdx == RX_BLOCK_BUF_SIZE)
						{
							fwrite(rxBlockBuf, sizeof(RX_BLOCK), RX_BLOCK_BUF_SIZE, fp);
							rxBlockBufIdx = 0;
						}
					}
					else
					{
						completeBlock = false;
						break;
					}
				}

				if (rxBlockBufIdx > 0)
				{
					fwrite(rxBlockBuf, sizeof(RX_BLOCK), rxBlockBufIdx, fp);
				}

				fclose(fp);

				delete [] rxBlockBuf;
			}
			else
			{
				CString errMsg;
				errMsg.Format(L"Open file error: %x\n", ret);
				::AfxMessageBox(errMsg);
			}

			process->smSourceInfo->Unlock();
		}	
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}
