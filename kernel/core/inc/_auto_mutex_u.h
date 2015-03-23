#pragma once

#include "sora.h"

class AutoMutex {

public:
	AutoMutex(HANDLE Mutex) : 
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
			WaitForSingleObject(m_Mutex, 
				INFINITE);
	}

	inline
	void 
	Unlock() {

		if (m_Mutex) 
			ReleaseMutex(m_Mutex);
	}

	inline
	void
	Detach() {

		m_Mutex = NULL;
	}

protected:
	HANDLE m_Mutex;
};
