#pragma once

#include <functional>
#include <set>

// Invokable

class Invokable : public CWnd
{
	DECLARE_DYNAMIC(Invokable)

public:
	Invokable();
	virtual ~Invokable();
	bool Invoke(const std::function<void(void)> & f);

protected:
	afx_msg LRESULT OnInvoke(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CRITICAL_SECTION cs;
	volatile bool _wndExist;
public:

private:
	volatile unsigned long _count;
	std::set<std::function<void(void)> *> _funcSet;
public:
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


