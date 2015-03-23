#pragma once
#include <brick.h>
#include <stdfacade.h>
#include <stdbrick.hpp>
#include <thread_if.h>

#define TARGET_RADIO 0

ISource* CreateGraph ();

class CMonitor : 
    public LOCAL_CONTEXT(TDropAny),
    public LOCAL_CONTEXT(TRxStream),
    public LOCAL_CONTEXT(TAvePower),
    public LOCAL_CONTEXT(TDCRemoveEx)
{
private:
    ISource * m_pSrc;
    bool      m_bRunning;
	HANDLE 	  m_thread;

public:
    ISource*& Source () { return m_pSrc; }
    bool &    IsRunning() { return m_bRunning; }

public:
    CMonitor () {
        m_pSrc = NULL;
		m_thread = 0; // ??
		m_bRunning = false;
        ResetContext ();
    }
    
public:
    void ResetContext () {
        // CF_Error
        CF_Error::error_code() = E_ERROR_SUCCESS;
    }

    void Reset () {
        ResetContext ();
        if ( m_pSrc ) {
            m_pSrc->Reset ();
        }
    }

    bool Create ( PSORA_RADIO_RX_STREAM pRxStream ) {
        m_pSrc = CreateGraph (); 

		// CF_RxStream
		CF_RxStream::rxstream_pointer() = pRxStream;
		CF_RxStream::rxstream_touched() = 0;

        return true;
    }

	static BOOLEAN ThreadProc ( VOID* pParam ) {
    	CMonitor* pMon = (CMonitor*) pParam;
	    if ( !pMon->IsRunning() ) {
			return false;
	    } else 
			return pMon->Source()->Process ();
	};
	
	bool Start () {
		if ( m_pSrc == NULL )
			return false;
		
		m_thread = SoraUThreadAlloc();
		if (!m_thread) {
			return false;
		}

		m_bRunning = true;
		if (!SoraUThreadStart(m_thread, ThreadProc, this)) {
			SoraUThreadStop(m_thread);
			SoraUThreadFree(m_thread);
			
			m_bRunning = false;	
			return false;
		}

		return true;
	}

	bool Stop () {
		m_bRunning = false;		

		if ( m_thread ) {
			SoraUThreadStop(m_thread);
			SoraUThreadFree(m_thread);
			m_thread = NULL;
		}

		return true;
	}

};

void ConfigureRadio ();
void process_kb (); 


