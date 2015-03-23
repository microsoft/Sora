#pragma once

#include <conio.h>
#include <time.h>
#include "stdbrick.hpp"

// Sora DSP Library
#include <sdl.hpp>

#include "ieee80211facade.hpp"
#include "depuncturer.hpp"
#include "PHY_11a.hpp"
#include "PHY_11b.hpp"
#include "viterbi.hpp"
#include "pilot.hpp"
#include "channel_11a.hpp"
#include "cca.hpp"
#include "freqoffset.hpp"
#include "fft.hpp"
#include "scramble.hpp"
#include "deinterleaver.hpp"
#include "samples.hpp"
#include "44MTo40M.hpp"
#include "thread_func.h"

// Debug
class CF_Constellation {
	FACADE_FIELD ( COMPLEX16, constellation, [64] ); 
};

DEFINE_LOCAL_CONTEXT(TConstellation, CF_Constellation);
template< TSINK_ARGS >
class TConstellation : public TSink<TSINK_PARAMS>
{
	CTX_VAR_RW ( COMPLEX16, constellation, [64] );

public:

    DEFINE_IPORT(COMPLEX16, 64);

	REFERENCE_LOCAL_CONTEXT(TConstellation);

    STD_TSINK_CONSTRUCTOR(TConstellation) 
		BIND_CONTEXT(CF_Constellation::constellation, constellation )
	{}
	STD_TSINK_RESET() {}
	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(pin) {
		while (pin.check_read()) {
			vcs* pi = (vcs*) pin.peek();
			rep<16>::vmemcpy ( (vcs*)constellation, pi );

			pin.pop ();
		}
        return true;
    }
};

typedef struct _tagBB11aDemodContext :
	  LOCAL_CONTEXT(TDropAny)
  	, LOCAL_CONTEXT(TBB11aFrameSink)									  	  	
  	, LOCAL_CONTEXT(T11aViterbi)									  
	, LOCAL_CONTEXT(T11aPLCPParser)	  		  		  		  		  	
	, LOCAL_CONTEXT(T11aDemap)
	, LOCAL_CONTEXT(TPilotTrack)	  		  		  	
	, LOCAL_CONTEXT(TChannelEqualization)	  		  	
	, LOCAL_CONTEXT(T11aLTSymbol)	  	
	, LOCAL_CONTEXT(TChannelEst)	
	, LOCAL_CONTEXT(TCCA11a)  
	, LOCAL_CONTEXT(TPhaseCompensate)
	, LOCAL_CONTEXT(TFFT64)  		
	, LOCAL_CONTEXT(TFreqCompensation)  			
	, LOCAL_CONTEXT(TDCRemoveEx)  
    , LOCAL_CONTEXT(TRxStream)
    , LOCAL_CONTEXT(TBB11aRxRateSel)
	, LOCAL_CONTEXT(TConstellation)
{
    void Reset () {
		// Reset all CFacade data in the context
        ResetCarrierSense();

		// CF_11aSymSel
		CF_11aSymState::Reset(); 

		// CF_CFOffset
		CF_CFOffset::Reset ();
		
		// CF_Channel_11a
		CF_Channel_11a::Reset ();

		// CF_FreqOffsetComp
		CF_FreqCompensate::Reset ();

		// CF_PhaseCompensation
		CF_PhaseCompensate::Reset ();

		// CF_PilotTrack
		CF_PilotTrack::Reset ();

		// CF_11RxPLCPSwitch
		CF_11RxPLCPSwitch::Reset();

        // CF_11aRxVector
        CF_11aRxVector::Reset();
	}
	
    void ResetCarrierSense () {
		// CF_Error
		CF_Error::error_code() = E_ERROR_SUCCESS;

		// CF_11CCA
		CF_11CCA::Reset();
		CF_11CCA::cca_pwr_threshold() = gCCAThreshold; // set energy threshold

	}

	void Init (PSORA_RADIO_RX_STREAM pRxStream, UCHAR* output, uint out_size ) 
	{
		// CF_RxStream
		CF_RxStream::rxstream_pointer() = pRxStream;
		CF_RxStream::rxstream_touched() = 0;

		// CF_VecDC
		CF_VecDC::Reset ();

		// CF_RxFrameBuffer
		CF_RxFrameBuffer::Init (output, out_size);

		Reset ();
	}

	ulong GetDataRate () {
		return CF_11aRxVector::data_rate_kbps();
	}
	
	void OnAGC ( int Energy ) {
static int seq = 0;
static ULONGLONG last_timestamp = 0;

		int target_level;
		
		// okay agc goes here
		ULONG RxGG = RxG;

		ULONG dc_energy = SquaredNorm(CF_VecDC::direct_current()[0]);
		ULONG e_control = Energy + dc_energy;
		
		uchar scope   = _bit_scope_ui (e_control);

		PlotText ( "debug", "OnAGC (%d) : %d scope - dc %d energy %d", seq ++, scope, dc_energy, Energy );

		if ( dc_energy > (ULONG) Energy ) {
			target_level = 20;
		} else {
			target_level = 18;
		}

		// the target is to control the amplitude to 10 to 12bits
		if ( scope > target_level+3 ) {
			uint gain = 0x300 * (scope - target_level);
						
			if ( RxGG >= gain + 0x100 ) {
				RxGG -= gain;
			} else {
				RxGG = 0x100;
			}
		} else if ( scope < target_level-3 ){
			uint gain = 0x300 * (target_level-scope );
			
			RxGG += gain;
			if (RxGG > MAX_RX_GAIN ) RxGG = MAX_RX_GAIN;
		}

		ULONGLONG ts = SoraGetCPUTimestamp (&tsinfo);
		ULONGLONG diff = ts - last_timestamp; 
		ULONGLONG gap = tsinfo.CPUFreq.QuadPart / 10000;
		if ( fAutoCfg && (RxGG != RxG) && diff > gap) {
			RxG = RxGG;
			SoraURadioSetRxGain ( TARGET_RADIO , RxG );
			last_timestamp = ts;
			PlotText ( "debug", "OnAGC:==> energy %d scope %d new gain %x )", e_control, scope, RxG );
		} else if ( diff < gap ) {
			PlotText ( "debug", "AGC is too closed! %I64d %I64d", diff, gap );
		}
		
	}

}BB11aDemodContext;

// Contruct bricks
SELECTANY BB11aDemodContext BB11aDemodCtx;

struct sym_selector {
	FINL
	int operator()(BB11aDemodContext& ctx) {
		if ( ctx.CF_11aSymState::symbol_type()  == CF_11aSymState::SYMBOL_TRAINING ) { 
			return PORT_0; 
		} else {
		 	return PORT_1;
		}
	}
};
	  
typedef TGDemux<2, COMPLEX16, 4, sym_selector> T11aSymSelEx;


//
// Define the selector object for the plcp switch
//
struct plcp_selector {
	FINL
	int operator()(BB11aDemodContext& ctx) {
		if ( ctx.CF_11RxPLCPSwitch::plcp_state()  == CF_11RxPLCPSwitch::plcp_header ) { 
			return PORT_0;
		} else {
		 	return PORT_1;
        }
	}
};
	  
typedef TGDemux<2, COMPLEX16, 64, plcp_selector> T11aPlcpSwitch;


/*************************************************************************
Processing graph

src-> downsample -> rxswt -> dc_removedc -> BB11aCCA -> estimate_dc 
                     |-> symsel -> LTSSymbol -> ch_est -> fine_cfo -> drop
                               |-> dataSym -> FFT -> Equalization -> phase_comp ..-> 
  ..-> pilot -> plcpswt -> plcp_demap -> plcp_deinter -> plcp_viterbi -> plcp_parser
                      | -> 6M_demap -> 6M_deinter -> thread_sep -> viterbi -> desc -> frame_sink

*************************************************************************/

// Note: use "static inline" instead of "inline" to prevent silent function confliction during linking.
// Otherwise, it will introduce wrong linking result because these inline function share the same name,
// and indeed not inlined.
// ref: http://stackoverflow.com/questions/2217628/multiple-definition-of-inline-functions-when-linking-static-libs
//
static inline
void CreateDemodGraph11a_20M (ISource*& srcAll, ISource*& srcViterbi, ISource*& srcCarrierSense)
{
    CREATE_BRICK_SINK  (drop, TDropAny,    BB11aDemodCtx );

    CREATE_BRICK_SINK   (fsink, TBB11aFrameSink, BB11aDemodCtx );	
	CREATE_BRICK_FILTER (desc,   T11aDesc,	BB11aDemodCtx, fsink );				

	typedef T11aViterbi <5000*8, 48, 256> T11aViterbiComm;
	CREATE_BRICK_FILTER (viterbi, T11aViterbiComm::Filter,	BB11aDemodCtx, desc );				

    CREATE_BRICK_FILTER (vit0, TThreadSeparator<>::Filter, BB11aDemodCtx, viterbi);

	// 6M
	CREATE_BRICK_FILTER (di6, 	T11aDeinterleaveBPSK, BB11aDemodCtx, vit0 );		
    CREATE_BRICK_FILTER (dm6, 	T11aDemapBPSK::Filter,      BB11aDemodCtx, di6 );

	
	// 12M
	CREATE_BRICK_FILTER (di12, 	T11aDeinterleaveQPSK, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm12, 	T11aDemapQPSK::Filter,      BB11aDemodCtx, di12 );
	
	// 24M
	CREATE_BRICK_FILTER (di24, 	T11aDeinterleaveQAM16, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm24, 	T11aDemapQAM16::Filter,      BB11aDemodCtx, di24 );
	
	// 48M
	CREATE_BRICK_FILTER (di48, 	T11aDeinterleaveQAM64, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm48, 	T11aDemapQAM64::Filter,      BB11aDemodCtx, di48 );
	
	// PLCP 
    CREATE_BRICK_SINK   (plcp,      T11aPLCPParser, BB11aDemodCtx );
	CREATE_BRICK_FILTER (sviterbik, T11aViterbiSig, BB11aDemodCtx, plcp );
	CREATE_BRICK_FILTER (dibpsk, 	T11aDeinterleaveBPSK, BB11aDemodCtx, sviterbik );		
	CREATE_BRICK_FILTER (dmplcp, 	T11aDemapBPSK::Filter, BB11aDemodCtx, dibpsk );

//	CREATE_BRICK_FILTER ( itube1,  TInspectTube<64>::Filter, BB11aDemodCtx, dm6 );	
//	CREATE_BRICK_FILTER ( itube2,  TInspectTube<64>::Filter, BB11aDemodCtx, dm12 );	
//	CREATE_BRICK_FILTER ( itube3,  TInspectTube<64>::Filter, BB11aDemodCtx, dm24 );	
//	CREATE_BRICK_FILTER ( itube4,  TInspectTube<64>::Filter, BB11aDemodCtx, dm48 );	

	// monitor
//	CREATE_BRICK_SINK   ( mon,  TConstellation, BB11aDemodCtx );	
//	CREATE_BRICK_DEMUX2 ( tee,  TTee<64>::Filter, BB11aDemodCtx, dmplcp, mon );

//    CREATE_BRICK_DEMUX5 ( sigsel,	TBB11aRxRateSel,    BB11aDemodCtx,
//                          tee, itube1, drop, drop, drop );


    CREATE_BRICK_DEMUX5 ( sigsel,	TBB11aRxRateSel,    BB11aDemodCtx,
//                          tee, itube1, itube2, itube3, itube4 /*dmplcp, dm6, dm12, dm24, dm48*/ );
                          dmplcp, dm6, dm12, dm24, dm48 );

//    CREATE_BRICK_DEMUX5 ( sigsel,	TBB11aRxRateSel,    BB11aDemodCtx,
//                        dmplcp, dm6, dm12, dm24, dm48 );
	
	CREATE_BRICK_FILTER (pilot,  TPilotTrack, BB11aDemodCtx, sigsel );		
	CREATE_BRICK_FILTER (pcomp,  TPhaseCompensate, BB11aDemodCtx, pilot );	
	CREATE_BRICK_FILTER (chequ,  TChannelEqualization, BB11aDemodCtx, pcomp );		
	CREATE_BRICK_FILTER (fft,    TFFT64, BB11aDemodCtx, chequ );		
	CREATE_BRICK_FILTER (fcomp,	 TFreqCompensation, BB11aDemodCtx, fft );		
	CREATE_BRICK_FILTER (dsym,   T11aDataSymbol, BB11aDemodCtx, fcomp );		
	CREATE_BRICK_FILTER (dsym0,  TNoInline,      BB11aDemodCtx, dsym );		

	// LTS path
//	CREATE_BRICK_FILTER (fcfo,  TFineCFOEst, BB11aDemodCtx, drop );		
//	CREATE_BRICK_FILTER (chest, TChannelEst, BB11aDemodCtx, fcfo );	
//	CREATE_BRICK_FILTER (lsym,  T11aLTSymbol, BB11aDemodCtx, chest );	

	CREATE_BRICK_SINK (lsym,	T11aLTS, BB11aDemodCtx );	


	CREATE_BRICK_DEMUX2 (symsel, T11aSymSelEx::Filter, BB11aDemodCtx, 
			lsym, dsym0 );	

	CREATE_BRICK_FILTER (dcest, TDCEstimator, BB11aDemodCtx, drop );
	CREATE_BRICK_FILTER (cca,   TCCA11a,      BB11aDemodCtx, dcest );
	CREATE_BRICK_FILTER (dc,    TDCRemoveEx<4>::Filter, BB11aDemodCtx, cca );
    CREATE_BRICK_SOURCE (dummy, TDummySource, BB11aDemodCtx, cca );
    srcCarrierSense = dummy;

	CREATE_BRICK_DEMUX2 ( rxswt, TBB11bRxSwitch, BB11aDemodCtx,
			dc, symsel);
	
	if ( gDoubleDeci ) {
		CREATE_BRICK_FILTER (ds2, TDownSample2, BB11aDemodCtx, rxswt );
		CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, ds2);
		srcAll = ssrc;
	} else {
		CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, rxswt);
		srcAll = ssrc;
	}

	srcViterbi = vit0;
}

static inline
void CreateDemodGraph11a_10M (ISource*& srcAll, ISource*& srcViterbi, ISource*& srcCarrierSense)
{
    CREATE_BRICK_SINK  (drop, TDropAny,    BB11aDemodCtx );

    CREATE_BRICK_SINK   (fsink, TBB11aFrameSink, BB11aDemodCtx );	
	CREATE_BRICK_FILTER (desc,   T11aDesc,	BB11aDemodCtx, fsink );				

	typedef T11aViterbi <5000*8, 48, 256> T11aViterbiComm;
	CREATE_BRICK_FILTER (viterbi, T11aViterbiComm::Filter,	BB11aDemodCtx, desc );				

    CREATE_BRICK_FILTER (vit0, TThreadSeparator<>::Filter, BB11aDemodCtx, viterbi);

	// 6M
	CREATE_BRICK_FILTER (di6, 	T11aDeinterleaveBPSK, BB11aDemodCtx, vit0 );		
    CREATE_BRICK_FILTER (dm6, 	T11aDemapBPSK::Filter,      BB11aDemodCtx, di6 );

	
	// 12M
	CREATE_BRICK_FILTER (di12, 	T11aDeinterleaveQPSK, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm12, 	T11aDemapQPSK::Filter,      BB11aDemodCtx, di12 );
	
	// 24M
	CREATE_BRICK_FILTER (di24, 	T11aDeinterleaveQAM16, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm24, 	T11aDemapQAM16::Filter,      BB11aDemodCtx, di24 );
	
	// 48M
	CREATE_BRICK_FILTER (di48, 	T11aDeinterleaveQAM64, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm48, 	T11aDemapQAM64::Filter,      BB11aDemodCtx, di48 );
	
	// PLCP 
    CREATE_BRICK_SINK   (plcp,      T11aPLCPParser, BB11aDemodCtx );
	CREATE_BRICK_FILTER (sviterbik, T11aViterbiSig, BB11aDemodCtx, plcp );
	CREATE_BRICK_FILTER (dibpsk, 	T11aDeinterleaveBPSK, BB11aDemodCtx, sviterbik );		
	CREATE_BRICK_FILTER (dmplcp, 	T11aDemapBPSK::Filter, BB11aDemodCtx, dibpsk );

	CREATE_BRICK_FILTER ( itube0,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dmplcp );	
	CREATE_BRICK_FILTER ( itube1,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dm6 );	
	CREATE_BRICK_FILTER ( itube2,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dm12 );	
	CREATE_BRICK_FILTER ( itube3,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dm24 );	
	CREATE_BRICK_FILTER ( itube4,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dm48 );	

	itube0->SetName ( "plcp constellation" );
	itube1->SetName ( "data constellation" );
	itube2->SetName ( "data constellation" );
	itube3->SetName ( "data constellation" );
	itube4->SetName ( "data constellation" );

	// monitor
	/*CREATE_BRICK_SINK   ( mon,  TConstellation, BB11aDemodCtx );	
	CREATE_BRICK_DEMUX2 ( tee,  TTee<64>::Filter, BB11aDemodCtx, dmplcp, mon );*/

//    CREATE_BRICK_DEMUX5 ( sigsel,	TBB11aRxRateSel,    BB11aDemodCtx,
//                          tee, itube1, drop, drop, drop );


    CREATE_BRICK_DEMUX5 ( sigsel,	TBB11aRxRateSel,    BB11aDemodCtx,
                          itube0, itube1, itube2, itube3, itube4 );

    //CREATE_BRICK_DEMUX5 ( sigsel,	TBB11aRxRateSel,    BB11aDemodCtx,
    //                    dmplcp, dm6, dm12, dm24, dm48 );
	
	CREATE_BRICK_FILTER (pilot,  TPilotTrack, BB11aDemodCtx, sigsel );		
	CREATE_BRICK_FILTER (pcomp,  TPhaseCompensate, BB11aDemodCtx, pilot );	
	CREATE_BRICK_FILTER (chequ,  TChannelEqualization, BB11aDemodCtx, pcomp );		
	CREATE_BRICK_FILTER (fft,    TFFT64, BB11aDemodCtx, chequ );		
	CREATE_BRICK_FILTER (fcomp,	 TFreqCompensation, BB11aDemodCtx, fft );		
	CREATE_BRICK_FILTER (dsym,   T11aDataSymbol, BB11aDemodCtx, fcomp );		
	CREATE_BRICK_FILTER (dsym0,  TNoInline,      BB11aDemodCtx, dsym );		

	// LTS path
//	CREATE_BRICK_FILTER (fcfo,  TFineCFOEst, BB11aDemodCtx, drop );		
//	CREATE_BRICK_FILTER (chest, TChannelEst, BB11aDemodCtx, fcfo );	
//	CREATE_BRICK_FILTER (lsym,  T11aLTSymbol, BB11aDemodCtx, chest );	

	CREATE_BRICK_SINK (lsym,	T11aLTS, BB11aDemodCtx );	


	CREATE_BRICK_DEMUX2 (symsel, T11aSymSelEx::Filter, BB11aDemodCtx, 
			lsym, dsym0 );	

	CREATE_BRICK_FILTER (dcest, TDCEstimator, BB11aDemodCtx, drop );
	CREATE_BRICK_FILTER (cca,   TCCA11a,      BB11aDemodCtx, dcest );
	CREATE_BRICK_FILTER (dc,    TDCRemoveEx<4>::Filter, BB11aDemodCtx, cca );
    CREATE_BRICK_SOURCE (dummy, TDummySource, BB11aDemodCtx, cca );
    srcCarrierSense = dummy;

	CREATE_BRICK_DEMUX2 ( rxswt, TBB11bRxSwitch, BB11aDemodCtx,
			dc, symsel);

	if ( gDoubleDeci ) {
		CREATE_BRICK_FILTER (ds2, TDownSample4, BB11aDemodCtx, rxswt );
		CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, ds2 );
		srcAll = ssrc;
	} else {
		CREATE_BRICK_FILTER (ds2, TDownSample2, BB11aDemodCtx, rxswt );
		CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, ds2 );
		srcAll = ssrc;
	}
/*
	CREATE_BRICK_FILTER(lpass,  THanningWin<16>::Filter, BB11aDemodCtx, ds2 );   
	uint cutoff[2] = {6, 0};
	lpass->config ( TYPE_LOWPASS, cutoff, 20);

	CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, lpass);
	//CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, ds2 );
    srcAll = ssrc;
*/
	srcViterbi = vit0;
}


static inline
void CreateDemodGraph11a_5M (ISource*& srcAll, ISource*& srcViterbi, ISource*& srcCarrierSense)
{
    CREATE_BRICK_SINK  (drop, TDropAny,    BB11aDemodCtx );

    CREATE_BRICK_SINK   (fsink, TBB11aFrameSink, BB11aDemodCtx );	
	CREATE_BRICK_FILTER (desc,   T11aDesc,	BB11aDemodCtx, fsink );				

	typedef T11aViterbi <5000*8, 48, 256> T11aViterbiComm;
	CREATE_BRICK_FILTER (viterbi, T11aViterbiComm::Filter,	BB11aDemodCtx, desc );				

    CREATE_BRICK_FILTER (vit0, TThreadSeparator<>::Filter, BB11aDemodCtx, viterbi);

	// 6M
	CREATE_BRICK_FILTER (di6, 	T11aDeinterleaveBPSK, BB11aDemodCtx, vit0 );		
    CREATE_BRICK_FILTER (dm6, 	T11aDemapBPSK::Filter,      BB11aDemodCtx, di6 );
	
	// 12M
	CREATE_BRICK_FILTER (di12, 	T11aDeinterleaveQPSK, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm12, 	T11aDemapQPSK::Filter,      BB11aDemodCtx, di12 );
	
	// 24M
	CREATE_BRICK_FILTER (di24, 	T11aDeinterleaveQAM16, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm24, 	T11aDemapQAM16::Filter,      BB11aDemodCtx, di24 );
	
	// 48M
	CREATE_BRICK_FILTER (di48, 	T11aDeinterleaveQAM64, BB11aDemodCtx, vit0 );		
	CREATE_BRICK_FILTER (dm48, 	T11aDemapQAM64::Filter,      BB11aDemodCtx, di48 );
	
	// PLCP 
    CREATE_BRICK_SINK   (plcp,      T11aPLCPParser, BB11aDemodCtx );
	CREATE_BRICK_FILTER (sviterbik, T11aViterbiSig, BB11aDemodCtx, plcp );
	CREATE_BRICK_FILTER (dibpsk, 	T11aDeinterleaveBPSK, BB11aDemodCtx, sviterbik );		
	CREATE_BRICK_FILTER (dmplcp, 	T11aDemapBPSK::Filter, BB11aDemodCtx, dibpsk );

	CREATE_BRICK_FILTER ( itube0,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dmplcp );	
	CREATE_BRICK_FILTER ( itube1,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dm6 );	
	CREATE_BRICK_FILTER ( itube2,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dm12 );	
	CREATE_BRICK_FILTER ( itube3,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dm24 );	
	CREATE_BRICK_FILTER ( itube4,  TInspectTube<CPlotDots>::Filter, BB11aDemodCtx, dm48 );	

	itube0->SetName ( "plcp constellation" );
	itube1->SetName ( "data constellation" );
	itube2->SetName ( "data constellation" );
	itube3->SetName ( "data constellation" );
	itube4->SetName ( "data constellation" );

    CREATE_BRICK_DEMUX5 ( sigsel, TBB11aRxRateSel, BB11aDemodCtx,
                          itube0, itube1, itube2, itube3, itube4 );

    //CREATE_BRICK_DEMUX5 ( sigsel,	TBB11aRxRateSel,    BB11aDemodCtx,
    //                    dmplcp, dm6, dm12, dm24, dm48 );
	
	CREATE_BRICK_FILTER (pilot,  TPilotTrack, BB11aDemodCtx, sigsel );		
	CREATE_BRICK_FILTER (pcomp,  TPhaseCompensate, BB11aDemodCtx, pilot );	
	CREATE_BRICK_FILTER (chequ,  TChannelEqualization, BB11aDemodCtx, pcomp );		
	CREATE_BRICK_FILTER (fft,    TFFT64, BB11aDemodCtx, chequ );		
	CREATE_BRICK_FILTER (fcomp,	 TFreqCompensation, BB11aDemodCtx, fft );		
	CREATE_BRICK_FILTER (dsym,   T11aDataSymbol, BB11aDemodCtx, fcomp );		
	CREATE_BRICK_FILTER (dsym0,  TNoInline,      BB11aDemodCtx, dsym );		

	// LTS path
//	CREATE_BRICK_FILTER (fcfo,  TFineCFOEst, BB11aDemodCtx, drop );		
//	CREATE_BRICK_FILTER (chest, TChannelEst, BB11aDemodCtx, fcfo );	
//	CREATE_BRICK_FILTER (lsym,  T11aLTSymbol, BB11aDemodCtx, chest );	

	CREATE_BRICK_SINK (lsym,	T11aLTS, BB11aDemodCtx );	


	CREATE_BRICK_DEMUX2 (symsel, T11aSymSelEx::Filter, BB11aDemodCtx, 
			lsym, dsym0 );	

	CREATE_BRICK_FILTER (dcest, TDCEstimator, BB11aDemodCtx, drop );
	CREATE_BRICK_FILTER (cca,   TCCA11a,      BB11aDemodCtx, dcest );
	CREATE_BRICK_FILTER (dc,    TDCRemoveEx<4>::Filter, BB11aDemodCtx, cca );
    CREATE_BRICK_SOURCE (dummy, TDummySource, BB11aDemodCtx, cca );
    srcCarrierSense = dummy;

	CREATE_BRICK_DEMUX2 ( rxswt, TBB11bRxSwitch, BB11aDemodCtx,
			dc, symsel);

	if ( gDoubleDeci ) {
		CREATE_BRICK_FILTER (ds3, TDownSample8, BB11aDemodCtx, rxswt );
		CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, ds3);
		srcAll = ssrc;
	} else {
		CREATE_BRICK_FILTER (ds3, TDownSample4, BB11aDemodCtx, rxswt );
		CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, ds3);
		srcAll = ssrc;
	}
	
	//CREATE_BRICK_FILTER(lpass,  TKaiserWin<15>::Filter, BB11aDemodCtx, ds3 );   
	//uint cutoff[2] = {3, 0};
	//lpass->config ( TYPE_LOWPASS, cutoff, 20, 4);
    //CREATE_BRICK_SOURCE (ssrc, TRxStream, BB11aDemodCtx, lpass);

	srcViterbi = vit0;
}

SELECTANY ISource * pBB11aRxSource = NULL, * pBB11aViterbi = NULL, * pBB11aCarrierSense = NULL;
