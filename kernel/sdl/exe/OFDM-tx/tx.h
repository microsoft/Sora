#pragma once

using namespace std;

#include <brick.h>
#include <stdfacade.h>
#include <stdbrick.hpp>
#include <sdl.hpp>
#include <thread_if.h>

#include <list>

#include "ieee80211facade.hpp"
#include "conv_enc.hpp"
#include "interleave.hpp"
#include "mapper11a.hpp"
#include "pilot.hpp"
#include "fft.hpp"
#include "preamble11a.hpp"
#include "PHY_11a.hpp"
#include "sampling.hpp"
#include "scramble.hpp"

#define TARGET_RADIO 0

ISource* CreateTSGraph ();
ISource* CreateModGraph ();

struct PKT_INFO {
	PACKET_HANDLE PktID;
	ULONG         TxID;
};

class COFDMTxChain :
	public LOCAL_CONTEXT(TDropAny),
	public LOCAL_CONTEXT(TModSink),
	public LOCAL_CONTEXT(T11aSc),
	public LOCAL_CONTEXT(TBB11aSrc)
{
private:
	ISource* m_preamble;
	ISource* m_source;
	
	bool     m_bRunning;
	bool     m_bStop;	
	HANDLE   m_thread;

	typedef std::list<PKT_INFO> TXQUEUE;
	TXQUEUE m_txqueue;

	void * m_txsamplebuf;
	int    m_txbufsize;
	
public:
	bool& Running () { return m_bRunning; };

public:
	COFDMTxChain ( ) { 
		m_source   = NULL; 
		m_bRunning = false;
		m_bStop    = false;
		m_thread   = NULL;
	}

	~COFDMTxChain () { 
		Stop ();
	}

	bool Create ( void * pTxSampleBuffer, int TxSampleBufSize ) {
		m_preamble = CreateTSGraph ();
		m_source   = CreateModGraph ();

		m_txsamplebuf   = pTxSampleBuffer;
		m_txbufsize     = TxSampleBufSize;

		return true;
	}

	int ModulatePacket ( uchar* pMPDU, int MPDUSize )
	{
		// we will Encapsulate packet and modulate it

		// Call the processing chain to modulate
		
        // CF_TxSampleBuf
        CF_TxSampleBuffer::tx_sample_buf () 	 = (COMPLEX8*)m_txsamplebuf; 
        CF_TxSampleBuffer::tx_sample_buf_size () = m_txbufsize; 

		CF_Error::error_code() = E_ERROR_SUCCESS;
		// add preamble
		m_preamble->Reset   ();
		m_preamble->Process ();

		uint preamble_size = CF_TxSampleBuffer::tx_sample_cnt();
		
		CF_TxSampleBuffer::tx_sample_buf () 	 = (COMPLEX8*)m_txsamplebuf + 
												   preamble_size;
		CF_TxSampleBuffer::tx_sample_buf_size () = m_txbufsize - 
												   preamble_size*sizeof(COMPLEX8);

		
        // TxVector
        CF_11aTxVector::frame_length()    = (short) MPDUSize; 
        CF_11aTxVector::data_rate_kbps () = 6000; // 6Mbps
        CF_11aTxVector::crc32 () = CalcCRC32(pMPDU, MPDUSize ); 
    
        // TxFrameBuffer
        CF_TxFrameBuffer::mpdu_buf0 () = pMPDU; 
        CF_TxFrameBuffer::mpdu_buf1 () = NULL;
        CF_TxFrameBuffer::mpdu_buf_size0 () = (short)MPDUSize; 
        CF_TxFrameBuffer::mpdu_buf_size1 () = 0; 
    
        // CF_Error
        CF_Error::error_code () = E_ERROR_SUCCESS; 
    
        // CF_Scrambler
        CF_ScramblerSeed::sc_seed() = 0xFF;

		m_source->Reset   ();
		m_source->Process ();
		m_source->Flush   ();

		if ( CF_Error::error_code () != E_ERROR_SUCCESS ) {
			printf ( "modulation error!\n" );
			return 0;
		}
		
		uint sample_size = preamble_size + CF_TxSampleBuffer::tx_sample_cnt();
		
		return sample_size;
	}

	bool Work () {
		// Check if there is any packet available
		if ( m_bStop ) {
			m_bRunning = false;
			return false;
		}
		
		PACKET_HANDLE packet;
		PUCHAR addr;
		UINT len;
		HRESULT hr;
		
		hr = SoraUGetTxPacket(&packet, (VOID**)&addr, &len, 10);
		if (hr == S_OK) {
			printf ( "Get a packet... len %d\n", len );
			
			PKT_INFO info;
			info.PktID = packet;
			info.TxID  = 0;

			int sampleCount = ModulatePacket ( addr, len );
			printf ( "Modulated samples %d\n", sampleCount );
			
			if ( sampleCount == 0 ) {
				// any thing failed
				SoraUCompleteTxPacket ( info.PktID, STATUS_SUCCESS );
			} else {
				// transfer
				ULONG txid;
				hr = SoraURadioTransferEx( TARGET_RADIO, 
					 						m_txsamplebuf, 
					 						sampleCount*sizeof(COMPLEX8), 
					 						&txid );
				info.TxID = txid;
				
				if ( FAILED(hr)) {
					// Fail to transfer
					printf ( "Failed to transfer!\n");
					SoraUCompleteTxPacket ( info.PktID, STATUS_SUCCESS );		
					return true;
				} 
				
				// immediate tx
				hr = SoraURadioTx(TARGET_RADIO, txid);

				if ( FAILED(hr)) {
					printf ( "Failed to TX!\n");
					SoraUCompleteTxPacket ( info.PktID, STATUS_SUCCESS );		
					SoraURadioTxFree ( TARGET_RADIO, info.TxID );
					return true;
				}

				SoraUCompleteTxPacket ( info.PktID, STATUS_SUCCESS );
				SoraURadioTxFree ( TARGET_RADIO, info.TxID );
				
				printf ( "packet send...\n" );
			}

		}		
			
		return TRUE;
	}
	
	static BOOLEAN ThreadProc ( VOID* pParam ) {
		COFDMTxChain* pThis = (COFDMTxChain*) pParam;
		return pThis->Work ();
	};

	void Stop () {
		m_bStop = true;
		while ( Running() ) {
			Sleep (100); // wait			
		}
		m_bStop = false;
		
		if ( m_source ) {
			IReferenceCounting::Release (m_source);
			m_source = NULL;
		} 

		// free the tx queue
		for (TXQUEUE::iterator it=m_txqueue.begin(); it!=m_txqueue.end(); it++)
		{
			SoraUCompleteTxPacket ( it->PktID, STATUS_SUCCESS );
			SoraURadioTxFree  ( TARGET_RADIO, it->TxID );
		}
		m_txqueue.clear ();
		
		if ( m_thread ) {
			SoraThreadStop (m_thread);
			SoraThreadFree (m_thread);
			m_thread = NULL;
		}
	}
	
	bool Start () {
		if ( m_source == NULL) {
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
