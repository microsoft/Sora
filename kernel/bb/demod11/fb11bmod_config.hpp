#pragma once

#include "ieee80211facade.hpp"
#include "cck.hpp"

//
// configuration file for the mod pipeline
//

struct _tagBB11bModContext
    : LOCAL_CONTEXT(TModSink)	  	
	, LOCAL_CONTEXT(TBB11bDBPSKSpread)
    , LOCAL_CONTEXT(TSc741)
	, LOCAL_CONTEXT(TBB11bSrc)
{
} BB11bModCtx;

/*************************************************************************
Processing graph

ssrc->sc741->mrsel-> bpskspread -> shaper -> pack -> bufsink
                 |-> qpskspread ---|
*************************************************************************/
static inline
ISource* CreateModGraph () {
	CREATE_BRICK_SINK   ( modsink, 	TModSink, BB11bModCtx );
	CREATE_BRICK_FILTER ( pack16to8, TPackSample16to8,BB11bModCtx, modsink );	
	CREATE_BRICK_FILTER ( shaper, 	TQuickPulseShaper, BB11bModCtx, pack16to8);

	CREATE_BRICK_FILTER ( bpskSpread, TBB11bDBPSKSpread, BB11bModCtx,shaper);
	CREATE_BRICK_FILTER ( qpskSpread, TBB11bDQPSKSpread, BB11bModCtx,shaper);
	CREATE_BRICK_FILTER ( cck5p5, TCCK5Encode, BB11bModCtx,shaper);
	CREATE_BRICK_FILTER ( cck11, TCCK11Encode, BB11bModCtx,shaper);

	CREATE_BRICK_DEMUX4 ( mrsel, TBB11bMRSelect, BB11bModCtx,
		bpskSpread, qpskSpread, cck5p5, cck11 );
	CREATE_BRICK_FILTER ( sc741, TSc741, BB11bModCtx, mrsel );

	CREATE_BRICK_SOURCE ( ssrc, TBB11bSrc, BB11bModCtx, sc741 );

	return ssrc;
}

void InitModGraphCtx ( uint data_rate_kbps, 
					   UCHAR* DataBuffer, int bCnt, 
					   COMPLEX8* SymbolBuffer, int SymbolBufferSize )
{
	// TxVector
	BB11bModCtx.CF_11bTxVector::frame_length() = (ushort) bCnt;
	BB11bModCtx.CF_11bTxVector::preamble_type() = 0;
	BB11bModCtx.CF_11bTxVector::mod_select () = 0;
	BB11bModCtx.CF_11bTxVector::data_rate_kbps () = data_rate_kbps;
	BB11bModCtx.CF_11bTxVector::crc32 () = CalcCRC32(DataBuffer, bCnt); // no CRC32?
	// TxFrameBuffer
	BB11bModCtx.CF_TxFrameBuffer::mpdu_buf0 () = DataBuffer; 
	BB11bModCtx.CF_TxFrameBuffer::mpdu_buf1 () = NULL; 
	BB11bModCtx.CF_TxFrameBuffer::mpdu_buf_size0 () = (ushort)bCnt; 
	BB11bModCtx.CF_TxFrameBuffer::mpdu_buf_size1 () = 0; 

	// CF_Error
	BB11bModCtx.CF_Error::error_code () = E_ERROR_SUCCESS; 

	// CF_Scrambler
    BB11bModCtx.CF_ScramblerSeed::sc_seed() = DOT11B_PLCP_LONG_TX_SCRAMBLER_REGISTER;


	// CF_TxSampleBuf
	BB11bModCtx.CF_TxSampleBuffer::tx_sample_buf ()      = SymbolBuffer; 
	BB11bModCtx.CF_TxSampleBuffer::tx_sample_buf_size () = SymbolBufferSize; 
}
