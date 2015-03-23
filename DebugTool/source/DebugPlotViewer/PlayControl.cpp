#include "stdafx.h"

#include <algorithm>
#include "PlayControl.h"
#include "Logger.h"
#include "DebugPlotViewerDoc.h"
#include "AppMessage.h"
#include "SharedObjManager.h"

CPlayControlWnd::CPlayControlWnd() {};

CPlayControlWnd::~CPlayControlWnd() {};

BEGIN_MESSAGE_MAP(CPlayControlWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_APP, OnApp)
END_MESSAGE_MAP()

int CPlayControlWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	return 0;
}

void CPlayControlWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();

	CRect rect;
	GetClientRect(&rect);
	for (unsigned int i = 0; i < controllerWnds.size(); i++)
	{
		SetLayout(controllerWnds[i]);
		this->CalcYLayout(&rect, i);
		controllerWnds[i]->MoveWindow(&rect, 1);
	}
}

void CPlayControlWnd::OnPaint()
{
	CPaintDC dc(this);
	Graphics graphics(dc.m_hDC);

	CRect rect;
	GetClientRect(&rect);
	Bitmap bmp(rect.right,rect.bottom);
	Graphics* memGraph = Graphics::FromImage(&bmp);

	Color color(50, 50, 50);

	SolidBrush brush(color);
	memGraph->FillRectangle(
		&brush, 
		rect.left, 
		rect.top,
		rect.right,
		rect.bottom
		); 

	graphics.DrawImage(&bmp,rect.left,rect.top,rect.right,rect.bottom);
	delete memGraph;
}

void CPlayControlWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	CDockablePane::OnMouseMove(nFlags, point);
}

LRESULT CPlayControlWnd::OnApp(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case CMD_NEW_DOCUMENT:
		this->doc = (CDebugPlotViewerDoc *)lParam;
		break;
	case CMD_CLEAR_DOCUMENT:
		{
			for (unsigned int i = 0; i < process.size(); i++)
			{
				process[i]->DecRefCount();
				controllerWnds[i]->ReleaseProcess();
				controllerWnds[i]->ShowWindow(false);
			}
			process.clear();
		}
		break;
	case CMD_UPDATE_CONTROLLER:
		this->UpdateProcessList();
		break;
	case CMD_CONTROLLER_TIMER:
		{
			std::vector<ControllerWnd *>::iterator iterController;
			for (iterController = this->controllerWnds.begin();
				iterController != this->controllerWnds.end();
				iterController++)
			{
				ControllerWnd * wnd = *iterController;
				wnd->InvalidateRgn(NULL, 1);
			}
		}
		break;
	}
	return 0;
}

void CPlayControlWnd::UpdateProcessList()
{
	SharedObjManager * manager = SharedObjManager::Instance();

	// remove all
	std::vector<DebuggingProcessProp *>::iterator iterProcess;
	for (iterProcess = this->process.begin(); iterProcess != this->process.end();iterProcess++)
	{
		(*iterProcess)->DecRefCount();
	}
	this->process.resize(0);

	manager->Lock();
	std::multimap<wstring, DebuggingProcessProp *>::iterator iterAllProcess;
	for (iterAllProcess = manager->allProcessObjs.begin();
		iterAllProcess != manager->allProcessObjs.end();
		iterAllProcess++)
	{
		DebuggingProcessProp * processProp = (*iterAllProcess).second;
		if ( processProp->openCount > 0 )
		{
			processProp->IncRefCount();
			process.push_back(processProp);
		}
	}
	manager->Unlock();

	std::sort(process.begin(), process.end(), [](DebuggingProcessProp* prop1, DebuggingProcessProp* prop2)->bool {
		CString nameProp1;
		nameProp1.Format(L"%s(%d)", prop1->moduleName, prop1->pid);
		CString nameProp2;
		nameProp2.Format(L"%s(%d)", prop2->moduleName, prop2->pid);
		bool less = nameProp1.Compare(nameProp2) < 0;
		return less;
	});

	unsigned int maxI = max(process.size(), controllerWnds.size());

	for (unsigned int i = 0; i < maxI; i++)
	{
		if (i >= controllerWnds.size())		// but less than process count, should increase controller bar
		{
			ControllerWnd * controllerWnd = new ControllerWnd;
			controllerWnds.push_back(controllerWnd);
			CRect rectDummy;
			rectDummy.SetRectEmpty();
			controllerWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, rectDummy, this, 0, 0);
			SetLayout(controllerWnd);
			CRect rect;
			GetClientRect(&rect);
			this->CalcYLayout(&rect, i);
			controllerWnd->MoveWindow(&rect, 1);
			controllerWnd->InvalidateRgn(NULL, 1);			
		}
		if (i < process.size())
		{
			controllerWnds[i]->SetProcess(process[i]);
			process[i]->trackbarWnd = controllerWnds[i];
		}
	}

	if (process.size() < controllerWnds.size())
	{
		for (unsigned int i = process.size(); i < controllerWnds.size(); i++)
		{
			controllerWnds[i]->ShowWindow(false);
			controllerWnds[i]->ReleaseProcess();
		}
	}
	else if (process.size() > controllerWnds.size())
	{
		for (unsigned int i = controllerWnds.size(); i < process.size(); i++)
		{
			ControllerWnd * controllerWnd = new ControllerWnd;
			controllerWnds.push_back(controllerWnd);
			CRect rectDummy;
			rectDummy.SetRectEmpty();
			controllerWnd->CreateEx(0, NULL, L"", WS_CHILD | WS_VISIBLE, rectDummy, this, 0, 0);
			SetLayout(controllerWnd);
			CRect rect;
			GetClientRect(&rect);
			this->CalcYLayout(&rect, i);
			controllerWnd->MoveWindow(&rect, 1);
			controllerWnd->InvalidateRgn(NULL, 1);
		}
	}

	for (unsigned int i = 0; i < process.size(); i++)
	{
		controllerWnds[i]->SetProcess(process[i]);
		process[i]->trackbarWnd = controllerWnds[i];
		controllerWnds[i]->ShowWindow(true);
	}
	

}

void CPlayControlWnd::AdjustLayout()
{
	CRect rect;

	/*

	||pid|-----------------bar------------------|up|down|play|auto||

	*/
	const int MARGIN = 0;
	const int INTERVAL = 0;
	const int BUTTON_WIDTH = 40;
	const int PID_WIDTH = 100;
	const int MIN_BAR_WIDTH = 200;
	const int MIN_WND_WIDTH = MARGIN*2 + PID_WIDTH + BUTTON_WIDTH*4 + MIN_BAR_WIDTH + INTERVAL*5;

	this->CalcClientRect(&rect);
	int wndWidth = max(rect.Width(), MIN_WND_WIDTH);

	this->pidLeft = rect.left;
	this->pidRight = this->pidLeft + PID_WIDTH;
	
	this->playLeft = this->pidRight;
	this->playRight = this->playLeft + BUTTON_WIDTH;

	this->barLeft = this->playRight;
	this->barRight = wndWidth;

	//this->autoPlayRight = wndWidth - MARGIN;
	//this->autoPlayLeft = this->autoPlayRight - BUTTON_WIDTH;

	//this->playRight = this->autoPlayLeft - INTERVAL;
	//this->playLeft = this->playRight - BUTTON_WIDTH;

	//this->speedDownRight = this->playLeft - INTERVAL;
	//this->speedDownLeft = this->speedDownRight - BUTTON_WIDTH;

	//this->speedUpRight = this->speedDownLeft - INTERVAL;
	//this->speedUpLeft = this->speedUpRight - BUTTON_WIDTH;

	//this->barLeft = this->pidRight + INTERVAL;
	//this->barRight = this->speedUpLeft - INTERVAL;
}

void CPlayControlWnd::CalcYLayout(CRect * rect, int index)
{
	const int MARGIN = 2;
	const int INTERVAL = 1;
	const int HEIGHT = 44;
	rect->top = MARGIN + (HEIGHT + INTERVAL) * index;
	rect->bottom = rect->top + HEIGHT;
}

void CPlayControlWnd::CalcClientRect(CRect * rect)
{
	ASSERT(rect);

	this->GetClientRect(rect);
}

void CPlayControlWnd::CalcCaptionRect(CRect * rect, int index)
{
	ASSERT(rect);

	rect->left = this->pidLeft;
	rect->right = this->pidRight;
	CalcYLayout(rect, index);
}

void CPlayControlWnd::CalcBarRect(CRect * rect, int index)
{
	ASSERT(rect);
	rect->left = this->barLeft;
	rect->right = this->barRight;
	CalcYLayout(rect, index);
}

//void CPlayControlWnd::CalcSpeedUpButtonRect(CRect * rect, int index)
//{
//	ASSERT(rect);
//	rect->left = this->speedUpLeft;
//	rect->right = this->speedUpRight;
//	CalcYLayout(rect, index);
//}
//
//void CPlayControlWnd::CalcSpeedDownButtonRect(CRect * rect, int index)
//{
//	ASSERT(rect);
//	rect->left = this->speedDownLeft;
//	rect->right = this->speedDownRight;
//	CalcYLayout(rect, index);
//}

void CPlayControlWnd::CalcPlayButtonRect(CRect * rect, int index)
{
	ASSERT(rect);
	rect->left = this->playLeft;
	rect->right = this->playRight;
	CalcYLayout(rect, index);
}

void CPlayControlWnd::SetLayout(ControllerWnd * controllerWnd)
{
	controllerWnd->SetLayout(
		this->pidLeft,
		this->pidRight,
		this->barLeft,
		this->barRight,
		//this->speedUpLeft,
		//this->speedUpRight,
		//this->speedDownLeft,
		//this->speedDownRight,
		this->playLeft,
		this->playRight
		//this->autoPlayLeft,
		//this->autoPlayRight
	);
}

void CPlayControlWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	for (unsigned int i = 0; i < this->process.size(); i++)
	{
		this->process[i]->DecRefCount();
	}

	for (unsigned int i = 0; i < this->controllerWnds.size(); i++)
	{
		delete this->controllerWnds[i];
	}

	CDockablePane::PostNcDestroy();
}
