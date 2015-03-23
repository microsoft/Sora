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

#include "depuncturer.hpp"
#include "PHY_11a.hpp"
#include "PHY_11b.hpp"
#include "viterbi.hpp"
#include "pilot.hpp"
#include "channel_11a.hpp"
#include "cca.hpp"
#include "freqoffset.hpp"
#include "deinterleaver.hpp"
#include "samples.hpp"

#define TARGET_RADIO 0

ISource* CreateDemodGraph ( ISource*& decoder );

struct PKT_INFO {
	PACKET_HANDLE PktID;
	ULONG         TxID;
};

class COFDMRxChain :
	public LOCAL_CONTEXT(TDropAny),
	public LOCAL_CONTEXT(TBB11aFrameSink),
	public LOCAL_CONTEXT(T11aViterbi),
	public LOCAL_CONTEXT(T11aPLCPParser),
	public LOCAL_CONTEXT(TPilotTrack),
	public LOCAL_CONTEXT(TChannelEqualization),
	public LOCAL_CONTEXT(T11aLTSymbol),		
	public LOCAL_CONTEXT(TCCA11a),  
	public LOCAL_CONTEXT(TPhaseCompensate),
	public LOCAL_CONTEXT(TFreqCompensation),
	public LOCAL_CONTEXT(TDCRemoveEx),  
//	public LOCAL_CONTEXT(TMemSamples)
	public LOCAL_CONTEXT(TBB11aRxRateSel),
    public LOCAL_CONTEXT(TRxStream)
{
private:
	ISource* m_source;
	ISource* m_decoder;
	
	int      m_bRunning;
	bool     m_bStop;	
	HANDLE   m_thread;
	HANDLE   m_vthread; // thread for viterbi decoder

public:
	int& Running () { return m_bRunning; };

public:
	COFDMRxChain ( ) { 
		m_source   = NULL; 
		m_decoder  = NULL;
		m_bRunning = 0;
		m_bStop    = false;
		m_thread   = NULL;
		m_vthread  = NULL;
	}

	~COFDMRxChain () { 
	}

	bool Create ( PSORA_RADIO_RX_STREAM pRxStream, uchar * pFrameBuffer, int FrameBufSize ) {
		m_source   = CreateDemodGraph ( m_decoder );

		// CF_RxStream
		CF_RxStream::rxstream_pointer() = pRxStream;
		CF_RxStream::rxstream_touched() = 0;

		// CF_VecDC
		CF_VecDC::Reset ();

		// CF_RxFrameBuffer
		CF_RxFrameBuffer::Init ( pFrameBuffer, FrameBufSize );
		
		return true;
	}

	// Reset all CFacade data in the context
	void ResetContext () {
		// CF_Error
		CF_Error::error_code() = E_ERROR_SUCCESS;

		// CF_11CCA
		CF_11CCA::Reset();
		CF_11CCA::cca_pwr_threshold() = 2000*2000; // set energy threshold

		// CF_Channel_11a
		CF_Channel_11a::Reset ();

		// CF_FreqOffsetComp
		CF_FreqCompensate::Reset ();

		// CF_PhaseCompensation
		CF_PhaseCompensate::Reset ();

		// CF_PilotTrack
		CF_PilotTrack::Reset ();

		// CF_11aSymSel
		CF_11aSymState::Reset(); 

		// CF_CFOffset
		CF_CFOffset::Reset ();

		// CF_11RxPLCPSwitch
		CF_11RxPLCPSwitch::Reset();

        // CF_11aRxVector
        CF_11aRxVector::Reset();
	}

	bool DecodeWork () {
		if ( m_bStop ) {
			m_bRunning--;
			return false;
		}

		m_decoder->Process ();
		return true;
	}

	bool Work () {
		// Check if there is any packet available
		if ( m_bStop ) {
			m_bRunning--;
			return false;
		}
		
		HRESULT hr;

		ResetContext ();
		m_source->Reset ();

		// try to receive something here
		while (1) {
			m_source->Process ();
			ulong& err = CF_Error::error_code();
			if (  err != E_ERROR_SUCCESS ) {
				if (err == E_ERROR_FRAME_OK ) {
					printf ( "A frame is decoded!\n" );
					printf ( "Data rate %dMbps Length %d\n", 
						CF_11aRxVector::data_rate_kbps(),
						CF_11aRxVector::frame_length() );
					// for 16 bytes
					for (int i=0; i<32; i++) {
						printf ( "%02X ", CF_RxFrameBuffer::rx_frame_buf()[i] );
						if ( i>0 && i % 8 == 7	 ) printf ( "\n" );
					}
					printf ( "\n" );
				} else if (err == E_ERROR_CS_TIMEOUT ) {
					// printf ( "A frame is decoded!\n" );
					
				} else if ( err == E_ERROR_CRC32_FAIL ) {
					printf ( "A frame is detected, but CRC32 error!\n" );
				} else {
					printf ( "Error occurs: code %x\n", err );
				}

				break;
			}

			if ( m_bStop ) break;
		}
					
		return TRUE;
	}
	
	static BOOLEAN ThreadProc ( VOID* pParam ) {
		COFDMRxChain* pThis = (COFDMRxChain*) pParam;
		return pThis->Work ();
	};

	static BOOLEAN VThreadProc ( VOID* pParam ) {
		COFDMRxChain* pThis = (COFDMRxChain*) pParam;
		return pThis->DecodeWork ();
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

		if ( m_vthread ) {
			SoraThreadStop (m_vthread);
			SoraThreadFree (m_vthread);
			m_vthread = NULL;
		}
		
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

		// start decoder thread
		m_vthread = SoraUThreadAlloc();
		if ( !m_vthread) return false;
		if (!SoraUThreadStart(m_vthread, VThreadProc, this)) {
			SoraUThreadStop(m_vthread);
			SoraUThreadFree(m_vthread);
		}
		m_bRunning ++;

		// start demod thread
		m_thread = SoraUThreadAlloc();
		if (!m_thread) {
			SoraUThreadStop(m_vthread);
			SoraUThreadFree(m_vthread);
			
			return false;
		}

		if (!SoraUThreadStart(m_thread, ThreadProc, this)) {
			SoraUThreadStop(m_vthread);
			SoraUThreadFree(m_vthread);

			SoraUThreadStop(m_thread);
			SoraUThreadFree(m_thread);
			return false;
		}
		
		m_bRunning ++;

		return true;
	}

};
