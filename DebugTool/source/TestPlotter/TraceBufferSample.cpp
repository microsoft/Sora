/*

	There are 3 APIs associated with raw data trace buffer:
	TraceBufferWriteData
	TraceBufferReadData
	TraceBufferClear

	The following program demostrates the usage of 3 APIs.

*/


#include <math.h>
#include "DebugPlotU.h"

#define BUF_SIZE 16*1024
static COMPLEX16 dataBuf[BUF_SIZE];

static void PrepareData()
{
	for (int i = 0; i < BUF_SIZE; i++)
	{
		dataBuf[i].im = 0;
		dataBuf[i].re = (short)i;
	}
}

int TraceBufferSample()
{
	HRESULT hr = S_OK;

	hr = DebugPlotInit();
	if (FAILED(hr))
		return -1;

	while(1)
	{
		// 1. Get data somehow, e.g. from DMA buffer
		PrepareData();

		// 2. rite the raw data into the trace buffer at once
		// Here we write 16k at once

		/*
		HRESULT __stdcall TracebufferWriteData(
		__in COMPLEX16 * pData,
		__in int inNum,
		__out int * pOutNum
		)

		pData:
			start pointer of the buffer
		inNum:
			length of the buffer, the unit is COMPLEX16
		pOutNum:
			pointer to a int which will be filled with the nunber of COMPLEX16s actually written. If 0, ignored.
		return:
			S_OK
			E_INVALID_PARAMETER: parameters are incorrect
			E_ALLOCATION_FAIL: memory allocation failed
			E_END_OF_BUF: buffer is empty after this operation, (and *pOutNum is less than inNum)
		*/
		TracebufferWriteData(dataBuf, BUF_SIZE, 0);

		while(true)
		{
			Sleep(15);
			COMPLEX16 data;
			// 3. Read some data from the trace buffer for processing 
			HRESULT hr = TracebufferReadData(&data, 1, 0);
			/*
			
			HRESULT __stdcall TracebufferReadData(
			__in COMPLEX16 * pData,
			__in int inNum,
			__out int * pOutNum
			);

			pData:
				start pointer of the buffer
			inNum:
				length of the buffer, the unit is COMPLEX16
			pOutNum:
				pointer to a int which will be filled with the nunber of COMPLEX16s actually Read. If 0, ignored.


			return:
				S_OK
				E_INVALID_PARAMETER: parameters are incorrect
				E_ALLOCATION_FAIL: memory allocation failed
				E_END_OF_BUF: buffer is empty after this operation, (and *pOutNum is less than inNum)
			*/


			if (hr == E_END_OF_BUF) // Trace buffer is empty, so break to write new data into the trace buffer
				break;

			// 4. process the data
			int result = data.re + data.im;
			Log("sum", "%d\n", result);
		}

		// Clear the trace buffer, reset read pointer and write pointer to the start of the trace buffer.
		::TracebufferClear();
	}

	DebugPlotDeinit();

	return 0;
}
