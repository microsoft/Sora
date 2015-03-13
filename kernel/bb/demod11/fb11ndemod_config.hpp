#pragma once
#include "brick.h"
#include "const.h"
#include "stdbrick.hpp"
#include "ieee80211facade.hpp"
#include "autocorr.hpp"
#include "depuncturer.hpp"
#include "PHY_11a.hpp"
#include "PHY_11b.hpp"
#include "PHY_11n.hpp"
#include "viterbi.hpp"
#include "pilot.hpp"
#include "pilot_11n.hpp"
#include "freqoffset.hpp"
#include "freqoffset_11n.hpp"
#include "channel_11a.hpp"
#include "channel_11n.hpp"
#include "cca.hpp"
#include "cca_11n.hpp"
#include "fft.hpp"
#include "scramble.hpp"
#include "deinterleaver.hpp"
#include "deinterleaver_11n.hpp"
#include "samples.hpp"
#include "sampling.hpp"
#include "MACStopwatch.h"
#include "demapper11n.hpp"

typedef struct _tagBB11nDemodContext :
      LOCAL_CONTEXT(TDropAny)
    , LOCAL_CONTEXT(TBB11aFrameSink)									  	  	
    , LOCAL_CONTEXT(T11aViterbi)									  
    , LOCAL_CONTEXT(T11nSigParser)	  		  		  		  		  	
    , LOCAL_CONTEXT(TSisoChannelEst)
    , LOCAL_CONTEXT(TFFT64)  		
    , LOCAL_CONTEXT(TMemSamples2)
    , LOCAL_CONTEXT(TCCA11n)
    , LOCAL_CONTEXT(TFreqEstimator_11n)
    , LOCAL_CONTEXT(T11nDataSymbol)
    , LOCAL_CONTEXT(TMimoChannelEst)
    , LOCAL_CONTEXT(TPilotTrack_11n)
{
    MACStopwatch stopwatch;

    FINL void MoveState_RX_CS(bool finished, bool good)
    {
        stopwatch.LeaveRX(CF_MemSamples::mem_sample_index(), finished, good);
        stopwatch.EnterCS(CF_MemSamples::mem_sample_index());
    }

    FINL void MoveState_CS_RX()
    {
        stopwatch.LeaveCS(CF_MemSamples::mem_sample_index());
        stopwatch.EnterRX(CF_MemSamples::mem_sample_index());
    }

    FINL void InitStopwatch()
    {
        stopwatch.EnterCS(0);
    }

    _tagBB11nDemodContext()
    {
        InitStopwatch();
    }

    FINL void OnPowerDetected()
    {
        stopwatch.LeaveCS(CF_MemSamples::mem_sample_index());
        stopwatch.EnterRX(CF_MemSamples::mem_sample_index());
    }

    void Reset () {
        // Reset all CFacade data in the context
        ResetCarrierSense();

        // CF_11aRxVector
        CF_11aRxVector::Reset();
    }

    void ResetCarrierSense() {
		// CF_Error
		CF_Error::error_code() = E_ERROR_SUCCESS;

        // CF_11CCA
		CF_11CCA::Reset();

        CF_11nSymState::Reset();
	}

    void Init ( UCHAR* output, uint out_size ) 
    {
        // CF_RxFrameBuffer
        CF_RxFrameBuffer::Init (output, out_size);

        Reset ();
        
    }
}BB11nDemodContext;
      
SELECTANY BB11nDemodContext BB11nDemodCtx;

struct rx_switch {
    FINL
    int operator()(BB11nDemodContext& ctx) {
        if ( ctx.cca_state() == CF_11CCA::power_clear) {
            return PORT_0;
        } else if ( ctx.CF_11nSymState::symbol_type()  == CF_11nSymState::SYMBOL_L_LTF ) { 
            return PORT_1;
        } else {
            return PORT_2;
        }
    }
};
typedef TGDemux<4, COMPLEX16, 4, rx_switch> RxSwitch;

struct sym_selector_11n {
    FINL
    int operator()(BB11nDemodContext& ctx) {
        switch (ctx.CF_11nSymState::symbol_type()) {
        case CF_11nSymState::SYMBOL_SIG:
            return PORT_0;
        case CF_11nSymState::SYMBOL_HT_STF:
            ctx.CF_11nSymState::symbol_type() = CF_11nSymState::SYMBOL_HT_LTF;
            return PORT_1;
        case CF_11nSymState::SYMBOL_HT_LTF:
            return PORT_2;
        case CF_11nSymState::SYMBOL_OFDM_DATA:
            return PORT_3;
        default: NODEFAULT;
        }
    }
};
typedef TGDemux<4, COMPLEX16, 64, sym_selector_11n> T11nSymSel;

struct rate_selector {
    FINL
    int operator()(BB11nDemodContext& ctx) {
        switch (ctx.CF_HTRxVector::ht_frame_mcs()) {
        case 8: return PORT_0;
        case 9: case 10: return PORT_1;
        case 11: case 12: return PORT_2;
        case 13: case 14: return PORT_3;
        default: NODEFAULT;
        }
    }
};
typedef TGDemux<4, COMPLEX16, 64, rate_selector> T11nRxRateSel;
typedef TStreamJoin<2, 64> Mimo64Join;

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
static inline
void CreateDemodGraph11n (ISource*& srcAll, ISource*& srcViterbi, IControlPoint*& srcCarrierSense)
{
    CREATE_BRICK_SINK   (drop,          TDropAny,    BB11nDemodCtx );
    CREATE_BRICK_SINK   (cca,	        TCCA11n, BB11nDemodCtx );	
    srcCarrierSense = cca;

    // L_LTF branch
    CREATE_BRICK_SINK (sisoest,	        TSisoChannelEst, BB11nDemodCtx );
    CREATE_BRICK_FILTER (llft_join,     Mimo64Join::Filter, BB11nDemodCtx, sisoest);
    CREATE_BRICK_FILTER (llft_fft0,     TFFT64, BB11nDemodCtx, llft_join );
    CREATE_BRICK_FILTER (llft_fft1,     TFFT64, BB11nDemodCtx, llft_join );
    CREATE_BRICK_DEMUX2 (llft_fork,     TStreamFork<2>::Filter, BB11nDemodCtx, llft_fft0, llft_fft1);
    CREATE_BRICK_FILTER (llft_freqcomp, TFreqComp_11n, BB11nDemodCtx, llft_fork );
    CREATE_BRICK_FILTER (freqest,       TFreqEstimator_11n, BB11nDemodCtx, llft_freqcomp );
                                        
    // L_SIG HT_SIG Data branch         
    CREATE_BRICK_SINK   (sigparser,     T11nSigParser, BB11nDemodCtx );
    CREATE_BRICK_FILTER (sviterbik,     T11nViterbiSig, BB11nDemodCtx, sigparser );
    CREATE_BRICK_FILTER (sigdi, 	    T11aDeinterleaveBPSK, BB11nDemodCtx, sviterbik );		
    CREATE_BRICK_FILTER (sigdemap0,     T11nSigDemap, BB11nDemodCtx, sigdi);
    CREATE_BRICK_FILTER (sigdemap,      TNoInline,      BB11nDemodCtx, sigdemap0 );
    CREATE_BRICK_FILTER (mrc,           TMrcCombine, BB11nDemodCtx, sigdemap);
    CREATE_BRICK_FILTER (sisocomp0,     TSisoChannelComp, BB11nDemodCtx, mrc);
    CREATE_BRICK_FILTER (sisocomp,      TNoInline,      BB11nDemodCtx, sisocomp0 );		
                                        
    // HT_LTF branch                    
    CREATE_BRICK_SINK (mimoest,         TMimoChannelEst, BB11nDemodCtx );

    // HT_DATA branch
    typedef TStreamConcat<2, 1>  StreamConcat_1;
    typedef TStreamConcat<2, 2>  StreamConcat_2;
    typedef TStreamConcat<2, 3>  StreamConcat_3;
    typedef T11aViterbi <5000*8, 52*6, 24*8, 36> T11aViterbiComm;

    CREATE_BRICK_SINK   (fsink,         TBB11aFrameSink, BB11nDemodCtx );	
    CREATE_BRICK_FILTER (desc,          T11aDesc,	BB11nDemodCtx, fsink );
    CREATE_BRICK_FILTER (viterbi,       T11aViterbiComm::Filter,	BB11nDemodCtx, desc );				
    CREATE_BRICK_FILTER (vit0,          TThreadSeparator<>::Filter,  BB11nDemodCtx, viterbi);
    CREATE_BRICK_FILTER (join_1,        StreamConcat_1::Filter, BB11nDemodCtx, vit0);
    CREATE_BRICK_FILTER (join_2,        StreamConcat_2::Filter, BB11nDemodCtx, vit0);
    CREATE_BRICK_FILTER (join_3,        StreamConcat_3::Filter, BB11nDemodCtx, vit0);
    CREATE_BRICK_FILTER (join_bpsk,     TStreamJoin<2 MACRO_COMMA 52>::Filter,     BB11nDemodCtx, join_1);
    CREATE_BRICK_FILTER (join_qpsk,     TStreamJoin<2 MACRO_COMMA 52*2>::Filter,   BB11nDemodCtx, join_1);
    CREATE_BRICK_FILTER (join_qam16,    TStreamJoin<2 MACRO_COMMA 52*4>::Filter,   BB11nDemodCtx, join_2);
    CREATE_BRICK_FILTER (join_qam64,    TStreamJoin<2 MACRO_COMMA 52*6>::Filter,   BB11nDemodCtx, join_3);
    CREATE_BRICK_FILTER (di_bpsk_0,     T11nDeinterleaveBPSK_S0,    BB11nDemodCtx, join_bpsk    );		
    CREATE_BRICK_FILTER (di_bpsk_1,     T11nDeinterleaveBPSK_S1,    BB11nDemodCtx, join_bpsk    );		
    CREATE_BRICK_FILTER (di_qpsk_0,     T11nDeinterleaveQPSK_S0,    BB11nDemodCtx, join_qpsk   );		
    CREATE_BRICK_FILTER (di_qpsk_1,     T11nDeinterleaveQPSK_S1,    BB11nDemodCtx, join_qpsk   );		
    CREATE_BRICK_FILTER (di_qam16_0,    T11nDeinterleaveQAM16_S0,   BB11nDemodCtx, join_qam16   );		
    CREATE_BRICK_FILTER (di_qam16_1,    T11nDeinterleaveQAM16_S1,   BB11nDemodCtx, join_qam16   );		
    CREATE_BRICK_FILTER (di_qam64_0,    T11nDeinterleaveQAM64_S0,   BB11nDemodCtx, join_qam64   );		
    CREATE_BRICK_FILTER (di_qam64_1,    T11nDeinterleaveQAM64_S1,   BB11nDemodCtx, join_qam64   );		
    CREATE_BRICK_FILTER (dm_bpsk_0_,    T11nDemapBPSK,   BB11nDemodCtx, di_bpsk_0    );
    CREATE_BRICK_FILTER (dm_bpsk_1_,    T11nDemapBPSK,   BB11nDemodCtx, di_bpsk_1    );
    CREATE_BRICK_FILTER (dm_qpsk_0_,    T11nDemapQPSK,   BB11nDemodCtx, di_qpsk_0    );
    CREATE_BRICK_FILTER (dm_qpsk_1_,    T11nDemapQPSK,   BB11nDemodCtx, di_qpsk_1    );
    CREATE_BRICK_FILTER (dm_qam16_0_,   T11nDemapQAM16,  BB11nDemodCtx, di_qam16_0   );
    CREATE_BRICK_FILTER (dm_qam16_1_,   T11nDemapQAM16,  BB11nDemodCtx, di_qam16_1   );
    CREATE_BRICK_FILTER (dm_qam64_0_,   T11nDemapQAM64,  BB11nDemodCtx, di_qam64_0   );
    CREATE_BRICK_FILTER (dm_qam64_1_,   T11nDemapQAM64,  BB11nDemodCtx, di_qam64_1   );
    CREATE_BRICK_FILTER (dm_bpsk_0,     TNoInline,      BB11nDemodCtx, dm_bpsk_0_    );		
    CREATE_BRICK_FILTER (dm_bpsk_1,     TNoInline,      BB11nDemodCtx, dm_bpsk_1_    );		
    CREATE_BRICK_FILTER (dm_qpsk_0,     TNoInline,      BB11nDemodCtx, dm_qpsk_0_    );		
    CREATE_BRICK_FILTER (dm_qpsk_1,     TNoInline,      BB11nDemodCtx, dm_qpsk_1_    );		
    CREATE_BRICK_FILTER (dm_qam16_0,    TNoInline,      BB11nDemodCtx, dm_qam16_0_   );		
    CREATE_BRICK_FILTER (dm_qam16_1,    TNoInline,      BB11nDemodCtx, dm_qam16_1_   );		
    CREATE_BRICK_FILTER (dm_qam64_0,    TNoInline,      BB11nDemodCtx, dm_qam64_0_   );		
    CREATE_BRICK_FILTER (dm_qam64_1,    TNoInline,      BB11nDemodCtx, dm_qam64_1_   );		
    CREATE_BRICK_DEMUX2 (fork_bpsk,     TStreamFork<2>::Filter, BB11nDemodCtx, dm_bpsk_0, dm_bpsk_1);
    CREATE_BRICK_DEMUX2 (fork_qpsk,     TStreamFork<2>::Filter, BB11nDemodCtx, dm_qpsk_0, dm_qpsk_1);
    CREATE_BRICK_DEMUX2 (fork_qam16,    TStreamFork<2>::Filter, BB11nDemodCtx, dm_qam16_0, dm_qam16_1);
    CREATE_BRICK_DEMUX2 (fork_qam64,    TStreamFork<2>::Filter, BB11nDemodCtx, dm_qam64_0, dm_qam64_1);
    CREATE_BRICK_DEMUX4 (ratesel,       T11nRxRateSel::Filter,  BB11nDemodCtx,
            fork_bpsk, fork_qpsk, fork_qam16, fork_qam64);
    CREATE_BRICK_FILTER (pilot,         TPilotTrack_11n, BB11nDemodCtx, ratesel);
    CREATE_BRICK_FILTER (mimocomp0,     TMimoChannelComp, BB11nDemodCtx, pilot);
    CREATE_BRICK_FILTER (mimocomp,      TNoInline,      BB11nDemodCtx, mimocomp0 );		


    CREATE_BRICK_DEMUX4 (symsel,        T11nSymSel::Filter, BB11nDemodCtx, 
            sisocomp, drop, mimoest, mimocomp );
    CREATE_BRICK_FILTER (ofdm_join,     Mimo64Join::Filter, BB11nDemodCtx, symsel);
    CREATE_BRICK_FILTER (ofdm_fft0,     TFFT64, BB11nDemodCtx, ofdm_join );
    CREATE_BRICK_FILTER (ofdm_fft1,     TFFT64, BB11nDemodCtx, ofdm_join );
    CREATE_BRICK_DEMUX2 (ofdm_fork,     TStreamFork<2>::Filter, BB11nDemodCtx, ofdm_fft0, ofdm_fft1);
    CREATE_BRICK_FILTER (sym0,          T11nDataSymbol, BB11nDemodCtx, ofdm_fork );
    CREATE_BRICK_FILTER (sym,           TNoInline,      BB11nDemodCtx, sym0 );		
    CREATE_BRICK_FILTER (freqcomp,      TFreqComp_11n, BB11nDemodCtx, sym );

    CREATE_BRICK_DEMUX3 (rxswt, RxSwitch::Filter, BB11nDemodCtx,
        cca, freqest, freqcomp);
    CREATE_BRICK_FILTER (ds2,  TDownSample2, BB11nDemodCtx, rxswt );
    CREATE_BRICK_SOURCE (fsrc, TMemSamples2, BB11nDemodCtx, ds2 );

    srcAll = fsrc;
    srcViterbi = vit0;
}
