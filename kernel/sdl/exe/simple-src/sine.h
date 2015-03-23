#pragma once

#include <brick.h>
#include <stdfacade.h>
#include <stdbrick.hpp>
#include <sdl.hpp>
#include <thread_if.h>

#define TARGET_RADIO 0

ISource* CreateGraph ();

class CSineSource :
	public LOCAL_CONTEXT(TSine),
	public LOCAL_CONTEXT(TModSink)
{
private:
	ISource* m_source;
	bool     m_bRunning;
	ULONG    m_signalID;
	HANDLE   m_thread;
	
public:
	bool& Running () { return m_bRunning; };

public:
	CSineSource () { 
		m_source   = NULL; 
		m_bRunning = false;
		m_signalID = 0;
		m_thread   = NULL;
	}

	~CSineSource () { 
		Stop ();
	}

	bool Create ( void * pTxSampleBuffer, int TxSampleBufSize ) {
		m_source = CreateGraph ();

		CF_TxSampleBuffer::tx_sample_buf()      = (COMPLEX8*)pTxSampleBuffer;
		CF_TxSampleBuffer::tx_sample_buf_size() = TxSampleBufSize;
		CF_Error::error_code () = E_ERROR_SUCCESS;

		m_source->Reset ();
		m_source->Process ();

		if ( CF_Error::error_code () != E_ERROR_SUCCESS) {
			// some error occurs
			return false;
		}

		// transfer the generated signal down to RCB
		HRESULT	hr = SoraURadioTransferEx(TARGET_RADIO, 
							pTxSampleBuffer, 
							CF_TxSampleBuffer::tx_sample_cnt()*sizeof(COMPLEX8), 
							&m_signalID );
		if ( FAILED(hr) ) {
			printf ( "Transfer failed!\n" );
			return false;
		}
		return true;
	}

	bool SendSignal () {
		if ( m_signalID == 0 ) return false;

		HRESULT hr = SoraURadioTx (TARGET_RADIO, m_signalID );
		if ( FAILED(hr)) {
			printf ( "Transmission failed!\n" );
			return false;
		}
		return true;
	}
	
	static BOOLEAN ThreadProc ( VOID* pParam ) {
		CSineSource* pSine = (CSineSource*) pParam;
		if ( !pSine->Running() ) {
			return false;
		} else 
			return pSine->SendSignal ();
	};

	void Stop () {
		if ( m_source ) {
			IReferenceCounting::Release (m_source);
			m_source = NULL;
		} 

		if ( m_signalID ) {
			SoraURadioTxFree ( TARGET_RADIO, m_signalID );
			m_signalID = 0;
		}

		if ( m_thread ) {
			SoraThreadStop (m_thread);
			SoraThreadFree (m_thread);
			m_thread = NULL;
		}
	}
	
	bool Start () {
		if ( m_signalID == 0) {
			return false;			
		}

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

};
