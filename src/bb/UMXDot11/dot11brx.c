#include <time.h>
#include <conio.h>
#include "alinew.h"
#include "bb/bbb.h"
#include "monitor.h"
#include "diagnostics.h"
#include "appext.h"
#include "thread_func.h"

#define INPUTBUF_SIZE		(_M(16))
#define FRAME_BUFFER_SIZE	(_M(1))

static SORA_RADIO_RX_STREAM		RxStream;
static PBB11B_RX_CONTEXT		pRxContext = NULL;
static PBB11B_SPD_CONTEXT		pSpdContext = NULL;
static UCHAR					FrameBuffer[FRAME_BUFFER_SIZE];

static PVOID				RxBuffer = NULL;
static ULONG				RxBufferSize = 0;

// Statistic monitor for frames
static Monitor monitor;

static FLAG					bCanWork = TRUE;

static ULONG				PowerThreshold	= 4000;
static ULONG				PowerThresholdLH= 0xFFFFFFF;
static ULONG				PowerThresholdHL= 4000;
static ULONG				ShiftRight		= 4;

static void Dot11BPrepareRxContext()
{
	pSpdContext->b_resetFlag = 1;

	BB11BRxSpdContextInit(
        pRxContext,
        pSpdContext,
        &bCanWork, 
        0xFFFFFFF,                          // nRxMaxBlockCount
		0xFFFFFFF,							// nSPDMaxBlockCount
        15,                                 // nSPDMinBlockCount
        PowerThreshold,                     // nSPDThreashold
		PowerThresholdLH,
		PowerThresholdHL,
		ShiftRight
        );
	BB11BPrepareRx(pRxContext, FrameBuffer, FRAME_BUFFER_SIZE);

    printf("[dot11b] Rx Spd Context Init ended.\n");
}

static int Dot11BRxInit()
{
	HRESULT hr;

	hr = SoraURadioMapRxSampleBuf(TARGET_RADIO, &RxBuffer, &RxBufferSize);
    printf("Map Buffer ret : %08x\n", hr);
	printf("Rxbuf: %08x, size: %08x\n", RxBuffer, RxBufferSize);

	if (FAILED(hr)) return -1;

	printf("%08x, %08x, %08x, %08x\n", 
            ((PULONG)RxBuffer)[0], 
            ((PULONG)RxBuffer)[1], 
            ((PULONG)RxBuffer)[2], 
            ((PULONG)RxBuffer)[3]);

	hr = SoraURadioAllocRxStream(&RxStream, TARGET_RADIO, (PUCHAR)RxBuffer, RxBufferSize);
    printf("SoraURadioAllocRxStream ret: %08x\n", hr);
    if (FAILED(hr)) return -2;

	Dot11BPrepareRxContext();
    printf("Rx Context Init ended.\n");

	return 0;
}

BOOLEAN DoDot11BRx(void*)
{
	HRESULT hr;
	
	while (1) {
        monitor.Query(false);

        TraceOutput("Carrier sense begin.\n");
		hr = BB11BSpd(pSpdContext, &RxStream);

        if (hr == BB11B_CHANNEL_CLEAN)
		{
            TraceOutput("Carrier sense return: Channel Clean\n");
			return true;
        }
		else if (hr == E_FETCH_SIGNAL_HW_TIMEOUT)
		{
			TraceOutput("Carrier sense return: Fetch Signal Hw Timeout.\n");
			return true;
		}
        else if (hr == BB11B_E_PD_LAG)
		{			
            TraceOutput("Carrier sense return: E_PD_LAG.\n");
            return true;
        }
		else if (hr == BB11B_OK_POWER_DETECTED)
		{			
            TraceOutput("Carrier sense return: Power Detected.\n");
			break;
        }
        else
        {
    		TraceOutput("Carrier sense return: 0x%08X\n", hr);
        }
	}

	do
	{		
        TraceOutput("Rx Begin.\n");

		pRxContext->b_dcOffset = pSpdContext->b_dcOffset;
		pRxContext->b_resetFlag = 1;

		hr = BB11BRx(pRxContext, &RxStream);

		if (hr == E_FETCH_SIGNAL_HW_TIMEOUT)
		{
			TraceOutput("Rx return: No more singals.\n");
		}
		else if (hr == E_FETCH_SIGNAL_FORCE_STOPPED)
		{
			TraceOutput("Rx return: Force stopped.\n");
		}
        else if (hr == BB11B_OK_FRAME)
		{
            monitor.IncGoodCounter();
            monitor.IncThroughput(pRxContext->BB11bCommon.b_length - 4); // Accumulate viterbi thread output length, excluding CRC32
			TraceOutput("Rx return: Good frame\n");
        }
		else if (hr == BB11B_E_DATA)
        {
            monitor.IncBadCounter();
			TraceOutput("Rx return: CRC Error\n");
        }
        else
        {
    		TraceOutput("Rx return : %08x\n", hr);
        }
	}while(FALSE);

	return true;
}

void Dot11BRxApp(const Config& config)
{
	HRESULT		hr;
	pSpdContext = aligned_malloc<BB11B_SPD_CONTEXT>();
	pRxContext = aligned_malloc<BB11B_RX_CONTEXT>();

	if (Dot11BRxInit() < 0)
		goto out;
	
	HANDLE hThread = AllocStartThread(DoDot11BRx);
    if (hThread)
    {
        printf("\n\nPress any key to exit the program\n");			
        time_t start = time(NULL);
	    while(!_kbhit())
        {
            if (config.Interval() != 0 && difftime(time(NULL), start) >= config.Interval()) break;
	    }

        StopFreeThread(hThread);
	}

out:

    SoraURadioReleaseRxStream(&RxStream, TARGET_RADIO);

	hr = SoraURadioUnmapRxSampleBuf(TARGET_RADIO, RxBuffer);
	printf("Unmap rx buffer ret: %08x\n", hr);
	printf("Rx out.\n");
}
