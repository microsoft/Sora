#pragma once

#include <map>
#include <algorithm>
#include <functional>
#include <memory>
#include <Windows.h>
#include "CSRecursiveLock.h"

namespace SoraDbgPlot { namespace Strategy {

template <typename T, typename R>
class Strategy
{
	typedef std::function<void(const void *, const T &, R &)> CallbackType;
public:
	void Set(const CallbackType & f) {
		_lock.Lock();
		_callback.reset();
		_callback = std::make_shared<CallbackType>(f);
		_lock.Unlock();
	}

	void UnSet() {
		_lock.Lock();
		_callback.reset();
		_lock.Unlock();
	}

	bool Call(const void * sender, const T & e, R & r) {

		bool ret = false;

		_lock.Lock();

		if (_callback.get())
		{
			(*_callback)(sender, e, r);
			ret = true;
		}

		_lock.Unlock();

		return ret;
	}

	void Reset()
	{
		_lock.Lock();
		_callback.reset();
		_lock.Unlock();
	}

private:
	std::shared_ptr<CallbackType > _callback;
	SoraDbgPlot::Lock::CSRecursiveLock _lock;
};

}}
