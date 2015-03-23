#include <Windows.h>
#include <stdio.h>
#include "sora.h"
#include "DpChannel.h"
#include "DpChannelExt.h"
#include "PlotLine.h"

void PlotLine()
{
	const int BUF_SIZE = 1024;
	int buf[BUF_SIZE];
	int lastNum = 0;

	while(1)
	{
		// Prepare buffer
		int num = lastNum;
		int rnd = (rand() + 1) % BUF_SIZE;
		for (int i = 0; i < rnd; ++i)
			buf[i] = num++;

		// Call dp API
		int lenWritten = 0;
		HRESULT hRes = ::DpPlotLine("line", buf, rnd);

		// Check result
		if (hRes == S_OK)
		{
			lastNum = num;
		}
		else if (hRes == E_ALLOCATION_FAIL) {}		// not enough memory
	}	
}
