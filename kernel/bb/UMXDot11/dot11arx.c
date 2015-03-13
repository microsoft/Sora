#include <time.h>
#include <conio.h>
#include "bb/bba.h"
#include "thread_if.h"
#include "monitor.h"
#include "crc32.h"
#include "appext.h"
#include "thread_func.h"

#define INPUTBUF_SIZE		(_M(16))
#define FrameBufferSize		(_M(1))

static SORA_RADIO_RX_STREAM	RxStream;
static BB11A_RX_CONTEXT		RxContext;
static UCHAR				FrameBuffer[FrameBufferSize];

static PVOID				RxBuffer = NULL;
static ULONG				RxBufferSize = 0;
static FLAG					fWork = TRUE;

// Statistic monitor for frames
static Monitor monitor;

BOOLEAN DoDot11ARx(void*)
{
	HRESULT hr;
	
	while (1) {
        monitor.Query(false);

        //printf("Carrier sense begin.\n");
		hr = BB11ARxCarrierSense(&RxContext, &RxStream);
		//printf("Carrier sense ret : %08x\n", hr);

        if (hr == E_INVALIDARG)
        {
            return FALSE;
        }

        if (hr == BB11A_CHANNEL_CLEAN)
		{
            //printf("Channel Clean.\n");
			return TRUE;
        }
		
		if (hr == E_FETCH_SIGNAL_HW_TIMEOUT)
		{
			printf("Fetch Signal Hw Timeout.\n");
			return TRUE;
		}

		if (hr == BB11A_E_PD_LAG)
		{			
            printf("E_PD_LAG.\n");
            return TRUE;
        }
		if (hr == BB11A_OK_POWER_DETECTED)
		{			
            //printf("Power Detected.\n");
			break;
        }
	}

	do
	{		
        //printf("Rx Begin.\n");
		hr = BB11ARxFrameDemod(&RxContext, &RxStream);
        //printf("Rx ret : %08x\n", hr);

        if (hr == E_INVALIDARG)
        {
            return FALSE;
        }

		if (hr == E_FETCH_SIGNAL_HW_TIMEOUT)
		{
			printf("No more singals.\n");
		}
		else if (hr == E_FETCH_SIGNAL_FORCE_STOPPED)
		{
			printf("Force stopped.\n");
		}
        else if (hr == BB11A_OK_FRAME)
		{
            monitor.IncGoodCounter();
            monitor.IncThroughput(RxContext.ri_uiFrameSize - 4); // Accumulate viterbi thread output length, excluding CRC32
        }
		else if (hr == BB11A_E_CRC32)
        {
            monitor.IncBadCounter();
        }
	} while(FALSE);

	return TRUE;
}

void PrepareRxContext(const Config& config)
{
	BB11ARxContextInit(
		&RxContext,
        config.GetSampleRate(),
        250000,                                    // rxThreshold
        INPUTBUF_SIZE / sizeof(RX_BLOCK),          // rxMaxBlockCount
        112,                                       // rxMinBlockCount
        (PFLAG)&fWork);
}

void Dot11ARxApp(const Config& config)
{
	HRESULT		hr;

    hr = SoraURadioMapRxSampleBuf(TARGET_RADIO, &RxBuffer, &RxBufferSize);
    printf("map rx buffer ret: %08x\n", hr);
	printf("rx buffer: %08x, size: %08x\n", RxBuffer, RxBufferSize);
	if (FAILED(hr)) goto out;
    printf("%08x, %08x, %08x, %08x\n", 
            ((PULONG)RxBuffer)[0], 
            ((PULONG)RxBuffer)[1], 
            ((PULONG)RxBuffer)[2], 
            ((PULONG)RxBuffer)[3]);	

	hr = SoraURadioAllocRxStream(&RxStream, TARGET_RADIO, (PUCHAR)RxBuffer, RxBufferSize);
    printf("SoraAllocRadioRxStream ret: %08x\n", hr);
    if (FAILED(hr)) goto out;

	PrepareRxContext(config);
    printf("Rx Context Init ended.\n");

	BB11APrepareRx(&RxContext, (char*)FrameBuffer, FrameBufferSize);

    HANDLE hViterbiThread = AllocStartThread(BB11ARxViterbiWorker, &RxContext);
	if (hViterbiThread == NULL) goto out;
	printf("Successfully start viterbi thread\n");

    HANDLE hRxThread = AllocStartThread(DoDot11ARx);
	if (hRxThread == NULL) goto out;
	printf("StartRxThread successfully\n");
	
    printf("\n\nPress any key to exit the program\n");			
    time_t start = time(NULL);
	while(!_kbhit())
    {
        if (config.Interval() != 0 && difftime(time(NULL), start) >= config.Interval()) break;
	}

out:
    StopFreeThread(hViterbiThread);
    StopFreeThread(hRxThread);
	
    BB11ARxContextCleanup(&RxContext);
    SoraURadioReleaseRxStream(&RxStream, TARGET_RADIO);

	if (RxBuffer) {
		hr = SoraURadioUnmapRxSampleBuf(TARGET_RADIO, RxBuffer);
		printf("Unmap rx buffer ret: %08x\n", hr);
	}
	printf("Rx out.\n");
}
