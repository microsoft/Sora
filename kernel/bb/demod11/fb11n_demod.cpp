// Copyright (c) Microsoft Corporation
//
// Abstract: Offline demodulation test for 802.11b (brick implementation, input data is from dump file

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>
#include <string>
#include "bb_test.h"
#include "MACStopwatch.h"
#include "stdbrick.hpp"
#include "fb11ndemod_config.hpp"
#include "thread_func.h"
#include "brickutil.h"

#define _M(N) (1024 * 1024 * (N))
#define _K(N) (1024 * (N))

#define INPUTBUF_SIZE (_M(16))
#define OUTPUTBUF_SIZE (_K(4))

static A16 COMPLEX16 *                  InputBuf[2];
static UCHAR                            OutputBuf[OUTPUTBUF_SIZE];

static HANDLE thread1, thread2; // thread2 for viterbi; thread1 for others
static ISource * ssrc, * svit;
static IControlPoint * scs;

static BOOLEAN RxThread ( void * pParam ) {
    const int           nDIFS = 12;
	uint nWaitCounter;
	nWaitCounter = nDIFS;

    bool bRet;
	while (1) {
    	bRet = ssrc->Process ();
		ulong err = BB11nDemodCtx.CF_Error::error_code();
	
		if ( err != E_ERROR_SUCCESS ) {
			// Any event happened
            if ( err == E_ERROR_FRAME_OK ) {
				// One frame is received
                BB11nDemodCtx.MoveState_RX_CS(true, true);
                //printf("rx aff = %16x, vit aff = %16x\n", GetThreadAffinityMask(GetCurrentThread()), affVit);
            } else if ( err == E_ERROR_CRC32_FAIL) {
                BB11nDemodCtx.MoveState_RX_CS(true, false);
			} else if ( err == E_ERROR_CS_TIMEOUT ) {
				// Channel clean
                BB11nDemodCtx.ResetCarrierSense();
    		    scs->Reset ();

				if ( nWaitCounter > 0) {
					nWaitCounter --; // just backoff
					continue;
				} else {
                    break;
				}

	        } else {
                BB11nDemodCtx.MoveState_RX_CS(false, false);
                printf("err = %08X\n", err);
            }

			//
			// Reset context
			BB11nDemodCtx.Reset ();
			ssrc->Reset ();
		}

        if (!bRet)
        {
            BB11nDemodCtx.stopwatch.OutputStats();
            return FALSE;
        }
	}

    return TRUE;
}

static BOOLEAN ViterbiThread ( void * pParam ) {
	svit->Process ();
    return TRUE;
}

int Test11N_FB_Demod(PDOT11_DEMOD_ARGS pArgs)
{
    InputBuf[0] = (COMPLEX16 *)aligned_malloc(INPUTBUF_SIZE, 16);
    InputBuf[1] = (COMPLEX16 *)aligned_malloc(INPUTBUF_SIZE, 16);

    int nCnt = LoadSoraDumpFile(std::string(pArgs->pcInFileName).append("_0.dmp").c_str(), (COMPLEX16*)InputBuf[0], INPUTBUF_SIZE);
    if (nCnt < 0)
    {
        puts("Failed to load input file."); 
        return -1;
    }

    nCnt = LoadSoraDumpFile(std::string(pArgs->pcInFileName).append("_1.dmp").c_str(), (COMPLEX16*)InputBuf[1], INPUTBUF_SIZE);
    if (nCnt < 0)
    {
        puts("Failed to load input file."); 
        return -1;
    }

    printf ( "Demodulate 11n with brick demod graph!\n" );

	BB11nDemodCtx.Init (OutputBuf, OUTPUTBUF_SIZE);
    CreateDemodGraph11n (ssrc, svit, scs);

    // Init bricks
    IQuery *q;
    size_t found = ssrc->TraverseGraph(&q, "TMemSamples2", 1);
    assert(found);
    MemSamplesDesc *memsamples = q->QueryInterface<MemSamplesDesc>();
    memsamples->Init(2, InputBuf, nCnt);

	thread2 = AllocStartThread ( ViterbiThread, NULL );
    ssrc->Reset();
    BB11nDemodCtx.InitStopwatch();
	thread1 = AllocStartThread ( RxThread, NULL );

    SoraThreadJoin (thread1, INFINITE);
	StopFreeThread (thread1 );
	StopFreeThread (thread2 );

	IReferenceCounting::Release (ssrc);
    aligned_free(InputBuf[1]);
    aligned_free(InputBuf[0]);
	return 0;
}
