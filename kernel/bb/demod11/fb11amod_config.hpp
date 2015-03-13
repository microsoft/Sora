#pragma once

#include "ieee80211facade.hpp"
#include "conv_enc.hpp"
#include "interleave.hpp"
#include "mapper11a.hpp"
#include "pilot.hpp"
#include "fft.hpp"
#include "preamble11a.hpp"
#include "PHY_11a.hpp"
#include "PHY_11b.hpp"
#include "sampling.hpp"

//
// configuration file for the mod pipeline
//

struct _tagBB11aModContext :
      LOCAL_CONTEXT(TDropAny)
    , LOCAL_CONTEXT(TModSink)
    , LOCAL_CONTEXT(T11aSc)
    , LOCAL_CONTEXT(TBB11aSrc)
{

    void set_mod_buffer ( COMPLEX8* pSymbolBuf, ulong symbol_buf_size )
    {
        // CF_Error
        CF_Error::error_code () = E_ERROR_SUCCESS; 
        
        // CF_TxSampleBuf
        CF_TxSampleBuffer::tx_sample_buf () 	 = pSymbolBuf; 
        CF_TxSampleBuffer::tx_sample_buf_size () = symbol_buf_size; 
    }
    
    void init ( uint data_rate_kbps, uchar * pBuffer, ushort frame_len, 
                COMPLEX8* pSymbolBuf, ulong symbol_buf_size )
    {
        // TxVector
        CF_11aTxVector::frame_length() = frame_len;
        CF_11aTxVector::data_rate_kbps () = data_rate_kbps; 	   // 1Mbps
        CF_11aTxVector::crc32 () = CalcCRC32(pBuffer, frame_len ); // no CRC32?
    
        // TxFrameBuffer
        CF_TxFrameBuffer::mpdu_buf0 () = pBuffer; 
        CF_TxFrameBuffer::mpdu_buf1 () = NULL;
        CF_TxFrameBuffer::mpdu_buf_size0 () = frame_len; 
        CF_TxFrameBuffer::mpdu_buf_size1 () = 0; 
    
        // CF_Error
        CF_Error::error_code () = E_ERROR_SUCCESS; 
    
        // CF_Scrambler
        CF_ScramblerSeed::sc_seed() = 0xFF;
    
    
        // CF_TxSampleBuf
        CF_TxSampleBuffer::tx_sample_buf () 	 = pSymbolBuf; 
        CF_TxSampleBuffer::tx_sample_buf_size () = symbol_buf_size; 
    }
} BB11aModCtx;

/*************************************************************************
Processing graphs

Preamble:
    ssrc --> pack --> modsink

Data:
    ssrc->scramble->MRSel->enc 1/2 ->interleave 1/2 -> mapper bpsk -> addpilot->ifft->pack->modsink
                       | ->enc 1/2 ->interleave 1/2 -> mapper qpsk -|               
*************************************************************************/

static FINL
ISource* CreateModGraph11a_40M () {
    CREATE_BRICK_SINK	( drop,	TDropAny,  BB11aModCtx );

    CREATE_BRICK_SINK   ( modsink, 	TModSink, BB11aModCtx );	
    CREATE_BRICK_FILTER ( pack, TPackSample16to8, BB11aModCtx, modsink );
    CREATE_BRICK_FILTER ( ifft,	TIFFTx, BB11aModCtx, pack );
    CREATE_BRICK_FILTER ( addpilot1,T11aAddPilot<>::Filter, BB11aModCtx, ifft );
    CREATE_BRICK_FILTER ( addpilot, TNoInline, BB11aModCtx, addpilot1);
    
    CREATE_BRICK_FILTER ( map_bpsk,		TMap11aBPSK<>::Filter,    BB11aModCtx, addpilot );
    CREATE_BRICK_FILTER ( map_qpsk,		TMap11aQPSK<>::Filter,    BB11aModCtx, addpilot );
    CREATE_BRICK_FILTER ( map_qam16,	TMap11aQAM16<>::Filter,   BB11aModCtx, addpilot );
    CREATE_BRICK_FILTER ( map_qam64,	TMap11aQAM64<>::Filter,   BB11aModCtx, addpilot );
    
    CREATE_BRICK_FILTER ( inter_bpsk,	T11aInterleaveBPSK::Filter,    BB11aModCtx, map_bpsk );
    CREATE_BRICK_FILTER ( inter_qpsk,	T11aInterleaveQPSK::Filter,    BB11aModCtx, map_qpsk );
    CREATE_BRICK_FILTER ( inter_qam16,  T11aInterleaveQAM16::Filter,    BB11aModCtx, map_qam16 );
    CREATE_BRICK_FILTER ( inter_qam64,	T11aInterleaveQAM64::Filter,    BB11aModCtx, map_qam64 );

    CREATE_BRICK_FILTER ( enc6,	    TConvEncode_12,    BB11aModCtx, inter_bpsk );
    CREATE_BRICK_FILTER ( enc9,	    TConvEncode_34,    BB11aModCtx, inter_bpsk );
    CREATE_BRICK_FILTER ( enc12,	TConvEncode_12,    BB11aModCtx, inter_qpsk );
    CREATE_BRICK_FILTER ( enc18,	TConvEncode_34,    BB11aModCtx, inter_qpsk );
    CREATE_BRICK_FILTER ( enc24,	TConvEncode_12,    BB11aModCtx, inter_qam16 );
    CREATE_BRICK_FILTER ( enc36,    TConvEncode_34,    BB11aModCtx, inter_qam16 );
    CREATE_BRICK_FILTER ( enc48,	TConvEncode_23,    BB11aModCtx, inter_qam64 );
    CREATE_BRICK_FILTER ( enc54,	TConvEncode_34,    BB11aModCtx, inter_qam64 );
    
    CREATE_BRICK_DEMUX8 ( mrsel,	TBB11aMRSelect,    BB11aModCtx,
                        enc6, enc9, enc12, enc18, enc24, enc36, enc48, enc54 );
    CREATE_BRICK_FILTER ( sc,	T11aSc,    BB11aModCtx, mrsel );

    CREATE_BRICK_SOURCE ( ssrc, TBB11aSrc, BB11aModCtx, sc ); 

    return ssrc;
}	


static FINL
ISource* CreateModGraph11a_44M () {
    CREATE_BRICK_SINK   ( modsink, 	TModSink, BB11aModCtx );	
    CREATE_BRICK_FILTER ( pack, TPackSample16to8, BB11aModCtx, modsink );


    CREATE_BRICK_FILTER ( upsample,     TUpsample40MTo44M, BB11aModCtx, pack );
    CREATE_BRICK_FILTER ( ifft,	        TIFFTx, BB11aModCtx, upsample );
    CREATE_BRICK_FILTER ( addpilot1,    T11aAddPilot<>::Filter, BB11aModCtx, ifft );
    CREATE_BRICK_FILTER ( addpilot,     TNoInline, BB11aModCtx, addpilot1);
    
    CREATE_BRICK_FILTER ( map_bpsk,		TMap11aBPSK<>::Filter,    BB11aModCtx, addpilot );
    CREATE_BRICK_FILTER ( map_qpsk,		TMap11aQPSK<>::Filter,    BB11aModCtx, addpilot );
    CREATE_BRICK_FILTER ( map_qam16,	TMap11aQAM16<>::Filter,   BB11aModCtx, addpilot );
    CREATE_BRICK_FILTER ( map_qam64,	TMap11aQAM64<>::Filter,   BB11aModCtx, addpilot );
    
    CREATE_BRICK_FILTER ( inter_bpsk,	T11aInterleaveBPSK::Filter,    BB11aModCtx, map_bpsk );
    CREATE_BRICK_FILTER ( inter_qpsk,	T11aInterleaveQPSK::Filter,    BB11aModCtx, map_qpsk );
    CREATE_BRICK_FILTER ( inter_qam16,  T11aInterleaveQAM16::Filter,    BB11aModCtx, map_qam16 );
    CREATE_BRICK_FILTER ( inter_qam64,	T11aInterleaveQAM64::Filter,    BB11aModCtx, map_qam64 );

    CREATE_BRICK_FILTER ( enc6,	    TConvEncode_12,    BB11aModCtx, inter_bpsk );
    CREATE_BRICK_FILTER ( enc9,	    TConvEncode_34,    BB11aModCtx, inter_bpsk );
    CREATE_BRICK_FILTER ( enc12,	TConvEncode_12,    BB11aModCtx, inter_qpsk );
    CREATE_BRICK_FILTER ( enc18,	TConvEncode_34,    BB11aModCtx, inter_qpsk );
    CREATE_BRICK_FILTER ( enc24,	TConvEncode_12,    BB11aModCtx, inter_qam16 );
    CREATE_BRICK_FILTER ( enc36,    TConvEncode_34,    BB11aModCtx, inter_qam16 );
    CREATE_BRICK_FILTER ( enc48,	TConvEncode_23,    BB11aModCtx, inter_qam64 );
    CREATE_BRICK_FILTER ( enc54,	TConvEncode_34,    BB11aModCtx, inter_qam64 );
    
    CREATE_BRICK_DEMUX8 ( mrsel,	TBB11aMRSelect,    BB11aModCtx,
                        enc6, enc9, enc12, enc18, enc24, enc36, enc48, enc54 );
    CREATE_BRICK_FILTER ( sc,	T11aSc,    BB11aModCtx, mrsel );

    CREATE_BRICK_SOURCE ( ssrc, TBB11aSrc, BB11aModCtx, sc ); 

    return ssrc;
}

static FINL
ISource* CreatePreamble11a_40M () {
    CREATE_BRICK_SINK	( drop,	TDropAny,  BB11aModCtx );
    
    CREATE_BRICK_SINK   ( modsink, 	TModSink, BB11aModCtx );
    CREATE_BRICK_FILTER ( pack, TPackSample16to8, BB11aModCtx, modsink );
    CREATE_BRICK_SOURCE ( ssrc, TTS11aSrc, BB11aModCtx, pack ); 

    return ssrc;
}

static FINL
ISource* CreatePreamble11a_44M () {
    CREATE_BRICK_SINK   ( modsink, 	TModSink, BB11aModCtx );
    CREATE_BRICK_FILTER ( pack, TPackSample16to8, BB11aModCtx, modsink );
    CREATE_BRICK_FILTER ( upsample,     TUpsample40MTo44M, BB11aModCtx, pack );
    CREATE_BRICK_SOURCE ( ssrc, TTS11aSrc, BB11aModCtx, upsample ); 

    return ssrc;
}
