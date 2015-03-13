#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include "const.h"
#include "soratime.h"
#include "stdbrick.hpp"
#include "CRC32.h"
#include "bb/bbb.h"
#include "bb/DataRate.h"
#include "dot11_pkt.h"
#include "atx.h"
#include "bb_test.h"
#include "dspcomm.h"

#include "brick.h"
#include "dot11_plcp.h"

#include "scramble.hpp"
#include "pulse.hpp"
#include "phy_11b.hpp"
#include "samples.hpp"
#include "barkerspread.hpp"


#include "fb11bmod_config.hpp"

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

int Test11B_FB_Mod (PDOT11_MOD_ARGS pArgs)
{
	printf ( "Modulate 11b with Fine Brick!\n" );
	
	ISource * ssrc = CreateModGraph ();
	
	int bCnt = LoadDumpFile (pArgs->pcInFileName, DataBuffer, INPUT_MAX-4 );

	InitModGraphCtx ( pArgs->nBitRate, DataBuffer, bCnt, SymbolBuffer, SAMPLE_BUFFER_SIZE);
    if (BB11bModCtx.error_code() != E_ERROR_SUCCESS)
    {
		printf ( "data rate is not supported yet!\n" );
    	IReferenceCounting::Release (ssrc);
        return -1;
    }

	// append CRC32 to the data stream
	*(PULONG)(DataBuffer + bCnt ) = BB11bModCtx.CF_11bTxVector::crc32 ();

    const int RUN_TIMES = 100;

	ULONGLONG ts1 = SoraGetCPUTimestamp ( &tsinfo );
	
	for (int i=0; i < RUN_TIMES; i++ ) {
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
    printf("Time cost average:   %.3f us \n", SoraTimeElapsed (ts2-ts1, &tsinfo) * 1.0 / 1000 / RUN_TIMES);

    FILE* pOut = NULL;
#pragma warning (push)
#pragma warning (disable:4996)
    pOut = fopen(pArgs->pcOutFileName, "wb");
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
