#include <stdio.h>
#include <assert.h>
#include <windows.h>

#include "const.h"
#include "dspcomm.h"
#include "soratime.h"

#include "brick.h"
#include "stdbrick.hpp"
#include "CRC32.h"

#include "scramble.hpp"
#include "samples.hpp"

#include "phy_11a.hpp"
#include "fb11amod_config.hpp"
#include "soratime.h"
#include "bb_test.h"

extern TIMESTAMPINFO tsinfo;

const int SAMPLE_BUFFER_SIZE = 2*1024*1024;
static UCHAR               DataBuffer[INPUT_MAX];
static A16 COMPLEX8        SymbolBuffer[SAMPLE_BUFFER_SIZE];

int Test11A_FB_Mod (PDOT11_MOD_ARGS pArgs)
{
	printf ( "Modulate 11a with Fine Brick!\n" );

	ISource * tssrc;
	ISource * ssrc;
    if (pArgs->SampleRate == 40) {
	    ssrc = CreateModGraph11a_40M ();
        tssrc = CreatePreamble11a_40M ();
    }
    else {
        assert(pArgs->SampleRate == 44);
	    ssrc = CreateModGraph11a_44M ();
        tssrc = CreatePreamble11a_44M ();
    }


	COMPLEX8* pSymBuf = SymbolBuffer;

	BB11aModCtx.set_mod_buffer (pSymBuf, SAMPLE_BUFFER_SIZE );
	tssrc->Reset ();
	if ( BB11aModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
		tssrc->Process ();
	}	
	uint TS_len = BB11aModCtx.CF_TxSampleBuffer::tx_sample_cnt();

	
	pSymBuf += TS_len;
	
	int bCnt = LoadDumpFile (pArgs->pcInFileName, DataBuffer, INPUT_MAX-4 );
    BB11aModCtx.init ( pArgs->nBitRate, DataBuffer, (ushort) bCnt, pSymBuf, SAMPLE_BUFFER_SIZE - TS_len);
	
	ULONGLONG ts1 = SoraGetCPUTimestamp ( &tsinfo );
	
    const int RUN_TIMES = 100;
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
    pOut = fopen(pArgs->pcOutFileName, "wb");
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
