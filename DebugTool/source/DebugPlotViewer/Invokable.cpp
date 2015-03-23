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
	_count = 0;
	_wndExist = false;
	::InitializeCriticalSection(&cs);
}

Invokable::~Invokable()
{
	::DeleteCriticalSection(&cs);

	std::for_each(_funcSet.begin(), _funcSet.end(), [this](std::function<void(void)> * f){
		delete f;
		::InterlockedDecrement(&this->_count);
	});

	ASSERT(_count == 0);
}


BEGIN_MESSAGE_MAP(Invokable, CWnd)
	ON_MESSAGE(WM_INVOKE, OnInvoke)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// Invokable message handlers

bool Invokable::Invoke(const std::function<void(void)> & f)
{
	bool ret = true;
	::EnterCriticalSection(&cs);

	if (_wndExist)
	{
		auto pf = new std::function<void(void)>(f);
		_funcSet.insert(pf);
		BOOL succ = this->PostMessageW(WM_INVOKE, 0, (LPARAM)pf);
		if (succ)
		{
			::InterlockedIncrement(&_count);
		}
		else
		{
			auto iter = _funcSet.find(pf);
			if (iter != _funcSet.end())
				_funcSet.erase(iter);

			delete pf;
			ret = false;

			::InterlockedDecrement(&_count);
		}
	}
	else
		ret = false;

	::LeaveCriticalSection(&cs);

	return ret;
}

LRESULT Invokable::OnInvoke(WPARAM wParam, LPARAM lParam)
{
	auto pf = (std::function<void(void)> *)lParam;
	(*pf)();
	delete pf;

	::EnterCriticalSection(&cs);

	auto iter = _funcSet.find(pf);
	if (iter != _funcSet.end())
		_funcSet.erase(iter);

	::InterlockedDecrement(&_count);

	::LeaveCriticalSection(&cs);

	return 0;
}

void Invokable::OnDestroy()
{
	CWnd::OnDestroy();

	::EnterCriticalSection(&cs);
	_wndExist = false;
	::LeaveCriticalSection(&cs);
}

int Invokable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	::EnterCriticalSection(&cs);
	_wndExist = true;
	::LeaveCriticalSection(&cs);

	return 0;
}
