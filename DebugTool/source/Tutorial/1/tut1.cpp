#include <math.h>
#include "DebugPlotU.h"

#define BUF_SIZE 64
static COMPLEX16 dataBuf[BUF_SIZE];
static int reBuf[BUF_SIZE];
static int imBuf[BUF_SIZE];

#define AMP			127
#define PERIOD		100
#define PI_2		(3.14159*2)

int __cdecl main()
{
	HRESULT hr = S_OK;

	hr = DebugPlotInit();
	if (FAILED(hr))
		return -1;

	int count = 0;
	
	while(1)
	{
		WaitForViewer(INFINITE);

		for (int i = 0; i < BUF_SIZE; i++)
		{
			double phase = PI_2 * (count++) / PERIOD;
			double re = AMP * sin(phase);
			double im = AMP * cos(phase); 

			dataBuf[i].re = reBuf[i] = short(re);
			dataBuf[i].im = imBuf[i] = short(im);
		}
		
		PlotLine("real", reBuf, BUF_SIZE);
		PlotLine("imaginary", imBuf, BUF_SIZE);
		PlotDots("constellation", dataBuf, BUF_SIZE);
		PlotText("sample count", "%d\n", count);
	}

	DebugPlotDeinit();

	return 0;
}
