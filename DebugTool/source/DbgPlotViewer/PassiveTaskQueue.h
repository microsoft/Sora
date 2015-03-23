#pragma once

#include <list>
#include <functional>
#include "CSLock.h"

class PassiveTaskQueue
{
public:
	void Queue(const std::function<void(bool)> & f)
	{
		_lock.Lock();
		_list.push_back(f);
		_lock.Unlock();
	}

	void Execute(bool bClose)
	{
		_lock.Lock();
		while(_list.size() > 0)
		{
			auto f = _list.front();
			f(bClose);
			_list.pop_front();
		}
		_lock.Unlock();
	}

private:
	std::list<std::function<void(bool)> > _list;
	SoraDbgPlot::Lock::CSLock _lock;
};
