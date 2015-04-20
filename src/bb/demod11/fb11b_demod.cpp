// Copyright (c) Microsoft Corporation
//
// Abstract: Offline demodulation test for 802.11b (brick implementation, input data is from dump file

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>
#include "bb/bbb.h"
#include "bb_test.h"
#include "MACStopwatch.h"
#include "stdbrick.hpp"
#include "fb11bdemod_config.hpp"
#include "thread_func.h"
#include "brickutil.h"

#define _M(N) (1024 * 1024 * (N))
#define _K(N) (1024 * (N))

#define INPUTBUF_SIZE (_M(16))
#define OUTPUTBUF_SIZE (_K(4))

static A16 UCHAR                        InputBuf[INPUTBUF_SIZE];
static UCHAR                            OutputBuf[OUTPUTBUF_SIZE];

BOOLEAN MAC11b_Receive (void*) {
    bool rc;
	while (1) {
		rc = pRxSource->Process ();
		ulong& err = BB11bDemodCtx.CF_Error::error_code();

		if ( err != BK_ERROR_SUCCESS ) {
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
                printf("err = %08X\n", err);
			}

            if (err == E_ERROR_FRAME_OK || err == E_ERROR_CRC32_FAIL)
            {
                // TRICK: jump advance of the last CRC byte, because we used spcialized CRC module
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

            pRxSource->Flush ();
			BB11bDemodCtx.reset ();
			pRxSource->Reset ();

			break;
		}

        if (!rc) {
            BB11bDemodCtx.stopwatch.OutputStats();
            return false;
        }
	}	
	return true;
}


int Test11B_FB_Demod(PDOT11_DEMOD_ARGS pArgs)
{
    int rc = 0;
    int nCnt = LoadSoraDumpFile(pArgs->pcInFileName, (COMPLEX16*)InputBuf, INPUTBUF_SIZE);
    if (nCnt < 0)
    {
        puts("Failed to load input file."); 
        return -1;
    }

    printf ( "Demodulate 11b with brick demod graph!\n" );
	BB11bDemodCtx.init ( (COMPLEX16*)InputBuf, nCnt*sizeof(COMPLEX16), OutputBuf, OUTPUTBUF_SIZE );
	pRxSource = CreateDemodGraph ();
    if (BB11bDemodCtx.CF_Error::error_code() != E_ERROR_SUCCESS)
    {
        printf("Failed to create demod graph.\n");
        rc = -1; goto out;
    }

	pRxSource->Reset ();
    if (BB11bDemodCtx.CF_Error::error_code() != E_ERROR_SUCCESS)
    {
        printf("Failed to reset demod graph.\n");
        rc = -1; goto out;
    }

    BB11bDemodCtx.InitStopwatch();
	while(MAC11b_Receive(NULL))
        ;

out:
	IReferenceCounting::Release (pRxSource);
	return rc;
}
