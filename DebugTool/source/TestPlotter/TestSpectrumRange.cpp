#include <Windows.h>
#include <stdio.h>
#include "DebugPlotU.h"

const int DATA_SIZE = 512;
static int dataBuf[DATA_SIZE];

void PrepareData()
{
	for (int i = 0; i < DATA_SIZE; ++i)
	{
		if (i % 128 == 0)
			dataBuf[i] = INT_MAX;
		else
			dataBuf[i] = INT_MIN;
	}

	dataBuf[DATA_SIZE - 1] = INT_MAX;
}

int TestSpectrumRange()
{
	::DebugPlotInit();

	PrepareData();

	::PlotSpectrum("spectrum", dataBuf, DATA_SIZE);

	printf("Press any key to exit\n");
	getchar();

	::DebugPlotDeinit();

	return 0;
}
