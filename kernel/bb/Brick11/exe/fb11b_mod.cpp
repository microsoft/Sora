#include <stdio.h>
#include <assert.h>
#include <windows.h>

#include <dspcomm.h>
#include <brick.h>
#include <stdbrick.hpp>

#include <soratime.h>

#include "scramble.hpp"
#include "pulse.hpp"

#include "sfd_sync.hpp"
#include "phy_11b.hpp"
#include "samples.hpp"
#include "barkerspread.hpp"

#include "phy_11a.hpp"
#include "fb11bmod_config.hpp"
#include "fb11amod_config.hpp"


extern TIMESTAMPINFO tsinfo;

FINL void FB_11B_ModulateBuffer ( ISource * src)
{
    src->Process (); // Just modulate it
    src->Flush ();
}

// Note: defined as static variables to prevent TestMod11B() stack too large
const int SAMPLE_BUFFER_SIZE = 2*1024*1024;
static UCHAR               DataBuffer[INPUT_MAX];
static A16 COMPLEX8        SymbolBuffer[SAMPLE_BUFFER_SIZE];

const int RUN_TIMES = 100;
int Test11B_FB_Mod ()
{
    printf ( "Modulate 11b with Fine Brick!\n" );
    
    ISource * ssrc = CreateModGraph ();
    
    int bCnt = LoadDumpFile ("d:\\test.txt", DataBuffer, INPUT_MAX-4 );

    InitModGraphCtx ( 1000, DataBuffer, bCnt, SymbolBuffer, SAMPLE_BUFFER_SIZE);

    // append CRC32 to the data stream
    *(PULONG)(DataBuffer + bCnt ) = BB11bModCtx.CF_11bTxVector::crc32 ();


    ULONGLONG ts1 = SoraGetCPUTimestamp ( &tsinfo );
    
    for (int i=0; i<RUN_TIMES; i++ ) {
        ssrc->Reset ();	
        if ( BB11bModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
            FB_11B_ModulateBuffer (ssrc);
        } else {
            printf ( "Reset Error!\n" );
        }
    }
    
    ULONGLONG ts2 = SoraGetCPUTimestamp ( &tsinfo );

    printf("Signal data rate:    %d kbps\n", BB11bModCtx.CF_11bTxVector::data_rate_kbps() ); 
    printf("Signal packet size:  %d\n",  BB11bModCtx.CF_11bTxVector::frame_length() );
    printf("Signal sample count: %d\n",  BB11bModCtx.CF_TxSampleBuffer::tx_sample_cnt() );
    printf("Signal encoded size: %d\n",  BB11bModCtx.CF_TxSampleBuffer::tx_sample_cnt() * sizeof(COMPLEX8) );
    printf("Time cost average:   %.3fus \n", SoraTimeElapsed (ts2-ts1, &tsinfo) * 1.0 / 1000 / RUN_TIMES );

    FILE* pOut = NULL;
#pragma warning (push)
#pragma warning (disable:4996)
    pOut = fopen("d:\\test.tx", "wb");
#pragma warning (pop)
    if (!pOut)
    {
        printf("Cannot create output file.\n");
        return -1;
    }

    fwrite(SymbolBuffer, BB11bModCtx.CF_TxSampleBuffer::tx_sample_cnt(), sizeof(COMPLEX8), pOut);
    fclose(pOut);

    IReferenceCounting::Release (ssrc);
    return 0;
}

int Test11A_FB_Mod ()
{
    printf ( "Modulate 11a with Fine Brick!\n" );

    ISource * tssrc = CreatePreamble11a_40M ();
    ISource * ssrc  = CreateModGraph11a_40M ();

    COMPLEX8* pSymBuf = SymbolBuffer;

    BB11aModCtx.set_mod_buffer (pSymBuf, SAMPLE_BUFFER_SIZE );
    tssrc->Reset ();
    if ( BB11aModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
        tssrc->Process ();
    }	
    uint TS_len = BB11aModCtx.CF_TxSampleBuffer::tx_sample_cnt();

    
    pSymBuf += TS_len;
    
    int bCnt = LoadDumpFile ("d:\\test.txt", DataBuffer, INPUT_MAX-4 );
    BB11aModCtx.init ( 24000, DataBuffer, (ushort) bCnt, pSymBuf, SAMPLE_BUFFER_SIZE - TS_len);
    
    // append CRC32 to the data stream
    *(PULONG)(DataBuffer + bCnt ) = BB11aModCtx.CF_11aTxVector::crc32 ();
    
    ULONGLONG ts1 = SoraGetCPUTimestamp ( &tsinfo );
    
    for (int i=0; i<RUN_TIMES; i++ ) {
        ssrc->Reset ();
        if ( BB11aModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
            ssrc->Process ();
            ssrc->Flush ();
        } else {
            printf ( "Reset Error!\n" );
        }
    }
    
    ULONGLONG ts2 = SoraGetCPUTimestamp ( &tsinfo );

    uint sample_size = (TS_len + BB11aModCtx.CF_TxSampleBuffer::tx_sample_cnt());
    
    // debug output
    printf("ts len %d\n", TS_len );
    printf("CRC32 %08x\n", BB11aModCtx.CF_11aTxVector::crc32 ());
    
    // statistics
    printf("Signal data rate:	 %d kbps\n", BB11aModCtx.CF_11aTxVector::data_rate_kbps() ); 
    printf("Signal packet size:  %d\n",  	 BB11aModCtx.CF_11aTxVector::frame_length() );
    printf("Signal sample count: %d\n",  	 sample_size );
    printf("Signal encoded size: %d\n",  	 sample_size * sizeof(COMPLEX8) );
    printf("Time cost average:   %.3fus \n", SoraTimeElapsed (ts2-ts1, &tsinfo) * 1.0 / 1000 / RUN_TIMES );
    
    FILE* pOut = NULL;
#pragma warning (push)
#pragma warning (disable:4996)
    pOut = fopen("d:\\test-11a.tx", "wb");
#pragma warning (pop)
    if (!pOut)
    {
        printf("Cannot create output file.\n");
        return -1;
    }
    
    fwrite(SymbolBuffer, sample_size + 32, sizeof(COMPLEX8), pOut);
    fclose(pOut);
    
    IReferenceCounting::Release (ssrc);
    IReferenceCounting::Release (tssrc);
    return 0;
}
