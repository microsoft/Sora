#include <Windows.h>
#include <stdio.h>
#include "DpChannel.h"

#define BUF_SIZE 256

static char buf[BUF_SIZE];

int PrepreBuffer()
{
	int rnd = (rand() + 1) % BUF_SIZE;
	for (int i = 0; i < rnd; ++i)
		buf[i] = rnd;

	return rnd;
}

void PlotData()
{
	long long totalWrittenCount = 0;
	DWORD lastReportTickCount = 0;

	while(1)
	{
		// Prepare buffer

		int dataLen = PrepreBuffer();

		// Call dp API
		int lenWritten = 0;
		HRESULT hRes = ::DpPlotData("data", 1, buf, dataLen);

		// Check result
		if (hRes == S_OK)	// do some simple statistics
		{
			totalWrittenCount += dataLen;
			
			DWORD tickCount = ::GetTickCount();
			if (tickCount - lastReportTickCount > 1000)	// 1 sec
			{
				lastReportTickCount = tickCount;
				printf("\r%lld bytes processed", totalWrittenCount);
			}
		}
		else if (hRes == E_ALLOCATION_FAIL) {} // not enough memory
	}
}
