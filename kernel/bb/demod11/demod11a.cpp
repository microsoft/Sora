#include <stdio.h>
#include <math.h>
#include "func.h"
#include "bb/bba.h"
#include "soradsp.h"
#include "thread_func.h"
#include "bb_test.h"

#define MAX_BLOCKS  ((16 * 1024 * 1024) / 128)
#define _16M_BYTES (16 * 1024 * 1024)

static FLAG fWork = TRUE;
static MEM_ALIGN(16) UCHAR RxBuffer[_16M_BYTES];
static SORA_RADIO_RX_STREAM RxStream;
static BB11A_RX_CONTEXT RxContext;

static UCHAR FrameBuffer[1024 * 1024];

BOOL LoadSignal(const char* pcFile)
{
    FILE *hSigFile;
    ULONG nRead;
#pragma warning (push)
#pragma warning (disable:4996)
    hSigFile = fopen(pcFile, "rb");
#pragma warning (pop)

    if (!hSigFile)
    {
        printf("Cannot open signal file\n");
        return FALSE;
    }

    nRead = fread(RxBuffer, sizeof(UCHAR), _16M_BYTES, hSigFile);
    return TRUE;
}

BOOL PrepareSigStream(const char* pcFile)
{
    BOOL succ = LoadSignal(pcFile);
    if (succ)
        SoraGenRadioRxStreamOffline(&RxStream, RxBuffer, 16 * 1024 * 1024);
    return succ;
}

#define MIN_CS_SCAN_BLOCKS      MAX_BLOCKS
void PrepareRxContext(PDOT11_DEMOD_ARGS pArgs)
{
    BB11ARxContextInit(
        &RxContext, pArgs->SampleRate, 250000, 150, 112, (PFLAG)&fWork);
}

void CsFrameDemod(PDOT11_DEMOD_ARGS pArgs)
{
    HRESULT     hr;
    FLAG        finished            = FALSE;
    int         iNoiseCounter       = 0;
    int         iFrameCounter       = 0;
    int         iGoodFrameCounter   = 0;
    size_t      iSPTemp;
    PRX_BLOCK   pbSPTemp;
    TIMINGINFO  ti;
    double dReq, dCost, dRatio;

    double dNoiseAverage = 0, dFrameAverage = 0;
    double dNoiseMax = 0, dFrameMax = 0;
    double dNosieSD = 0, dFrameSD = 0;
    int dNoiseAboveCounter = 0, dFrameAboveCounter = 0;
    int dNoiseAbove0_8Counter = 0, dFrameAbove0_8Counter = 0;

    if(!PrepareSigStream(pArgs->pcInFileName)) return;

    FILE *pcOutFile = fopen_try(pArgs->pcOutFileName, "wb");

    PrepareRxContext(pArgs);

    HANDLE hViterbiThread = AllocStartThread(BB11ARxViterbiWorker, &RxContext);
	if (hViterbiThread == NULL) goto out;
	printf("Successfully start viterbi thread\n");

    do
    {
        pbSPTemp = SoraRadioGetRxStreamPos(&RxStream);

        TimerStart(&ti);
        hr = BB11ARxCarrierSense(&RxContext, &RxStream);
        TimerStop(&ti);

        // Possible the scan pointer wraps to the buffer header
        dReq = buffer_span(pbSPTemp, SoraRadioGetRxStreamPos(&RxStream), SoraRadioGetRxStreamSize(&RxStream)) * 28.0 / 40;
        dCost = TimerRead(&ti) * 1000;
        dRatio = dCost / dReq;

        printf("[noise]  req: %9.3fus  cost: %9.3fus \n", dReq, dCost);
        iNoiseCounter++;
        dNoiseAverage += dRatio;
        dNosieSD += dRatio * dRatio;
        dNoiseMax = (dNoiseMax >= dRatio)?dNoiseMax : dRatio;
        if (dRatio >= 0.8)
        {
            dNoiseAbove0_8Counter++;
            if (dRatio >= 1.0)
            {
                dNoiseAboveCounter++;
            }
        }

        switch(hr)
        {
        case BB11A_CHANNEL_CLEAN:
        case BB11A_E_PD_LAG:
            break;
        case BB11A_OK_POWER_DETECTED:
            BB11APrepareRx(&RxContext, (char*)FrameBuffer, 1024 * 1024);

            pbSPTemp = SoraRadioGetRxStreamPos(&RxStream);
            iSPTemp = SoraRadioGetRxStreamIndex(&RxStream);

            TimerStart(&ti);
            hr = BB11ARxFrameDemod(&RxContext, &RxStream);
            TimerStop(&ti);

            // Possible the scan pointer wraps to the buffer header
            dReq = buffer_span(pbSPTemp, SoraRadioGetRxStreamPos(&RxStream), SoraRadioGetRxStreamSize(&RxStream)) * 28.0 / 40;
            dCost = TimerRead(&ti) * 1000;
            dRatio = dCost / dReq;
            
            if (hr == E_FETCH_SIGNAL_HW_TIMEOUT)
            {
                finished = TRUE;
                continue;
            }

            if ( hr != BB11A_E_SYNC_FAIL )
            {
                iFrameCounter++;                
                printf("[frame]  req: %9.3fus  cost: %9.3fus  "
                    "frame#%3d@desc#%d-desc#%d\n", 
                    dReq, 
                    dCost,
                    iFrameCounter,
                    iSPTemp / 28,
                    SoraRadioGetRxStreamIndex(&RxStream) / 28);

                dFrameAverage += dRatio;
                dFrameSD += dRatio * dRatio;
                dFrameMax = (dFrameMax >= dRatio)?dFrameMax : dRatio;
                if (dRatio >= 0.8)
                {
                    dFrameAbove0_8Counter++;
                    if (dRatio >= 1.0)
                    {
                        dFrameAboveCounter++;
                    }
                }
            }

            if (hr == BB11A_OK_FRAME || hr == BB11A_E_CRC32)
            {
                if (pcOutFile) fwrite(RxContext.ri_pbFrame, RxContext.__usLength - 4, 1, pcOutFile);
            }
                      
            if (FAILED(hr))
            {
                printf("         err:   %08xh\n", hr);
            }
            else
            {
                iGoodFrameCounter++;

                printf("         pre: %9.3fus  sig: %9.3fus   data: %9.3fus\n",
                    TimerRead(&RxContext.ri_tOfflineTimings[0]) * 1000,
                    TimerRead(&RxContext.ri_tOfflineTimings[1]) * 1000,
                    TimerRead(&RxContext.ri_tOfflineTimings[4]) * 1000);

                printf("         len: %5d\n", RxContext.ri_uiFrameSize);
            }
            break;
        case E_FETCH_SIGNAL_HW_TIMEOUT:
        case E_INVALIDARG:
            finished = TRUE;
            break;
        }
    } while(!finished);

    printf("[summary] positive: %5d  good: %5d        bad: %5d\n", 
        iFrameCounter, iGoodFrameCounter, iFrameCounter - iGoodFrameCounter);

    dNoiseAverage /= iNoiseCounter;
    dFrameAverage /= iFrameCounter;
    dNosieSD = sqrt(dNosieSD / iNoiseCounter - dNoiseAverage * dNoiseAverage);
    dFrameSD = sqrt(dFrameSD / iFrameCounter - dFrameAverage * dFrameAverage);

    printf("\nDetail statistics for noise:\n"
        "total: \t\t%d\n"
        "average: \t%.2f\n"
        "max: \t\t%.2f\n"
        "std: \t\t%.2f\n"
        "#(cost >= req):     %d (%3.2f%%)\n"
        "#(cost >= req*0.8): %d (%3.2f%%)\n",
        iNoiseCounter,
        dNoiseAverage,
        dNoiseMax,
        dNosieSD,
        dNoiseAboveCounter, dNoiseAboveCounter / (double)iNoiseCounter * 100,
        dNoiseAbove0_8Counter, dNoiseAbove0_8Counter / (double)iNoiseCounter * 100);

    printf("\nDetail statistics for frame:\n"
        "total: \t\t%d\n"
        "average: \t%.2f\n"
        "max: \t\t%.2f\n"
        "std: \t\t%.2f\n"
        "#(cost >= req):     %d (%3.2f%%)\n"
        "#(cost >= req*0.8): %d (%3.2f%%)\n",
        iFrameCounter,
        dFrameAverage,
        dFrameMax,
        dFrameSD,
        dFrameAboveCounter, dFrameAboveCounter / (double)iFrameCounter * 100,
        dFrameAbove0_8Counter, dFrameAbove0_8Counter / (double)iFrameCounter * 100);

out:
    fclose_try(pcOutFile);

    fWork = false;
    StopFreeThread(hViterbiThread);
    BB11ARxContextCleanup(&RxContext);
}
