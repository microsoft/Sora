#pragma once

#include "ieee80211facade.hpp"
#include "conv_enc.hpp"
#include "interleave.hpp"
#include "mapper11n.hpp"
#include "pilot.hpp"
#include "pilot_11n.hpp"
#include "fft.hpp"
#include "preamble11n.hpp"
#include "PHY_11n.hpp"
#include "sampling.hpp"
#include "streamparser.hpp"
#include "gi.hpp"
#include "csd.hpp"
#include "mapper11a.hpp"
#include "stdbrick.hpp"

//
// configuration file for the mod pipeline
//
SELECTANY 
struct _tagBB11nModContext :
      LOCAL_CONTEXT(TDropAny)
    , LOCAL_CONTEXT(TModSink1)
    , LOCAL_CONTEXT(T11aSc)
    , LOCAL_CONTEXT(TBB11nSrc)
{
    void init (	UCHAR * hdr, int hdrsize, UCHAR* data, int datasize, ushort mcs_index, COMPLEX16* sbuf[], uint sbuf_size )
    {
        tx_sample_buf[0] = sbuf[0];
        tx_sample_buf[1] = sbuf[1];
        tx_sample_cnt[0] = 0;
        tx_sample_cnt[1] = 0;
        tx_sample_buf_size = sbuf_size;

        // TxVector
        int frame_len = hdrsize + datasize;
        CF_11nTxVector::frame_length() = (ushort)frame_len;
        CF_11nTxVector::mcs_index() = mcs_index;
        CF_11nTxVector::crc32 () = CalcCRC32_Concat2(hdr, hdrsize, data, datasize);

        // TxFrameBuffer
        CF_TxFrameBuffer::mpdu_buf0 () = hdr; 
        CF_TxFrameBuffer::mpdu_buf1 () = data;
        CF_TxFrameBuffer::mpdu_buf_size0 () = (ushort)hdrsize; 
        CF_TxFrameBuffer::mpdu_buf_size1 () = (ushort)datasize;

        // CF_Error
        CF_Error::error_code () = E_ERROR_SUCCESS; 

        // CF_Scrambler
        CF_ScramblerSeed::sc_seed() = 0xAB;
    }

    void BindSinkBuffer(ModSinkDesc *sink0, ModSinkDesc *sink1)
    {
        sink0->Init(tx_sample_buf[0], tx_sample_buf_size, tx_sample_cnt[0]);
        sink1->Init(tx_sample_buf[1], tx_sample_buf_size, tx_sample_cnt[1]);
    }

    uint GetSinkSampleCount()
    {
        assert(tx_sample_cnt[0] == tx_sample_cnt[1]);
        return tx_sample_cnt[0];
    }

private:
    COMPLEX16* tx_sample_buf[2];
    uint       tx_sample_buf_size;
    uint       tx_sample_cnt[2];
} BB11nModCtx;

static FINL ISource* CreateSigGraph11n () {
    CREATE_BRICK_SINK   ( modsink_s0, 	    TModSink1, BB11nModCtx);
    CREATE_BRICK_FILTER ( addgi_s0,         TAddGI,  BB11nModCtx, modsink_s0 );

    CREATE_BRICK_SINK   ( modsink_s1, 	    TModSink1, BB11nModCtx);
    CREATE_BRICK_FILTER ( addgi_s1,         TAddGI,  BB11nModCtx, modsink_s1 );
    CREATE_BRICK_FILTER ( csd,              TCSD<2>::Filter,  BB11nModCtx, addgi_s1 );

    CREATE_BRICK_DEMUX2 ( tee,	            TTeeEx, BB11nModCtx, addgi_s0, csd );
    CREATE_BRICK_FILTER ( ifft,	            TIFFTxOnly, BB11nModCtx, tee );
    CREATE_BRICK_FILTER ( addpilot,         T11aAddPilot<30339>::Filter, BB11nModCtx, ifft );
    CREATE_BRICK_FILTER ( map_bpsk,	        TSigMap11n,    BB11nModCtx, addpilot );
    CREATE_BRICK_FILTER ( inter_bpsk,	    T11aInterleaveBPSK::Filter,    BB11nModCtx, map_bpsk );
    CREATE_BRICK_FILTER ( enc,	            TConvEncode_12,    BB11nModCtx, inter_bpsk );

    CREATE_BRICK_SOURCE ( ssrc,             TBB11nSigSrc, BB11nModCtx, enc );

    BB11nModCtx.BindSinkBuffer(modsink_s0, modsink_s1);
    return ssrc;
}

/*************************************************************************
Processing graphs

Preamble:
ssrc --> pack --> modsink

Data:
ssrc->scramble->MRSel->enc 1/2 ->interleave 1/2 -> mapper bpsk -> addpilot->ifft->pack->modsink
| ->enc 1/2 ->interleave 1/2 -> mapper qpsk -|               
*************************************************************************/

static FINL ISource* CreateModGraph11n () {
    CREATE_BRICK_SINK	( drop,	TDropAny,  BB11nModCtx );

    CREATE_BRICK_SINK   ( modsink_s0, 	    TModSink1, BB11nModCtx );	
    CREATE_BRICK_FILTER ( addgi_s0,         TAddGI,  BB11nModCtx, modsink_s0 );
    CREATE_BRICK_FILTER ( ifft_s0,	        TIFFTxOnly, BB11nModCtx, addgi_s0 );
    CREATE_BRICK_FILTER ( addpilot1_s0,     T11nAddPilot<0>::Filter, BB11nModCtx, ifft_s0 );
    CREATE_BRICK_FILTER ( addpilot_s0,      TNoInline, BB11nModCtx, addpilot1_s0);

    CREATE_BRICK_SINK   ( modsink_s1, 	    TModSink1, BB11nModCtx );
    CREATE_BRICK_FILTER ( addgi_s1,         TAddGI,  BB11nModCtx, modsink_s1 );
    CREATE_BRICK_FILTER ( csd,              TCSD<4>::Filter,  BB11nModCtx, addgi_s1 );
    CREATE_BRICK_FILTER ( ifft_s1,	        TIFFTxOnly, BB11nModCtx, csd );
    CREATE_BRICK_FILTER ( addpilot1_s1,     T11nAddPilot<1>::Filter, BB11nModCtx, ifft_s1 );
    CREATE_BRICK_FILTER ( addpilot_s1,      TNoInline, BB11nModCtx, addpilot1_s1);

    CREATE_BRICK_FILTER ( map_bpsk_s0,		TMap11aBPSK<30339>::Filter,    BB11nModCtx, addpilot_s0 );
    CREATE_BRICK_FILTER ( map_bpsk_s1,		TMap11aBPSK<30339>::Filter,    BB11nModCtx, addpilot_s1 );
    CREATE_BRICK_FILTER ( map_qpsk_s0,		TMap11aQPSK<21453>::Filter,    BB11nModCtx, addpilot_s0 );
    CREATE_BRICK_FILTER ( map_qpsk_s1,		TMap11aQPSK<21453>::Filter,    BB11nModCtx, addpilot_s1 );

    CREATE_BRICK_FILTER ( inter_bpsk_s0,	T11nInterleaveBPSK_S1::Filter,     BB11nModCtx, map_bpsk_s0 );
    CREATE_BRICK_FILTER ( inter_bpsk_s1,	T11nInterleaveBPSK_S2::Filter,     BB11nModCtx, map_bpsk_s1 );
    CREATE_BRICK_FILTER ( inter_qpsk_s0,	T11nInterleaveQPSK_S1::Filter,     BB11nModCtx, map_qpsk_s0 );
    CREATE_BRICK_FILTER ( inter_qpsk_s1,	T11nInterleaveQPSK_S2::Filter,     BB11nModCtx, map_qpsk_s1 );

    CREATE_BRICK_DEMUX2 ( sp_bpsk,	        TStreamParserBPSK_12,     BB11nModCtx, inter_bpsk_s0, inter_bpsk_s1 );
    CREATE_BRICK_DEMUX2 ( sp_qpsk,	        TStreamParserQPSK_12,     BB11nModCtx, inter_qpsk_s0, inter_qpsk_s1 );

    CREATE_BRICK_FILTER ( enc8,	            TConvEncode_12,     BB11nModCtx, sp_bpsk );
    CREATE_BRICK_FILTER ( enc9,	            TConvEncode_12,     BB11nModCtx, sp_qpsk );
    CREATE_BRICK_FILTER ( enc10,	        TConvEncode_34,     BB11nModCtx, sp_qpsk );

    CREATE_BRICK_DEMUX8 ( mrsel,	        TBB11nMRSelect,     BB11nModCtx,
        enc8, enc9, enc10, drop, drop, drop, drop, drop );
    CREATE_BRICK_FILTER ( sc,	            T11aSc,             BB11nModCtx, mrsel );
    CREATE_BRICK_SOURCE ( ssrc,             TBB11nSrc,          BB11nModCtx, sc ); 

    BB11nModCtx.BindSinkBuffer(modsink_s0, modsink_s1);
    return ssrc;
}

static FINL void CreatePreambleGraph11n (OUT ISource*& lsrc, OUT ISource*& htsrc) {
    CREATE_BRICK_SINK   ( modsink_s0, 	    TModSink1,          BB11nModCtx );
    CREATE_BRICK_SINK   ( modsink_s1, 	    TModSink1,          BB11nModCtx );
    CREATE_BRICK_DEMUX2 ( l_src0,           LSrc,               BB11nModCtx, modsink_s0, modsink_s1 ); 
    CREATE_BRICK_DEMUX2 ( ht_src0,          HTSrc,              BB11nModCtx, modsink_s0, modsink_s1 ); 
    CREATE_BRICK_SOURCE ( l_src,	        TDummySource,       BB11nModCtx, l_src0 );
    CREATE_BRICK_SOURCE ( ht_src,	        TDummySource,       BB11nModCtx, ht_src0 );

    BB11nModCtx.BindSinkBuffer(modsink_s0, modsink_s1);
    lsrc = l_src;
    htsrc = ht_src;
}

SELECTANY ISource * pBB11nTxSource, * pBB11nLPreambleSource, *pBB11nHtPreambleSource, * pBB11nSigSource;
