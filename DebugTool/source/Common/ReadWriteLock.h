#pragma once

#include "CSRecursiveLock.h"

namespace SoraDbgPlot { namespace Lock {

	class RWLock
	{
	public:
		RWLock()
		{
			_readCount = 0;
		}
		~RWLock()
		{
		}

		void LockWrite()
		{
			_wlock.Lock();
		}

		void UnlockWrite()
		{
			_wlock.Unlock();
		}

		void LockRead()
		{
			_rlock.Lock();
			_readCount++;
			if (_readCount == 1)
				_wlock.Lock();
			_rlock.Unlock();
		}

		void UnlockRead()
		{
			_rlock.Lock();
			_readCount--;
			if (_readCount == 0)
				_wlock.Unlock();
			_rlock.Unlock();
		}

	private:
		SoraDbgPlot::Lock::CSLock _rlock;
		SoraDbgPlot::Lock::CSLock _wlock;
		int _readCount;
	};

}}
