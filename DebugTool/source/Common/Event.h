#pragma once

#include <map>
#include <algorithm>
#include <functional>
#include <memory>
#include <Windows.h>
#include "CSRecursiveLock.h"

namespace SoraDbgPlot { namespace Event {

#define CALLBACK_TYPE std::function<void(const void *, const T &)>

#ifdef _DEBUG
int __stdcall EventHandlerCount();
void __stdcall EventHandlerCountModify(int count, int sign);
#endif

template <typename T>
class Event
{
public:
	Event()
	{
#ifdef _DEBUG
		_fortest = new char[13];
#endif
	}

	~Event()
	{
		Reset();
	}

	HANDLE Subscribe(const std::function<void(const void *, const T &)> & f) {
		auto fp = std::make_shared<CALLBACK_TYPE>(f);
		_lock.Lock();
		_callbacks.insert(
			std::make_pair(
				(HANDLE)fp.get(), fp
			)
		);

#ifdef _DEBUG
		SoraDbgPlot::Event::EventHandlerCountModify(1, true);
#endif

		_lock.Unlock();
		return (HANDLE)fp.get();
	}

	bool UnSubscribe(HANDLE h) {

		bool ret = false;

		_lock.Lock();

		auto iter = _callbacks.find(h);
		if (iter != _callbacks.end())
		{
			_callbacks.erase(iter);
#ifdef _DEBUG
			SoraDbgPlot::Event::EventHandlerCountModify(1, 0);
#endif
			ret = true;
		}

		_lock.Unlock();

		return ret;
	}

	void Raise(const void * sender, const T & e) const {

		_lock.Lock();
		std::for_each(_callbacks.begin(), _callbacks.end(), [sender, e](std::pair<HANDLE, std::shared_ptr<CALLBACK_TYPE> > pair){
			(*(pair.second))(sender, e);
		});
		_lock.Unlock();

	}

	void Reset()
	{
		_lock.Lock();
		int handlerCount = _callbacks.size();
		_callbacks.clear();

#ifdef _DEBUG
		SoraDbgPlot::Event::EventHandlerCountModify(handlerCount, 0);
		if (_fortest) {
			delete [] _fortest;
			_fortest = 0;
		}
#endif		

		_lock.Unlock();

		
	}

private:
	std::map<HANDLE, std::shared_ptr<CALLBACK_TYPE > > _callbacks;
	mutable SoraDbgPlot::Lock::CSRecursiveLock _lock;
#ifdef _DEBUG
	char * _fortest;
#endif
};

#undef CALLBACK_TYPE

}}
