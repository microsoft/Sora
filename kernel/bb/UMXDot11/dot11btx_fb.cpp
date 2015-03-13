#include <time.h>
#include <conio.h>
#include "soratime.h"
#include "stdbrick.hpp"
#include "CRC32.h"
#include "bb/bbb.h"
#include "bb/DataRate.h"
#include "monitor.h"
#include "config.h"
#include "dot11_pkt.h"
#include "thread_func.h"
#include "appext.h"

#include "scramble.hpp"
#include "pulse.hpp"
#include "phy_11b.hpp"
#include "samples.hpp"
#include "barkerspread.hpp"
#include "cck.hpp"

static PVOID				SampleBuffer = NULL;
static ULONG				SampleBufferSize = 0;
static ULONG			    TxID = 0;
static UCHAR                DataBuffer[INPUT_MAX];

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

BOOLEAN __stdcall DoDot11BTx_FineBrick(void* ctx)
{
    // Alias
    Monitor& monitor = ((TxContext *)ctx)->monitor;
    const Config& config = ((TxContext *)ctx)->config;

    monitor.Query(true);

    HRESULT hr = SoraURadioTx(TARGET_RADIO, TxID);
    if (FAILED(hr))
    {
        printf("[dot11b:tx] tx failed\n");
        //break;
    }
    else
    {
        monitor.IncGoodCounter();
        monitor.IncThroughput(config.GetPayloadLength());
    }

    SoraStallWait(&tsinfo, 10000); // not to send too fast
    return TRUE;
}

// Prepare data buffer with CRC32
static void Dot11BPrepareDataBuffer(PUCHAR DataBuffer, unsigned int dataSize, PVOID pSymbolBuffer, ULONG SymbolBufferSize)
{
    ULONG	i;
    PUCHAR  pbIn = DataBuffer;

    assert(DataBufferSize >= dataSize + 4);
    for (i = 0; i < dataSize; i++)
    {
        *pbIn = 'B';
        pbIn++;
    }

    PDOT11RFC1042ENCAP pWlanHeader = (PDOT11RFC1042ENCAP)DataBuffer;
    pWlanHeader->MacHeader.FrameControl.Type = FRAME_DATA;

    ULONG crc = CalcCRC32(DataBuffer, dataSize);
    *(ULONG *)(DataBuffer + dataSize) = crc;
}

static int Dot11BTxInit()
{
    SampleBufferSize = _M(2);
    SampleBuffer = SoraUAllocBuffer(SampleBufferSize);
    printf("[dot11b:tx] tx buffer: %08x\n", SampleBuffer);
    printf("[dot11b:tx] tx buffer size: %08x\n", SampleBufferSize);
    if (SampleBuffer == NULL) return -1;

    if (SampleBufferSize < BB11B_MAX_SYMBOL_LENGTH)
    {
        printf("[dot11b:tx] Buffer size not enough.\n");
        return -1;
    }

    return 0;
}

static void Dot11BTxClean()
{
    SoraUReleaseBuffer((PVOID)SampleBuffer);
}

void Dot11BTxApp_FineBrick(const Config& config)
{
    HRESULT hr;

    if (Dot11BTxInit() < 0)
        return;

    ISource *src = CreateModGraph();

    Dot11BPrepareDataBuffer(DataBuffer, config.GetPayloadLength(), SampleBuffer, SampleBufferSize);
    InitModGraphCtx(config.GetDataRate(), DataBuffer, config.GetPayloadLength(), (COMPLEX8 *)SampleBuffer, SampleBufferSize);

    do
    {
        // Generate Signal
        src->Reset();
        if (BB11bModCtx.CF_Error::error_code() != E_ERROR_SUCCESS)
        {
            printf("[ERROR] reset brick graph\n");
            break;
        }

        src->Process();
        src->Flush();
        printf("[dot11b:tx] GenSignal return\n");
        ULONG sampleSize = BB11bModCtx.tx_sample_cnt() * sizeof(COMPLEX8);
        printf("[dot11b:tx] Signal bytes=%d\n", sampleSize);

        hr = SoraURadioTransferEx(TARGET_RADIO, SampleBuffer, sampleSize, &TxID);
        printf("[dot11b:tx] transfer, hr=%08x, id=%d\n", hr, TxID);
        FAILED_BREAK(hr);

        // Start TX thread
        Monitor monitor;
        TxContext ctx(config, monitor);
        HANDLE hTxThread = AllocStartThread(DoDot11BTx_FineBrick, &ctx);

        if (SUCCEEDED(hr))
        {
            printf("\n\nPress any key to exit the program\n");			
            time_t start = time(NULL);
	        while(!_kbhit())
            {
                if (config.Interval() != 0 && difftime(time(NULL), start) >= config.Interval()) break;
	        }

            StopFreeThread(hTxThread);

            hr = SoraURadioTxFree(TARGET_RADIO, TxID);
            printf("[dot11b:tx] tx free return %08x\n", hr);
        }
    } while(false);

    Dot11BTxClean();

    printf("[dot11b:tx] Tx out.\n");
}
