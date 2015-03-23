#include <stdio.h>
#include <math.h>
#include "DebugPlotU.h"

#define BUF_SIZE  128
static int lineDataBuf[BUF_SIZE];

static void PrepareData()
{
	const int AMPLITUDE = 127;
	const int PERIOD = 32;
	const double PI_2 = 3.14;

	for (int i = 0; i < BUF_SIZE; i++)
	{
		double phase		= i * PI_2 / PERIOD;
		lineDataBuf[i]		= (int)(AMPLITUDE * cos(phase));
	}
}

int Manual2()
{
	DebugPlotInit();

	PrepareData();

	int numberOfWrites = 0;

	while(numberOfWrites < 10000)
	{
		WaitForViewer(INFINITE);
		PlotLine("line graph", lineDataBuf, BUF_SIZE);
		numberOfWrites++;

		if (numberOfWrites == 5000)
			PauseViewer();
	}

	getchar();

	DebugPlotDeinit();

	return 0;
}
