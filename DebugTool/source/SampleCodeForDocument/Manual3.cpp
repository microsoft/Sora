#include <stdio.h>
#include <math.h>
#include "DebugPlotU.h"

#define BUF_SIZE  128
static COMPLEX16 rawDataBuf[BUF_SIZE];

void GetSomeDataFromDMABuffer(COMPLEX16 * buffer, int numSample) {
	// code for raw data fetching from DMA buffer
}

void ProcessRawData(COMPLEX16 * buffer, int numSample) {
	// code for raw data processing
}

int Manual3()
{
	HRESULT hRes;

	DebugPlotInit();

	Log("log", "program started");

	while(1)
	{
		while(1)
		{
			GetSomeDataFromDMABuffer(rawDataBuf, BUF_SIZE);
			hRes = ::TracebufferWriteData(rawDataBuf, BUF_SIZE, 0);
			if (hRes == E_END_OF_BUF)
				break;
		}

		while(1)
		{
			int numRead;
			TracebufferReadData(rawDataBuf, BUF_SIZE, &numRead);
			if (numRead > 0)
				ProcessRawData(rawDataBuf, numRead);
			else
				break;
		}

		TracebufferClear();
	}

	DebugPlotDeinit();

	return 0;
}
