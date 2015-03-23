// Invokable.cpp : implementation file
//

#include "stdafx.h"
#include "Invokable.h"

#include <algorithm>

#define WM_INVOKE (WM_APP + 7)

// Invokable

IMPLEMENT_DYNAMIC(Invokable, CWnd)

Invokable::Invokable()
{
	::InitializeCriticalSection(&cs);
}

Invokable::~Invokable()
{
	::DeleteCriticalSection(&cs);

	for (auto iter = _funcList.begin();
		iter != _funcList.end();
		++iter)
	{
		(*iter)(true);
	}

	_funcList.clear();
}


BEGIN_MESSAGE_MAP(Invokable, CWnd)
	ON_MESSAGE(WM_INVOKE, OnInvoke)
	ON_WM_DESTROY()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// Invokable message handlers

bool Invokable::Invoke(const std::function<void(bool close)> & f)
{
	bool ret = true;
	::EnterCriticalSection(&cs);
	
	_funcList.push_back(f);

	::LeaveCriticalSection(&cs);

	HWND hWnd = m_hWnd;
	if (hWnd)
	{
		BOOL succ = ::PostMessage(hWnd, WM_INVOKE, 0, 0);
		ret = succ == TRUE ? true : false;
	}

	return ret;
}

LRESULT Invokable::OnInvoke(WPARAM wParam, LPARAM lParam)
{
	
	std::list<std::function<void(bool)> > exeList;

	::EnterCriticalSection(&cs);

	exeList = _funcList;
	_funcList.clear();

	::LeaveCriticalSection(&cs);

	for (auto iter = exeList.begin();
		iter != exeList.end();
		++iter)
	{
		(*iter)(false);
	}

	return 0;
}

void Invokable::OnDestroy()
{
	CWnd::OnDestroy();
}

int Invokable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	HWND hWnd = m_hWnd;
	if (hWnd)
		::PostMessage(hWnd, WM_INVOKE, 0, 0);

	return 0;
}
