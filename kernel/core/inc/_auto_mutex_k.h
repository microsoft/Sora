#pragma once

#include <wdm.h>

class AutoMutex {

public:
	AutoMutex(PRKMUTEX Mutex) : 
		m_Mutex(Mutex) {

		Lock();
	}
   ~AutoMutex() {

		Unlock();
		Detach();
   	}

	inline
	void 
	Lock() {

		if (m_Mutex)
			KeWaitForMutexObject(m_Mutex, 
				Executive,
				KernelMode,
				FALSE,
				NULL);
	}

	inline
	void 
	Unlock() {

		if (m_Mutex)
			KeReleaseMutex(m_Mutex, 
				FALSE);
	}

	inline
	void
	Detach() {

		m_Mutex = NULL;
	}

protected:
	PRKMUTEX m_Mutex;
};
