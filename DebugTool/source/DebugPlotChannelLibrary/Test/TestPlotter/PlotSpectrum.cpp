#include <Windows.h>
#include <stdio.h>
#include "sora.h"
#include "DpChannel.h"
#include "DpChannelExt.h"
#include "PlotSpectrum.h"

void PlotSpectrum()
{
	const int BUF_SIZE = 1024;
	int buf[BUF_SIZE];
	int lastNum = 0;
	while(1)
	{
		// Prepare buffer
		for (int i = 0; i < BUF_SIZE; ++i)
			buf[i] = lastNum;

		// Call dp API
		int lenWritten = 0;
		HRESULT hRes = ::DpPlotSpectrum("spectrum", buf, BUF_SIZE);

		// Check result
		if (hRes == S_OK)
		{
			lastNum++;
		}
		else if (hRes == E_ALLOCATION_FAIL) {}		// not enough memory
	}	
}
