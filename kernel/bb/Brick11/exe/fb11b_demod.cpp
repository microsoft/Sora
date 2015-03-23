// Copyright (c) Microsoft Corporation
//
// Abstract: Offline demodulation test for 802.11b (brick implementation, input data is from dump file

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>

#include <math.h>
#include <thread_func.h>

#include "bb/bbb.h"
#include "dspcomm.h"
#include "soratime.h"
#include "brickutil.h"
#include "stdbrick.hpp"

#include "cca.hpp"
#include "barkerspread.hpp"
#include "scramble.hpp"

#include "symtiming.hpp"
#include "sfd_sync.hpp"
#include "phy_11b.hpp"

#include "phy_11a.hpp"
#include "samples.hpp"
#include "freqoffset.hpp"
#include "channel_11a.hpp"
#include "demapper11a.hpp"
#include "pilot.hpp"
#include "deinterleaver.hpp"
#include "viterbi.hpp"
#include "fft.hpp"

#include "fb11bdemod_config.hpp"
#include "fb11ademod_config.hpp"

#define IN_BUF_SIZE (16*1024*1024)
#define OUT_BUF_SIZE (16*1024*1024)

static A16 UCHAR InputBuf [IN_BUF_SIZE ];
static UCHAR     OutputBuf[OUT_BUF_SIZE];

extern TIMESTAMPINFO tsinfo;

void FB_11B_DemodBuffer (ISource* ssrc) {
	while (1) {
		bool bRet = ssrc->Process ();
		if ( !bRet ) return;
		
		ulong& err = BB11bDemodCtx.CF_Error::error_code();
		if ( err != E_ERROR_SUCCESS ) {
			if ( err == E_ERROR_CS_TIMEOUT ) 
				continue;
		}

//		if (BB11bDemodCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
//			ssrc->Flush ();
//		} 
	}	
}

int Test11B_FB_Demod()
{
	printf ( "Demodulate 11b with Fine Brick!\n" );


	ISource * ssrc = CreateDemodGraph ();

//	int bCnt = LoadSoraDumpFile ( "c:\\11b-1M-1.dmp", (COMPLEX16*)InputBuf, IN_BUF_SIZE );
//	int bCnt = LoadSoraDumpFile ( "d:\\noise.dmp", (COMPLEX16*)InputBuf, IN_BUF_SIZE );
	int bCnt = LoadSoraDumpFile ( "d:\\test.dmp", (COMPLEX16*)InputBuf, IN_BUF_SIZE );


	printf ( "Load sample %d\n", bCnt );



	ULONGLONG ts1 = SoraGetCPUTimestamp ( &tsinfo );
	for ( int i=0; i<100; i++) {
		
		InitFBDemodContext ((COMPLEX16*)InputBuf, (bCnt)*sizeof(COMPLEX16), OutputBuf, OUT_BUF_SIZE);

		ssrc->Reset ();
		if ( BB11bDemodCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
			FB_11B_DemodBuffer (ssrc);
		} else {
			printf ( "Reset error!\n" );
		}
	}	
	ULONGLONG ts2 = SoraGetCPUTimestamp ( &tsinfo );
	
	ulong& err = BB11bDemodCtx.CF_Error::error_code();
	if (  err == E_ERROR_FRAME_OK ) {	
		printf ( "one good frame find!\n" );
	} else {
		printf ( "error code: %08x \n", err );
	}


	printf("Signal data rate:	 %dk\n",BB11bDemodCtx.CF_11bRxVector::data_rate_kbps() ); 
	printf("Bytes decoded: %d\n",   	BB11bDemodCtx.CF_11bRxVector::frame_length() );
    printf("Time cost average:   %.3fus \n", SoraTimeElapsed (ts2-ts1, &tsinfo) * 1.0 / 1000 / 100 );

/*
	if ( pArgs->pcOutFileName != NULL ) {
		FILE* pOut = NULL;
#pragma warning (push)
#pragma warning (disable:4996)
		pOut = fopen(pArgs->pcOutFileName, "w+b");
#pragma warning (pop)
		if (!pOut)
		{
			printf("Cannot create output file.\n");
			return -1;
		}

		fwrite(	OutputBuf, 
				fb11bDemodCtx.CF_MPDU_Info::frame_len(), 
				1, pOut);
		fclose(pOut);
	}
*/
	IReferenceCounting::Release (ssrc);

	return 0;
}

void FB_11A_DemodBuffer (ISource* ssrc) {
	while (1) {
		bool bRet = ssrc->Process ();
		
		ulong& err = BB11aDemodCtx.CF_Error::error_code();
		if ( err != E_ERROR_SUCCESS ) {
			if ( err == E_ERROR_CS_TIMEOUT ) {
				printf ( "cs timeout \n" );
				
				ssrc->Reset ();
				BB11aDemodCtx.Reset ();
				if ( err != E_ERROR_SUCCESS ) break;
				
				continue;
			}	
			else break;
		}

		if (!bRet ) break;

//		if (BB11bDemodCtx.CF_Error::error_code() == E_ERROR_SUCCESS ) {
//			ssrc->Flush ();
//		} 
	}	
}


HANDLE thread1, thread2; // thread2 for viterbi; thread1 for others

bool fRunning;
ISource * ssrc, * svit;
ULONGLONG tts1, tts2; 

const int TEST_NUM = 2;
int test_count = TEST_NUM;
int bCnt = 0;	

BOOLEAN RxThread ( void * pParam ) {
	ulong& err = BB11aDemodCtx.CF_Error::error_code();
	
	while (test_count) {
		BB11aDemodCtx.Reset ();
		ssrc->Reset ();
		if ( err != E_ERROR_SUCCESS ) {
			printf ( "Reset error (%X)!\n", err );
		} 

		FB_11A_DemodBuffer(ssrc);
		
		if (  err == E_ERROR_FRAME_OK ) {	
	//    	printf("Time cost average:   %.3fus \n", SoraTimeElapsed (tts2-tts1, &tsinfo) * 1.0 / 1000 );		
	//		printf ( "one good frame\n" );
			if ( test_count == TEST_NUM ) {
				printf ( "one good frame - data rate %d frame length %d\n", 
                    BB11aDemodCtx.CF_11aRxVector::data_rate_kbps(),
                    BB11aDemodCtx.CF_11aRxVector::frame_length() 
                );
				tts1 = SoraGetCPUTimestamp ( &tsinfo );
			}
		} else {	
			printf ( "error code: %08x \n", err );
		}

		test_count --;
		if ( test_count == 0 ) {
			tts2 = SoraGetCPUTimestamp ( &tsinfo );
			fRunning = false;
	//		return false;
		} else {
			BB11aDemodCtx.Init ( (COMPLEX16*)InputBuf, (bCnt)*sizeof(COMPLEX16), OutputBuf, OUT_BUF_SIZE);
	//		return true;
		}
	}
	return false;
}

BOOLEAN ViterbiThread ( void * pParam ) {
	while ( fRunning ) {
		svit->Process ();

	}	
	
//	if ( fRunning ) return TRUE;
//	else return FALSE;
	return false;
}


int Test11A_FB_Demod()
{
	printf ( "Demodulate 11a with Fine Brick!\n" );

	// test usin/ucos

//	int bCnt = LoadSoraDumpFile ( "c:\\11b-1M-1.dmp", (COMPLEX16*)InputBuf, IN_BUF_SIZE );
//	int bCnt = LoadSoraDumpFile ( "d:\\noise.dmp", (COMPLEX16*)InputBuf, IN_BUF_SIZE );
//	int bCnt = LoadSoraDumpFile ( "d:\\test-real.dmp", (COMPLEX16*)InputBuf, IN_BUF_SIZE );
//	int bCnt = LoadSoraDumpFile ( "d:\\test-ideal.dmp", (COMPLEX16*)InputBuf, IN_BUF_SIZE );
	bCnt = LoadSoraDumpFile ( "c:\\a48-40.dmp", (COMPLEX16*)InputBuf, IN_BUF_SIZE );



	printf ( "Load sample %d\n", bCnt );


	BB11aDemodCtx.Init ( (COMPLEX16*)InputBuf, (bCnt)*sizeof(COMPLEX16), OutputBuf, OUT_BUF_SIZE);
	
	CreateDemodGraph11a (ssrc, svit);
	fRunning = true;
	ULONGLONG tts1 = SoraGetCPUTimestamp ( &tsinfo );	
	
	thread2 = AllocStartThread ( ViterbiThread, NULL );
	thread1 = AllocStartThread ( RxThread, NULL );

	while ( fRunning ) {
		Sleep (1);
	}

	StopFreeThread (thread1 );
	StopFreeThread (thread2 );
	
//	printf("Signal data rate:	 %dk\n",BB11bDemodCtx.CF_11bRxVector::data_rate_kbps() ); 
//	printf("Bytes decoded: %d\n",   	BB11bDemodCtx.CF_11bRxVector::frame_length() );
    printf("Time cost average:   %.3fus \n", SoraTimeElapsed (tts2-tts1, &tsinfo) * 1.0 / 1000 / (TEST_NUM-1));
	printf ( "Total symbol %d!\n", BB11aDemodCtx.CF_11aRxVector::total_symbols() );


/*
	if ( pArgs->pcOutFileName != NULL ) {
		FILE* pOut = NULL;
#pragma warning (push)
#pragma warning (disable:4996)
		pOut = fopen(pArgs->pcOutFileName, "w+b");
#pragma warning (pop)
		if (!pOut)
		{
			printf("Cannot create output file.\n");
			return -1;
		}

		fwrite(	OutputBuf, 
				fb11bDemodCtx.CF_MPDU_Info::frame_len(), 
				1, pOut);
		fclose(pOut);
	}
*/

	IReferenceCounting::Release (ssrc);

	return 0;
}

