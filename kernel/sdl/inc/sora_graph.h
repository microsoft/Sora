#pragma once

#include <sora.h>
#include <brick.h>
#include <thread_func.h>

//
// template TInnerObject
//
template<typename TOutter>
struct TInnerObject {
	TOutter& m_This;
	TInnerObject ( TOutter& Outter) : m_This(Outter) {};
	TOutter& This() { return m_This; }
};
	
//
// CSoraThread - managing a Sora EThread
//
template<typename TWorker>
class CSoraThread {
	HANDLE   m_hThread;
	PVOID    m_pParam;
	TWorker* m_pWorker;
public:
	CSoraThread () {
		m_hThread = NULL;
		m_pParam  = NULL;
		m_pWorker = NULL;
	}

	~CSoraThread () {
		Stop ();
	}

	bool Start (TWorker * pWorker, LPVOID pParam) {
		if ( m_hThread ) return false;

		m_pParam  = pParam;
		m_pWorker = pWorker;

		m_hThread = AllocStartThread (proc, this );
		if ( m_hThread == 0 ) {
			m_pParam  = NULL;
			m_pWorker = NULL;
			return false;
		}

		return true;
	}

	bool Stop () {
		if ( m_hThread ) {
			SoraThreadStop (m_hThread);

			m_hThread = NULL;
			m_pParam  = NULL;
			m_pWorker = NULL;			
		}

		return true;
	}

static BOOLEAN proc ( LPVOID pParam ) {
		CSoraThread * pThis = (CSoraThread*) pParam;
		
		return (*(pThis->m_pWorker))( pThis->m_pParam );
	}
};

//
// CSoraGraph - managing the context as well as the processing graph
//
class CSoraGraph {
// interface
public:
	// Create a processing graph
	ISource* CreateGraph () { return false; };

	// processing function
	BOOLEAN operator() ( LPVOID pParam ) { return false; }

	// We need to reset the context as well as the processing graph
	bool Reset () { return false;}
	// Default behavior: just flush the processing graph
	void Flush () {};
	
};

