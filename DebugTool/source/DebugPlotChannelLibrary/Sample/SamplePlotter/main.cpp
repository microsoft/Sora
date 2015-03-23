#include <Windows.h>
#include "DpChannel.h"
#include "DpChannelExt.h"

void PlotSample();
void PlotSampleExt();

int __cdecl main()
{
	PlotSample();
	PlotSampleExt();
	getchar();
}


/*

	Shows how to use basic plot API:

	DpPlotData

*/

void PlotSample()
{
	const int BUF_SIZE = 1024;
	char buf[BUF_SIZE];

	// prepare data
	for (int i = 0; i < BUF_SIZE; ++i)
	{
		buf[i] = (char)i;
	}

	HRESULT hRes = ::DpPlotData(
		"channel name 1",		// name of the channel
		1,						// type, a positive integer
		buf,					// pointer of the buffer
		BUF_SIZE);				// buffer size

	// check result
	switch(hRes)
	{
	case S_OK:					// ALL data has been written in shared memory
		break;
	case E_ALLOCATION_FAIL:		// not enough memory, so NO data has been written in shared memory
		break;
	}
}
/*
	Shows how to use extension plot API:

	DpPlotLine
	DpPlotSpectrum
	DpPlotDots
	DpPlotText

	take DpPlotSpectru for example
*/

void PlotSampleExt()
{
	const int BUF_SIZE = 1024;
	int buf[BUF_SIZE];

	// prepare data
	for (int i = 0; i < BUF_SIZE; ++i)
	{
		buf[i] = i;
	}

	HRESULT hRes = DpPlotSpectrum("spectrum", buf, BUF_SIZE);
	
	// check result
	switch(hRes)
	{
	case S_OK:					// ALL data has been written in shared memory
		break;
	case E_ALLOCATION_FAIL:		// not enough memory, so NO data has been written in shared memory
		break;
	}
}
