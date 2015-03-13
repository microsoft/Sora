#include <math.h>
#include "DebugPlotU.h"

#define BUF_SIZE 16*1024
static COMPLEX16 dataBuf[BUF_SIZE];

void PrepareData()
{
	for (int i = 0; i < BUF_SIZE; i++)
	{
		dataBuf[i].im = 0;
		dataBuf[i].re = (short)i;
	}
}

int __cdecl main()
{
	HRESULT hr = S_OK;

	hr = DebugPlotInit();
	if (FAILED(hr))
		return -1;

	PrepareData();

	TracebufferWriteData(dataBuf, BUF_SIZE, 0);
	
	while(true)
	{
		Sleep(1000);
		COMPLEX16 data;
		HRESULT hr = TracebufferReadData(&data, 1, 0);
		if (hr == E_END_OF_BUF)
			break;
		
		int result = data.re + data.im;

		Log("sum", "%d\n", result);
	}

	DebugPlotDeinit();
	
	return 0;
}
