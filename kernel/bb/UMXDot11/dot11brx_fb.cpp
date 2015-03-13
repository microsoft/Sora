#pragma once

#include <time.h>
#include <conio.h>
#include "brick.h"
#include "stdbrick.hpp"
#include "monitor.h"
#include "thread_func.h"
#include "appext.h"

#include "cca.hpp"
#include "barkerspread.hpp"
#include "scramble.hpp"
#include "symtiming.hpp"
#include "sfd_sync.hpp"
#include "phy_11b.hpp"
#include "cck.hpp"

#include "ieee80211facade.hpp"

#define OUT_BUF_SIZE (16*1024*1024)
static UCHAR                    OutputBuf[OUT_BUF_SIZE];
static SORA_RADIO_RX_STREAM		RxStream;
static PVOID				    RxBuffer = NULL;
static ULONG				    RxBufferSize = 0;
static ISource *                pRxSource = NULL;

SELECTANY
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
    , LOCAL_CONTEXT(TRxStream)
    , LOCAL_CONTEXT(TCCK5P5Decoder)
    , LOCAL_CONTEXT(TCCK11Decoder)
{
    // Statistic monitor for frames
    Monitor monitor;

    FINL void MoveState_RX_CS(bool finished, bool good)
    {
        QueryMacStopwatch();
        if (finished)
        {
            if (good)
            {
                monitor.IncGoodCounter();
                monitor.IncThroughput(CF_11bRxVector::frame_length() - 4); // Accumulate viterbi thread output length, excluding CRC32
            }
            else
            {
                monitor.IncBadCounter();
            }
        }
    }

    FINL void QueryMacStopwatch()
    {
        monitor.Query(false); // false means receiving
    }

    _tagBB11bDemodContext()
    {
    }

    FINL void OnPowerDetected()
    {
        QueryMacStopwatch();
    }

    bool reset ( ) 
    {
 	    // CF_Error
	    CF_Error::error_code() = E_ERROR_SUCCESS;
	  
	    // CF_11CCA
	    CF_11CCA::cca_state() = CF_11CCA::power_clear;
	  
	    // CF_11bRxMRSel
	    CF_11bRxMRSel::rxrate_state() = CF_11bRxMRSel::rate_sync;
	    // CF_11RxPLCPSwitch
	    CF_11RxPLCPSwitch::plcp_state() = CF_11RxPLCPSwitch::plcp_header;

	    return true;
	  
    }

	bool init ( PSORA_RADIO_RX_STREAM pRxStream, UCHAR* output, uint out_size ) 
	{
		// CF_RxStream
		CF_RxStream::rxstream_pointer() = pRxStream;
		CF_RxStream::rxstream_touched() = 0;

		// CF_VecDC
		vcs& vdc = CF_VecDC::direct_current();
		set_zero(vdc);

		// CF_RxFrameBuffer
		CF_RxFrameBuffer::rx_frame_buf() = output;
		CF_RxFrameBuffer::rx_frame_buf_size() = out_size;

	    CF_11CCA::cca_pwr_threshold() = 1000*1000*4;

		return reset ();
	}
} BB11bDemodCtx;

static ULONG				PowerThreshold	= 4000;
static ULONG				PowerThresholdLH= 0xFFFFFFF;
static ULONG				PowerThresholdHL= 4000;
static ULONG				ShiftRight		= 4;

/*************************************************************************
Processing graph

src->dc_removedc-> rxswitch -> estimate_dc -> energy_det
               |-> symtiming -> barker_sync -> barker_despread -> MrSel...

 ...MrSel -> SFD
		|--> bpsk_demapper -> dsc741 -> plcp_swt -> hdr 
		|__> qpsk_demapper _|                  |--> data_collector
*************************************************************************/
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
    CREATE_BRICK_SOURCE(ssrc, TRxStream, BB11bDemodCtx, dc);

    return ssrc;
}

static int Dot11BRxInit()
{
	HRESULT hr;

	hr = SoraURadioMapRxSampleBuf(TARGET_RADIO, &RxBuffer, &RxBufferSize);
    printf("Map Buffer ret : %08x\n", hr);
	printf("Rxbuf: %08x, size: %08x\n", RxBuffer, RxBufferSize);

	if (FAILED(hr)) return -1;

	printf("%08x, %08x, %08x, %08x\n", 
            ((PULONG)RxBuffer)[0], 
            ((PULONG)RxBuffer)[1], 
            ((PULONG)RxBuffer)[2], 
            ((PULONG)RxBuffer)[3]);

	hr = SoraURadioAllocRxStream(&RxStream, TARGET_RADIO, (PUCHAR)RxBuffer, RxBufferSize);
    printf("SoraURadioAllocRxStream ret: %08x\n", hr);
    if (FAILED(hr)) return -2;

    printf("Rx Context Init ended.\n");

	return 0;
}

BOOLEAN MAC11b_Receive ( void * ) {
    pRxSource->Seek(ISource::END_POS);
    while (1) {
        BB11bDemodCtx.QueryMacStopwatch();

	    pRxSource->Process ();
	    ulong& err = BB11bDemodCtx.CF_Error::error_code();
	    if ( err != E_ERROR_SUCCESS ) {
			// Any event happened
		    if ( err == E_ERROR_FRAME_OK ) {
			    // One frame is received
                BB11bDemodCtx.MoveState_RX_CS(true, true);
            } else if ( err == E_ERROR_CRC32_FAIL) {
                BB11bDemodCtx.MoveState_RX_CS(true, false);
		    } else if ( err == E_ERROR_CS_TIMEOUT ) {
                // Channel clean
		    } else {
                BB11bDemodCtx.MoveState_RX_CS(false, false);
                //printf("err = %08X\n", err);
		    }

            if (err == E_ERROR_FRAME_OK || err == E_ERROR_CRC32_FAIL)
            {
                // TRICK: jump advance of the last CRC byte, because we uses spcialized CRC module
                switch(BB11bDemodCtx.CF_11bRxVector::data_rate_kbps())
                {
                case 1000:
                    pRxSource->Seek(8 * 11 * 4);
                    break;
                case 2000:
                    pRxSource->Seek(4 * 11 * 4);
                    break;
                case 5500:
                    pRxSource->Seek(8 * 2 * 4);
                    break;
                case 11000:
                    pRxSource->Seek(8 * 1 * 4);
                    break;
                }
            }

		    BB11bDemodCtx.reset ();
		    pRxSource->Reset ();
			break;
        }
    }
    return TRUE;
}

void Dot11BRxApp_FineBrick(const Config& config)
{
	HRESULT		hr;
	if (Dot11BRxInit() < 0)
		goto out;

    BB11bDemodCtx.init(&RxStream, OutputBuf, OUT_BUF_SIZE);
    pRxSource = CreateDemodGraph();
	pRxSource->Reset();

    HANDLE hThread = AllocStartThread(MAC11b_Receive, pRxSource);
	if (!hThread) goto out;

    printf("\n\nPress any key to exit the program\n");
    time_t start = time(NULL);
	while(!_kbhit())
    {
        // Timeout if interval configured
        if (config.Interval() != 0 && difftime(time(NULL), start) >= config.Interval()) break;
	}

out:
    StopFreeThread(hThread);
    IReferenceCounting::Release(pRxSource);
    SoraURadioReleaseRxStream(&RxStream, TARGET_RADIO);
	hr = SoraURadioUnmapRxSampleBuf(TARGET_RADIO, RxBuffer);
	printf("Unmap rx buffer ret: %08x\n", hr);
	printf("Rx out.\n");
}
