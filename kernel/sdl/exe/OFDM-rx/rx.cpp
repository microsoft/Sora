#include <windows.h>

#include <stdio.h>
#include <conio.h>
#include <sora.h>
#include <brick.h>
#include <dspcomm.h>
#include <soratime.h>
#include <debugplotu.h>

// some stocked bricks
#include <stdbrick.hpp>

// Sora DSP Library
#include <sdl.hpp>

#include "radioinfo.h"
#include "rx.h"

COFDMRxChain ofdmRx;

//
// Symbol selector
//
struct sym_selector {
	FINL
	int operator()(COFDMRxChain& ctx) {
		if ( ctx.CF_11aSymState::symbol_type()  == CF_11aSymState::SYMBOL_TRAINING ) { 
			return PORT_0; 
		} else {
		 	return PORT_1;
		}
	}
};
	  
typedef TGDemux2<COMPLEX16, 4, sym_selector> T11aSymSelEx;


//
// plcp selector
//
struct plcp_selector {
	FINL
	int operator()(COFDMRxChain& ctx) {
		if ( ctx.CF_11RxPLCPSwitch::plcp_state()  == CF_11RxPLCPSwitch::plcp_header ) { 
			return PORT_0;
		} else {
			if ( ctx.CF_11aRxVector::data_rate_kbps() != 6000 ) {
				// not supported
				ctx.CF_Error::error_code () = E_ERROR_NOT_SUPPORTED;
			}
		 	return PORT_1;
        }
	}
};
	  
typedef TGDemux2<COMPLEX16, 64, plcp_selector> T11aPlcpSwitch;


ISource* CreateDemodGraph ( ISource*& decoder ) {
	CREATE_BRICK_SINK  (drop, TDropAny,    ofdmRx );

	CREATE_BRICK_SINK	(fsink, TBB11aFrameSink, ofdmRx );	
	CREATE_BRICK_FILTER (desc,	 T11aDesc,	ofdmRx, fsink ); 			

	// define Viterbi
	typedef T11aViterbi <5000*8, 48, 24> T11aViterbiComm;
	CREATE_BRICK_FILTER (viterbi, T11aViterbiComm::Filter, ofdmRx, desc );				
	CREATE_BRICK_FILTER (vit0, TThreadSeparator<>::Filter, ofdmRx, viterbi);
	
	// Data 6M
	CREATE_BRICK_FILTER (di6,	T11aDeinterleaveBPSK, ofdmRx, vit0 );		
	CREATE_BRICK_FILTER (dm6,	T11aDemapBPSK,		  ofdmRx, di6 );

	// PLCP 
	CREATE_BRICK_SINK	(plcp,		T11aPLCPParser, ofdmRx );
	CREATE_BRICK_FILTER (sviterbik, T11aViterbiSig, ofdmRx, plcp );
	CREATE_BRICK_FILTER (dibpsk,	T11aDeinterleaveBPSK, ofdmRx, sviterbik );		
	CREATE_BRICK_FILTER (dmplcp,	T11aDemapBPSK, ofdmRx, dibpsk );

	CREATE_BRICK_DEMUX2 (plcpsel, T11aPlcpSwitch::Filter, ofdmRx, 
			dmplcp, dm6 );	

	// Data symbol path
	CREATE_BRICK_FILTER (pilot,  TPilotTrack, ofdmRx, plcpsel );		
	CREATE_BRICK_FILTER (pcomp,  TPhaseCompensate, ofdmRx, pilot );	
	CREATE_BRICK_FILTER (chequ,  TChannelEqualization, ofdmRx, pcomp );		
	CREATE_BRICK_FILTER (fft,	 TFFT64, ofdmRx, chequ );
	CREATE_BRICK_FILTER (fcomp,  TFreqCompensation, ofdmRx, fft );		
	CREATE_BRICK_FILTER (dsym,	 T11aDataSymbol, ofdmRx, fcomp );		
	CREATE_BRICK_FILTER (dsym0,  TNoInline, 	 ofdmRx, dsym ); 	
	
	// LTS path
	CREATE_BRICK_SINK (lsym, T11aLTS, ofdmRx );	
	
	CREATE_BRICK_DEMUX2 (symsel, T11aSymSelEx::Filter, ofdmRx, 
			lsym, dsym0 );	

	// CCA path
	CREATE_BRICK_FILTER (dcest, TDCEstimator, ofdmRx, drop );
	CREATE_BRICK_FILTER (cca,	TCCA11a,	  ofdmRx, dcest );
	CREATE_BRICK_FILTER (dc,	TDCRemoveEx<4>::Filter, ofdmRx, cca );
	
	CREATE_BRICK_DEMUX2 ( rxswt, TBB11bRxSwitch, ofdmRx,
			dc, symsel);
	
	CREATE_BRICK_FILTER (ds2, TDownSample2, ofdmRx, rxswt );
	
	CREATE_BRICK_SOURCE(rxsrc, TRxStream,  ofdmRx, ds2 );	
	
	decoder = vit0;
	return rxsrc;
}


