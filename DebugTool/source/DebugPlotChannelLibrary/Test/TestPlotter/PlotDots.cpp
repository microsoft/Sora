#include <Windows.h>
#include <stdio.h>
#include "sora.h"
#include "DpChannel.h"
#include "DpChannelExt.h"
#include "PlotDots.h"

void PlotDots()
{
	const int BUF_SIZE = 1024;
	COMPLEX16 buf[BUF_SIZE];
	int lastNum = 0;

	while(1)
	{
		// Prepare buffer
		int num = lastNum;
		int rnd = (rand() + 1) % BUF_SIZE;
		for (int i = 0; i < rnd; ++i)
		{
			buf[i].re = num;
			buf[i].im = num++;
		}

		// Call dp API
		int lenWritten = 0;
		HRESULT hRes = ::DpPlotDots("dots", buf, rnd);

		// Check result
		if (hRes == S_OK)
		{
			lastNum = num;
		}
		else if (hRes == E_ALLOCATION_FAIL) {}		// not enough memory
	}	
}
