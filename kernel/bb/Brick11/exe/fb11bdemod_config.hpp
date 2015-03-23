#pragma once

#include "stdbrick.hpp"
#include "ieee80211facade.hpp"
#include "cck.hpp"

struct _tagBB11bDemodContext :
	  LOCAL_CONTEXT(TDropAny)
	, LOCAL_CONTEXT(TBB11bFrameSink)  		  	
	, LOCAL_CONTEXT(TBB11bPlcpParser)  	
	, LOCAL_CONTEXT(TBB11bPlcpSwitch)  	
	, LOCAL_CONTEXT(TDBPSKDemap)  		  		  	  		  		  	
	, LOCAL_CONTEXT(TSFDSync)  		  		  	  		  	
	, LOCAL_CONTEXT(TBB11bRxRateSel)  		  		  	  	
	, LOCAL_CONTEXT(TEnergyDetect)  		  		  	
	, LOCAL_CONTEXT(TDCEstimator)  		  		  	
	, LOCAL_CONTEXT(TBB11bRxSwitch)  		  	
	, LOCAL_CONTEXT(TDCRemove)  	
	, LOCAL_CONTEXT(TMemSamples)
{
} BB11bDemodCtx;

/*************************************************************************
Processing graph

src->dc_removedc-> rxswitch -> energy_det -> estimate_dc 
               |-> symtiming -> barker_sync -> barker_despread -> MrSel...

 ...MrSel -> SFD
		|--> bpsk_demapper -> dsc741 -> plcp_swt -> hdr 
		|__> qpsk_demapper _|                  |--> data_collector
*************************************************************************/

// Note: use "static inline" instead of "inline" to prevent silent function confliction during linking.
// Otherwise, it will introduce wrong linking result because these inline function share the same name,
// and indeed not inlined.
// ref: http://stackoverflow.com/questions/2217628/multiple-definition-of-inline-functions-when-linking-static-libs
static inline
ISource* CreateDemodGraph ()
{
    CREATE_BRICK_SINK  (drop, TDropAny,  BB11bDemodCtx );

	// plcp header
    CREATE_BRICK_SINK  (plcphdr, TBB11bPlcpParser,  BB11bDemodCtx );
	
	// data 
    CREATE_BRICK_SINK  (mpdu, TBB11bFrameSink,  BB11bDemodCtx );

	CREATE_BRICK_DEMUX2 ( plcpswt, TBB11bPlcpSwitch, BB11bDemodCtx,
		plcphdr, mpdu);

	CREATE_BRICK_FILTER(dsc741,TDesc741,    BB11bDemodCtx, plcpswt );

    // CCK11 branch
	CREATE_BRICK_FILTER(cck11, TCCK11Decoder, BB11bDemodCtx, dsc741 );	

    // CCK5 branch
	CREATE_BRICK_FILTER(cck5p5, TCCK5P5Decoder, BB11bDemodCtx, dsc741 );	

	// QPSK branch
	CREATE_BRICK_FILTER(dqpsk, TDQPSKDemap, BB11bDemodCtx, dsc741 );	
	CREATE_BRICK_FILTER(despread_dqpsk,TBB11bDespread, BB11bDemodCtx, dqpsk );
	
	// BPSK branch
	CREATE_BRICK_FILTER(dbpsk, TDBPSKDemap, BB11bDemodCtx, dsc741 );
	CREATE_BRICK_FILTER(despread_dbpsk,TBB11bDespread, BB11bDemodCtx, dbpsk );

	// sfd sync branch
    CREATE_BRICK_SINK  (sfdsync, TSFDSync,  BB11bDemodCtx );
	CREATE_BRICK_FILTER(despread_sfd,TBB11bDespread, BB11bDemodCtx, sfdsync );

	CREATE_BRICK_DEMUX5 (mrsel, TBB11bRxRateSel, BB11bDemodCtx,
		despread_sfd, despread_dbpsk, despread_dqpsk, cck5p5, cck11 );

	// Demod branch
	CREATE_BRICK_FILTER(bsync,   TBarkerSync, BB11bDemodCtx, mrsel);
	CREATE_BRICK_FILTER(symtime, TSymTiming, BB11bDemodCtx, bsync );
	

	// CS branch
	CREATE_BRICK_FILTER(dcest, TDCEstimator, BB11bDemodCtx, drop );
	CREATE_BRICK_FILTER(pwrdet, TEnergyDetect, BB11bDemodCtx, dcest );


	
	CREATE_BRICK_DEMUX2 ( rxswt, TBB11bRxSwitch, BB11bDemodCtx,
		pwrdet, symtime);
	
    CREATE_BRICK_FILTER(dc,   TDCRemove, BB11bDemodCtx, rxswt );
    CREATE_BRICK_SOURCE(fsrc, TMemSamples, BB11bDemodCtx, dc );

    return fsrc;
}

static inline
void InitFBDemodContext ( COMPLEX16* input, uint in_size, UCHAR* output, uint out_size )
{
	// CF_MemSamples
	BB11bDemodCtx.CF_MemSamples::mem_sample_buf()       = input;
	BB11bDemodCtx.CF_MemSamples::mem_sample_buf_size()  = in_size;
	BB11bDemodCtx.CF_MemSamples::mem_sample_count()     = in_size / sizeof(COMPLEX16);
	BB11bDemodCtx.CF_MemSamples::mem_sample_start_pos() = 0;

	// CF_Error
	BB11bDemodCtx.CF_Error::error_code() = E_ERROR_SUCCESS;

	// CF_VecDC
	vcs& vdc = BB11bDemodCtx.CF_VecDC::direct_current();
	set_zero(vdc);

	// CF_11CCA
	BB11bDemodCtx.CF_11CCA::cca_state() = CF_11CCA::power_clear;
	BB11bDemodCtx.CF_11CCA::cca_pwr_threshold() = 1000*1000;

	// CF_11bRxMRSel
	BB11bDemodCtx.CF_11bRxMRSel::rxrate_state() = CF_11bRxMRSel::rate_sync;	
	// CF_11RxPLCPSwitch
	BB11bDemodCtx.CF_11RxPLCPSwitch::plcp_state() = CF_11RxPLCPSwitch::plcp_header;

	// CF_RxFrameBuffer
	BB11bDemodCtx.CF_RxFrameBuffer::rx_frame_buf() = output;
	BB11bDemodCtx.CF_RxFrameBuffer::rx_frame_buf_size() = out_size;
	
}

