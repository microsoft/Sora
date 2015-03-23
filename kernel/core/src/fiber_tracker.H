#pragma once

#include <assert.h>
#include <Windows.h>
#include "const.h"
#include "thread_if.h"

// TRICK: strange that pre-Vista OSes does NOT implement IsThreadAFiber().
// We implement it by thread local storage (TLS) variable, and hacking
// fiber APIs
#if (_WIN32_WINNT < 0x0600)
enum TlsValue_FiberFlag
{
    FF_NORMAL_THREAD = 0,
    FF_FIBER_THREAD
};

SELECTANY DWORD TlsIndex_FiberFlag = TlsAlloc(); // default value is 0(FF_NORMAL_THREAD)

inline BOOL IsThreadAFiber(void)
{
    if (TlsIndex_FiberFlag == TLS_OUT_OF_INDEXES) return FALSE;
    return TlsGetValue(TlsIndex_FiberFlag) == (LPVOID)FF_FIBER_THREAD;
}

static inline LPVOID ConvertThreadToFiber_(LPVOID lpParameter)
{
    if (TlsIndex_FiberFlag == TLS_OUT_OF_INDEXES) return NULL;

    LPVOID rc = ConvertThreadToFiber(lpParameter);
    if (rc == NULL) return rc;

    TlsSetValue(TlsIndex_FiberFlag, (LPVOID)FF_FIBER_THREAD);
    assert(TlsGetValue(TlsIndex_FiberFlag) == (LPVOID)FF_FIBER_THREAD);
    return rc;
}
#define ConvertThreadToFiber ConvertThreadToFiber_

static inline BOOL ConvertFiberToThread_()
{
    BOOL suc = ConvertFiberToThread();
    if (!suc) return suc;

    if (TlsIndex_FiberFlag == TLS_OUT_OF_INDEXES) return suc;
    TlsSetValue(TlsIndex_FiberFlag, (LPVOID)FF_NORMAL_THREAD);
    assert(TlsGetValue(TlsIndex_FiberFlag) == (LPVOID)FF_NORMAL_THREAD);
    return suc;
}
#define ConvertFiberToThread ConvertFiberToThread_
#endif

class fiber_tracker
{
    void *_fiber;
    void *_next;
	unsigned long _ref;

    PSORA_UTHREAD_PROC _user_routine;
    void *_user_context;
    BOOLEAN _user_return;

    static void CALLBACK fiber_func(void *tracker_)
    {
        fiber_tracker *tracker = (fiber_tracker *)tracker_;
        assert(tracker->_user_routine != NULL);

        for (;;)
        {
            BOOLEAN rc = tracker->_user_routine(tracker->_user_context);
            SoraThreadYield(rc);
        }
    }

	~fiber_tracker()
	{
		if (is_valid())
		{
			DeleteFiber(_fiber);
		}
	}

public:
    fiber_tracker(PSORA_UTHREAD_PROC func, void *arg)
        : _user_context(arg)
        , _user_routine(func)
        , _next(NULL)
        , _ref(1)
    {
        _fiber = CreateFiber(0, fiber_func, this);
    }

	unsigned long addref() {
		
		return InterlockedIncrement(&_ref);
	}

	unsigned long release() {

		unsigned long ref;
		ref = InterlockedDecrement(&_ref);
		if (!ref)
			delete this;
		return ref;
	}

	PSORA_UTHREAD_PROC get_user_routine() {

		return _user_routine;
	}

	void* get_user_context() {

		return _user_context;
	}

    bool is_valid()
    {
        return _fiber != NULL;
    }

    void *link_next(fiber_tracker *nfibertracker)
    {
        void *old = _next;
        _next = nfibertracker->_fiber;
        return old;
    }

    void *link_next(void *nfiber)
    {
        void *old = _next;
        _next = nfiber;
        return old;
    }

    FINL BOOLEAN fiber_call()
    {
        assert(IsThreadAFiber());
        assert(is_valid());
		link_next(GetCurrentFiber());
        SwitchToFiber(_fiber);
        return _user_return;
    }

    FINL static BOOLEAN fiber_call(void *self_)
    {
        fiber_tracker *self = (fiber_tracker *)self_;
        return self->fiber_call();
    }

    friend void SoraThreadYield(BOOLEAN rc = TRUE);
};

FINL void SoraThreadYield(BOOLEAN rc)
{
    // Possible called by non-fiber thread, just ignore yield request and return control to caller
    // Caller normally call this in a loop and thus result to a busy wait
    if (!IsThreadAFiber()) return;

    fiber_tracker *tracker = (fiber_tracker *)GetFiberData();
    tracker->_user_return = rc;

    assert(tracker->_next);
    SwitchToFiber(tracker->_next);
}
