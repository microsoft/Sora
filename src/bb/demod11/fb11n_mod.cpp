#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <string>

#include "const.h"
#include "dspcomm.h"
#include "soratime.h"

#include "brick.h"
#include "stdbrick.hpp"
#include "CRC32.h"

#include "scramble.hpp"
#include "samples.hpp"

#include "phy_11n.hpp"
#include "fb11nmod_config.hpp"
#include "soratime.h"
#include "bb_test.h"
using namespace std;

extern TIMESTAMPINFO tsinfo;

const int SAMPLE_BUFFER_SIZE = 2*1024*1024;
static UCHAR                DataBuffer[INPUT_MAX];
static A16 COMPLEX16 *      SampleBuffer[2];

int Test11N_FB_Mod (PDOT11_MOD_ARGS pArgs)
{
    printf ( "Modulate 11n with Fine Brick!\n" );

    int bCnt = LoadDumpFile (pArgs->pcInFileName, DataBuffer, INPUT_MAX-4 );
    SampleBuffer[0] = (COMPLEX16 *)aligned_malloc(sizeof(COMPLEX16) * SAMPLE_BUFFER_SIZE, 16);
    SampleBuffer[1] = (COMPLEX16 *)aligned_malloc(sizeof(COMPLEX16) * SAMPLE_BUFFER_SIZE, 16);

    ISource *lsrc, *htsrc;
    CreatePreambleGraph11n(lsrc, htsrc);
    ISource *sigsrc = CreateSigGraph11n();
    ISource *ssrc = CreateModGraph11n();

    // L_STF + L_LTF
    ULONGLONG ts1 = SoraGetCPUTimestamp ( &tsinfo );
    const int RUN_TIMES = 100;
	for (int i=0; i<RUN_TIMES; i++ ) {
        BB11nModCtx.init ( (ushort)pArgs->nBitRate, DataBuffer, (ushort) bCnt, SampleBuffer, SAMPLE_BUFFER_SIZE);
        lsrc->Reset ();
        if ( BB11nModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
            lsrc->Process ();
        }	

        // L_SIG + HT_SIG
        sigsrc->Reset ();
        if ( BB11nModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
            sigsrc->Process ();
        }

        // HT_STF + HT_LTF
        htsrc->Reset ();
        if ( BB11nModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
            htsrc->Process ();
        }

        // Data + CRC32
        ssrc->Reset ();
        if ( BB11nModCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
            ssrc->Process ();
            ssrc->Flush ();
        } else {
            printf ( "Reset Error!\n" );
        }
	}
    ULONGLONG ts2 = SoraGetCPUTimestamp ( &tsinfo );

    // debug output
    printf("CRC32 %08x\n", BB11nModCtx.CF_11nTxVector::crc32 ());
    
    // statistics
    printf("MCS index: %d\n", BB11nModCtx.CF_11nTxVector::mcs_index() ); 
    printf("Signal packet size:  %d\n",  	 BB11nModCtx.CF_11nTxVector::frame_length() );
    printf("Signal sample count: %d\n",  	 BB11nModCtx.GetSinkSampleCount() );
    printf("Signal encoded size: %d\n",  	 BB11nModCtx.GetSinkSampleCount() * sizeof(COMPLEX16) );
    printf("Time expected:   %.3fus \n",     BB11nModCtx.GetSinkSampleCount() / 40.0); // 40 MHz sampling rate
    printf("Time cost average:   %.3fus \n", SoraTimeElapsed (ts2-ts1, &tsinfo) * 1.0 / 1000 / RUN_TIMES );
    
    FILE* pOut;
    string filename;

    // Output the radio 0 tx file
    filename = pArgs->pcOutFileName + string("_0.dmp");
#pragma warning (push)
#pragma warning (disable:4996)
    pOut = fopen(filename.c_str(), "wb");
#pragma warning (pop)
    if (!pOut)
    {
        printf("Cannot create output file: %s.\n", filename.c_str());
        return -1;
    }
    fwrite(SampleBuffer[0], BB11nModCtx.GetSinkSampleCount() + 32, sizeof(COMPLEX16), pOut);
    fclose(pOut);

    // Output the radio 1 tx file
    filename = pArgs->pcOutFileName + string("_1.dmp");
#pragma warning (push)
#pragma warning (disable:4996)
    pOut = fopen(filename.c_str(), "wb");
#pragma warning (pop)
    if (!pOut)
    {
        printf("Cannot create output file: %s.\n", filename.c_str());
        return -1;
    }
    fwrite(SampleBuffer[1], BB11nModCtx.GetSinkSampleCount() + 32, sizeof(COMPLEX16), pOut);
    fclose(pOut);
    
    IReferenceCounting::Release (ssrc);
    IReferenceCounting::Release (sigsrc);
    IReferenceCounting::Release (lsrc);
    IReferenceCounting::Release (htsrc);
    return 0;
}
