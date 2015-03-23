#pragma once

#include <sora.h>
#include <brick.h>
#include <stdbrick.hpp>
#include <CRC32.h>
#include "sora_graph.h"

// Brick11 
#include <ieee80211facade.hpp>
#include <conv_enc.hpp>
#include <interleave.hpp>
#include <mapper11a.hpp>
#include <pilot.hpp>
#include <fft.hpp>
#include <preamble11a.hpp>
#include <scramble.hpp>
#include <PHY_11a.hpp>
#include <sampling.hpp>

//
// CTxGraph11a 
//
class CTxGraph11a : public CSoraGraph 
    , public LOCAL_CONTEXT(TDropAny)
    , public LOCAL_CONTEXT(TModSink)
    , public LOCAL_CONTEXT(T11aSc)
    , public LOCAL_CONTEXT(TBB11aSrc)
{
public:
	ISource * m_pPreambleSrc;
	ISource * m_pDataSrc;

	CTxGraph11a () {
		m_pPreambleSrc = NULL;
		m_pDataSrc     = NULL;
	}

	~CTxGraph11a () {
		if ( m_pPreambleSrc) {
			IReferenceCounting::Release (m_pPreambleSrc); 
			m_pPreambleSrc = NULL;
		}

		if ( m_pDataSrc) {
			IReferenceCounting::Release (m_pDataSrc); 
			m_pDataSrc = NULL;
		}
	}

protected:
	// context operations
    void set_mod_buffer ( COMPLEX8* pSymbolBuf, ulong symbol_buf_size )
    {
        // CF_Error
        CF_Error::error_code () = E_ERROR_SUCCESS; 
        
        // CF_TxSampleBuf
        CF_TxSampleBuffer::tx_sample_buf () 	 = pSymbolBuf; 
        CF_TxSampleBuffer::tx_sample_buf_size () = symbol_buf_size; 
    }

	// interfaces
	bool Init (	UCHAR * hdr, int hdrsize, UCHAR* data, int datasize, 
				ushort rate_kbps,
				COMPLEX8* txbuf, uint txbufsize )
    {
        // TxVector
		int frame_size = hdrsize + datasize;
		CF_11aTxVector::frame_length() = (ushort) frame_size;
        CF_11aTxVector::data_rate_kbps () = rate_kbps;
		CF_11aTxVector::crc32 () = CalcCRC32_Concat2(hdr, hdrsize, data, datasize); 
    
        // TxFrameBuffer
		CF_TxFrameBuffer::mpdu_buf0 () = hdr; 
		CF_TxFrameBuffer::mpdu_buf1 () = data; 
		CF_TxFrameBuffer::mpdu_buf_size0 () = (ushort) hdrsize; 
		CF_TxFrameBuffer::mpdu_buf_size1 () = (ushort) datasize; 
    
        // CF_Error
        CF_Error::error_code () = E_ERROR_SUCCESS; 
    
        // CF_Scrambler
        CF_ScramblerSeed::sc_seed() = 0xFF;
    
        // CF_TxSampleBuf
		CF_TxSampleBuffer::tx_sample_buf ()      = txbuf; 
		CF_TxSampleBuffer::tx_sample_buf_size () = txbufsize;
        return true;
    }

// Interface
public:
	bool CreateGraph () {
		// try to create the graph(s)
		CreatePreamble  ();
		CreateDataGraph ();

		return true;
	}

	ULONG GetErrorCode () { return  CF_Error::error_code (); }

	//
	// modulate a packet, given in whdr and data, and store the waveform samples into a buffer
	//
	bool Modulate ( UCHAR * whdr, int hdrsize, UCHAR* data, int datasize, 
					 ushort rate_kbps,
				     COMPLEX8* sbuf, uint sbuf_size, ULONG* sample_size )
	{
		if ( m_pPreambleSrc == NULL ||
			m_pDataSrc      == NULL ) 
			return false;

		// Generate signals for fixed preamble
		COMPLEX8* pSymBuf = sbuf;

		set_mod_buffer (pSymBuf, sbuf_size );
		m_pPreambleSrc->Reset ();
		if ( CF_Error::error_code() == E_ERROR_SUCCESS ) {
			// Reset okay, processing graph
			m_pPreambleSrc->Process (); 
		}	

		// skip to the data part
		uint TS_len = CF_TxSampleBuffer::tx_sample_cnt();
		pSymBuf += TS_len;

		// Generate signals for PLSC, data and CRC
		Init ( whdr, hdrsize, data, datasize, rate_kbps, pSymBuf, sbuf_size-TS_len );
		m_pDataSrc->Reset ();
		if ( CF_Error::error_code() != BK_ERROR_SUCCESS ) {
			printf ( "Mod graph reset error!\n" );
			return false;
		}	

		// modulation
		while ( m_pDataSrc->Process () );
		if ( CF_Error::error_code() != BK_ERROR_SUCCESS ) 
			return false;

		// set output samples
		(*sample_size) = (CF_TxSampleBuffer::tx_sample_cnt() + TS_len) * sizeof(COMPLEX8);
		return true;
	}


public:
	// Create graphs
	ISource* CreateDataGraph () {
		CREATE_BRICK_SINK	( drop,	TDropAny,  (*this) );

		CREATE_BRICK_SINK   ( modsink, 	TModSink, (*this) );	
		CREATE_BRICK_FILTER ( pack, TPackSample16to8, (*this), modsink );
		CREATE_BRICK_FILTER ( ifft,	TIFFTx, (*this), pack );
		CREATE_BRICK_FILTER ( addpilot1,T11aAddPilot<>::Filter, (*this), ifft );
		CREATE_BRICK_FILTER ( addpilot, TNoInline, (*this), addpilot1);
    
		CREATE_BRICK_FILTER ( map_bpsk,		TMap11aBPSK<>::Filter,    (*this), addpilot );
		CREATE_BRICK_FILTER ( map_qpsk,		TMap11aQPSK<>::Filter,    (*this), addpilot );
		CREATE_BRICK_FILTER ( map_qam16,	TMap11aQAM16<>::Filter,   (*this), addpilot );
		CREATE_BRICK_FILTER ( map_qam64,	TMap11aQAM64<>::Filter,   (*this), addpilot );
    
		CREATE_BRICK_FILTER ( inter_bpsk,	T11aInterleaveBPSK::Filter,    (*this), map_bpsk );
		CREATE_BRICK_FILTER ( inter_qpsk,	T11aInterleaveQPSK::Filter,    (*this), map_qpsk );
		CREATE_BRICK_FILTER ( inter_qam16,  T11aInterleaveQAM16::Filter,    (*this), map_qam16 );
		CREATE_BRICK_FILTER ( inter_qam64,	T11aInterleaveQAM64::Filter,    (*this), map_qam64 );

		CREATE_BRICK_FILTER ( enc6,	    TConvEncode_12,    (*this), inter_bpsk );
		CREATE_BRICK_FILTER ( enc9,	    TConvEncode_34,    (*this), inter_bpsk );
		CREATE_BRICK_FILTER ( enc12,	TConvEncode_12,    (*this), inter_qpsk );
		CREATE_BRICK_FILTER ( enc18,	TConvEncode_34,    (*this), inter_qpsk );
		CREATE_BRICK_FILTER ( enc24,	TConvEncode_12,    (*this), inter_qam16 );
		CREATE_BRICK_FILTER ( enc36,    TConvEncode_34,    (*this), inter_qam16 );
		CREATE_BRICK_FILTER ( enc48,	TConvEncode_23,    (*this), inter_qam64 );
		CREATE_BRICK_FILTER ( enc54,	TConvEncode_34,    (*this), inter_qam64 );
    
		CREATE_BRICK_DEMUX8 ( mrsel,	TBB11aMRSelect,    (*this),
							enc6, enc9, enc12, enc18, enc24, enc36, enc48, enc54 );

		CREATE_BRICK_FILTER ( sc,	T11aSc,    (*this), mrsel );

		CREATE_BRICK_SOURCE ( ssrc, TBB11aSrc, (*this), sc ); 
		m_pDataSrc = ssrc;

		return ssrc;
	}

	ISource* CreatePreamble () {
		CREATE_BRICK_SINK   ( modsink, 	TModSink, (*this) );
		CREATE_BRICK_FILTER ( pack, TPackSample16to8, (*this), modsink );
		CREATE_BRICK_SOURCE ( ssrc, TTS11aSrc, (*this), pack ); 

		m_pPreambleSrc = ssrc;
		return ssrc;
	}
};