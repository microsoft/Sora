#pragma once

#include <windows.h>

#include "stdbrick.hpp"
#include "ieee80211facade.hpp"

#include "barkerspread.hpp"
#include "scramble.hpp"
#include "samples.hpp"
#include "phy_11b.hpp"
#include "pulse.hpp"
#include "cck.hpp"

//
// configuration file for the mod pipeline
//
SELECTANY
struct _tagBB11bModContext :
	  LOCAL_CONTEXT(TDropAny)
	, LOCAL_CONTEXT(TModSink)	  	
	, LOCAL_CONTEXT(TBB11bDBPSKSpread)
    , LOCAL_CONTEXT(TSc741)
	, LOCAL_CONTEXT(TBB11bSrc)
{
	bool init (	UCHAR * hdr, int hdrsize, UCHAR* data, int datasize, 
				ushort rate_kbps,
				COMPLEX8* txbuf, uint txbufsize )
	{
		// TxVector
		int frame_size = hdrsize + datasize;
		CF_11bTxVector::frame_length() = (ushort) frame_size;
		CF_11bTxVector::preamble_type() = 0;
		CF_11bTxVector::mod_select () = 0;
		CF_11bTxVector::data_rate_kbps () = rate_kbps; 
		CF_11bTxVector::crc32 () = CalcCRC32_Concat2(hdr, hdrsize, data, datasize); 

		// TxFrameBuffer
		CF_TxFrameBuffer::mpdu_buf0 () = hdr; 
		CF_TxFrameBuffer::mpdu_buf1 () = data; 
		CF_TxFrameBuffer::mpdu_buf_size0 () = (ushort) hdrsize; 
		CF_TxFrameBuffer::mpdu_buf_size1 () = (ushort) datasize; 

		// CF_Error
		CF_Error::error_code () = E_ERROR_SUCCESS; 

		// CF_Scrambler
	    CF_ScramblerSeed::sc_seed() = DOT11B_PLCP_LONG_TX_SCRAMBLER_REGISTER;


		// CF_TxSampleBuf
		CF_TxSampleBuffer::tx_sample_buf ()      = txbuf; 
		CF_TxSampleBuffer::tx_sample_buf_size () = txbufsize; 
		
		return true;
	}	
} BB11bModCtx;


/*************************************************************************
Processing graph

ssrc->sc741->mrsel-> bpskspread -> shaper -> pack -> bufsink
                 |-> qpskspread ---|
*************************************************************************/
static inline
ISource* CreateModGraph11b () {
	CREATE_BRICK_SINK	( drop,	TDropAny,  BB11bModCtx );
	
	CREATE_BRICK_SINK   ( modsink, 	TModSink, BB11bModCtx );
	CREATE_BRICK_FILTER ( pack16to8, TPackSample16to8,BB11bModCtx, modsink );	
	CREATE_BRICK_FILTER ( shaper, 	TQuickPulseShaper, BB11bModCtx, pack16to8);

	CREATE_BRICK_FILTER ( bpskSpread, TBB11bDBPSKSpread, BB11bModCtx,shaper);
	CREATE_BRICK_FILTER ( qpskSpread, TBB11bDQPSKSpread, BB11bModCtx,shaper);

	CREATE_BRICK_FILTER ( cck5p5, TCCK5Encode, BB11bModCtx,shaper);
	CREATE_BRICK_FILTER ( cck11, TCCK11Encode, BB11bModCtx,shaper);

	CREATE_BRICK_DEMUX4 ( mrsel, TBB11bMRSelect, BB11bModCtx,
		bpskSpread, qpskSpread, cck5p5, cck11 );
	CREATE_BRICK_FILTER ( sc741,	TSc741, BB11bModCtx, mrsel );


	CREATE_BRICK_SOURCE ( ssrc, TBB11bSrc, BB11bModCtx, sc741 ); 

	return ssrc;
}	

SELECTANY ISource * pBB11bTxSource;	  


