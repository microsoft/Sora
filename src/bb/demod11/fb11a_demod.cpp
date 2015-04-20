// Copyright (c) Microsoft Corporation
//
// Abstract: Offline demodulation test for 802.11b (brick implementation, input data is from dump file

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>
#include "bb_test.h"
#include "MACStopwatch.h"
#include "stdbrick.hpp"
#include "fb11ademod_config.hpp"
#include "thread_func.h"
#include "brickutil.h"

#define _M(N) (1024 * 1024 * (N))
#define _K(N) (1024 * (N))

#define INPUTBUF_SIZE (_M(16))
#define OUTPUTBUF_SIZE (_K(4))

static A16 UCHAR                        InputBuf[INPUTBUF_SIZE];
static UCHAR                            OutputBuf[OUTPUTBUF_SIZE];

HANDLE thread1, thread2; // thread2 for viterbi; thread1 for others
ISource * ssrc, * svit;
IControlPoint * scs;

BOOLEAN RxThread ( void * pParam ) {
    const int           nDIFS = 12;
	uint nWaitCounter;
	nWaitCounter = nDIFS;

    bool bRet;
	while (1) {
    	bRet = ssrc->Process ();
		ulong err = BB11aDemodCtx.CF_Error::error_code();
	
		if ( err != E_ERROR_SUCCESS ) {
			// Any event happened
            if ( err == E_ERROR_FRAME_OK ) {
				// One frame is received
                BB11aDemodCtx.MoveState_RX_CS(true, true);
                //printf("rx aff = %16x, vit aff = %16x\n", GetThreadAffinityMask(GetCurrentThread()), affVit);
            } else if ( err == E_ERROR_CRC32_FAIL) {
                BB11aDemodCtx.MoveState_RX_CS(true, false);
			} else if ( err == E_ERROR_CS_TIMEOUT ) {
				// Channel clean
                BB11aDemodCtx.ResetCarrierSense();
    		    scs->Reset ();

				if ( nWaitCounter > 0) {
					nWaitCounter --; // just backoff
					continue;
				} else {
                    break;
				}

	        } else {
                BB11aDemodCtx.MoveState_RX_CS(false, false);
                printf("err = %08X\n", err);
            }

			//
			// Reset context
			BB11aDemodCtx.Reset ();
			ssrc->Reset ();
		}

        if (!bRet)
        {
            BB11aDemodCtx.stopwatch.OutputStats();
            return FALSE;
        }
	}

    return TRUE;
}

BOOLEAN ViterbiThread ( void * pParam ) {
	svit->Process ();
    return TRUE;
}

int Test11A_FB_Demod(PDOT11_DEMOD_ARGS pArgs)
{
    int nCnt = LoadSoraDumpFile(pArgs->pcInFileName, (COMPLEX16*)InputBuf, INPUTBUF_SIZE);
    if (nCnt < 0)
    {
        puts("Failed to load input file."); 
        return -1;
    }

    printf ( "Demodulate 11a with brick demod graph!\n" );

	BB11aDemodCtx.Init ( (COMPLEX16*)InputBuf, (nCnt)*sizeof(COMPLEX16), OutputBuf, OUTPUTBUF_SIZE);
	
    if (pArgs->SampleRate == 40)
        CreateDemodGraph11a_40M (ssrc, svit, scs);
    else
    {
        assert(pArgs->SampleRate == 44);
        CreateDemodGraph11a_44M (ssrc, svit, scs);
    }

	thread2 = AllocStartThread ( ViterbiThread, NULL );
    ssrc->Reset();
    BB11aDemodCtx.InitStopwatch();
	thread1 = AllocStartThread ( RxThread, NULL );

    SoraThreadJoin (thread1, INFINITE);
	StopFreeThread (thread1 );
	StopFreeThread (thread2 );

	IReferenceCounting::Release (ssrc);
	return 0;
}
