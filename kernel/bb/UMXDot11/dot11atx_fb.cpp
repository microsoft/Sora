#include <time.h>
#include <conio.h>
// #include "stockbrk.hpp"
#include "stdbrick.hpp"
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
#include "dot11_pkt.h"
#include "scramble.hpp"
#include "samples.hpp"
#include "monitor.h"
#include "appext.h"
#include "thread_func.h"

static PVOID				SampleBuffer = NULL;
static ULONG				SampleBufferSize = 0;
static ULONG			    TxID = 0;
static UCHAR                DataBuffer[INPUT_MAX];
static ISource *            tssrc;
static ISource *            ssrc;

//
// configuration for the mod pipeline
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

BOOLEAN __stdcall DoDot11ATx_FineBrick(void* ctx)
{
    // Alias
    Monitor& monitor = ((TxContext *)ctx)->monitor;
    const Config& config = ((TxContext *)ctx)->config;

    monitor.Query(true);

    // Change sequence number
    ((PDOT11RFC1042ENCAP)DataBuffer)->MacHeader.SequenceControl.SequenceNumber++;

    // Generate Signal
    HRESULT hr;
	COMPLEX8* pSymBuf = (COMPLEX8 *)SampleBuffer;

	BB11aModCtx.set_mod_buffer (pSymBuf, SampleBufferSize );
	tssrc->Reset ();
	if ( BB11aModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
		tssrc->Process ();
	}	
	uint TS_len = BB11aModCtx.CF_TxSampleBuffer::tx_sample_cnt();

	pSymBuf += TS_len;
	
    BB11aModCtx.init ( config.GetDataRate(), DataBuffer, (ushort) config.GetPayloadLength(), pSymBuf, SampleBufferSize - TS_len);
	
    ssrc->Reset ();
	if ( BB11aModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
		ssrc->Process ();
		ssrc->Flush ();
	} else {
        printf("[ERROR] reset brick graph\n");
        return FALSE;
	}

    //printf("[dot11b:tx] GenSignal return\n");
    ULONG sampleSize = (BB11aModCtx.CF_TxSampleBuffer::tx_sample_cnt() + TS_len) * sizeof(COMPLEX8);

    //printf("[dot11b:tx] Signal bytes=%d\n", sampleSize);

    hr = SoraURadioTransferEx(TARGET_RADIO, SampleBuffer, sampleSize, &TxID);
    if (FAILED(hr))
    {
        printf("[dot11b:tx] transfer, hr=%08x, id=%d\n", hr, TxID);
        return FALSE;
    }

    hr = SoraURadioTx(TARGET_RADIO, TxID);
    if (FAILED(hr))
    {
        printf("[dot11b:tx] tx failed\n");
        return FALSE;
    }
    else
    {
        monitor.IncGoodCounter();
        monitor.IncThroughput(config.GetPayloadLength());
    }

    hr = SoraURadioTxFree(TARGET_RADIO, TxID);
    //printf("[dot11b:tx] tx free return %08x\n", hr);

    SoraStallWait(&tsinfo, 10000); // not to send too fast
    return TRUE;
}

// Prepare data buffer without CRC32
static void Dot11APrepareDataBuffer(PUCHAR DataBuffer, unsigned int dataSize, PVOID pSymbolBuffer, ULONG SymbolBufferSize)
{
    ULONG	i;
    PUCHAR  pbIn = DataBuffer;

    assert(DataBufferSize >= dataSize);
    for (i = 0; i < dataSize; i++)
    {
        *pbIn = 'B';
        pbIn++;
    }

    PDOT11RFC1042ENCAP pWlanHeader = (PDOT11RFC1042ENCAP)DataBuffer;
    pWlanHeader->MacHeader.FrameControl.Type = FRAME_DATA;
}

static int Dot11ATxInit()
{
    SampleBufferSize = _M(2);
    SampleBuffer = SoraUAllocBuffer(SampleBufferSize);
    printf("[dot11a:tx] tx buffer: %08x\n", SampleBuffer);
    printf("[dot11a:tx] tx buffer size: %08x\n", SampleBufferSize);
    if (SampleBuffer == NULL) return -1;
    return 0;
}

static void Dot11ATxClean()
{
    SoraUReleaseBuffer((PVOID)SampleBuffer);
}

void Dot11ATxApp_Brick(const Config& config)
{
    if (Dot11ATxInit() < 0)
        return;

    if (config.GetSampleRate() == 40) {
	    ssrc = CreateModGraph11a_40M ();
        tssrc = CreatePreamble11a_40M ();
    }
    else {
        assert(config.GetSampleRate() == 44);
	    ssrc = CreateModGraph11a_44M ();
        tssrc = CreatePreamble11a_44M ();
    }

    Dot11APrepareDataBuffer(DataBuffer, config.GetPayloadLength(), SampleBuffer, SampleBufferSize);
    BB11aModCtx.init ( config.GetDataRate(), DataBuffer, (ushort) config.GetPayloadLength(), (COMPLEX8 *)SampleBuffer, SampleBufferSize);

    do
    {
        // Start TX thread
        Monitor monitor;
        TxContext ctx(config, monitor);
        HANDLE hTxThread = AllocStartThread(DoDot11ATx_FineBrick, &ctx);

        printf("\n\nPress any key to exit the program\n");			
        time_t start = time(NULL);
	    while(!_kbhit())
        {
            if (config.Interval() != 0 && difftime(time(NULL), start) >= config.Interval()) break;
	    }

        StopFreeThread(hTxThread);
    } while(false);

    Dot11ATxClean();

    printf("[dot11b:tx] Tx out.\n");
}
