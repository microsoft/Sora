#pragma once

#include <functional>
#include <set>
#include <list>

// Invokable

class Invokable : public CWnd
{
	DECLARE_DYNAMIC(Invokable)

public:
	Invokable();
	virtual ~Invokable();
	bool Invoke(const std::function<void(bool)> & f);

protected:
	afx_msg LRESULT OnInvoke(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CRITICAL_SECTION cs;
public:

private:
	std::list<std::function<void(bool)> > _funcList;

public:
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


