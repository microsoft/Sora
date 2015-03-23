#include <math.h>
#include "DebugPlotU.h"

#define BUF_SIZE 64
static COMPLEX16 dataBuf[BUF_SIZE];
static int reBuf[BUF_SIZE];
static int imBuf[BUF_SIZE];

void Tut1()
{
	HRESULT hr = S_OK;

	hr = ::DebugPlotInit();
	if (FAILED(hr))
		return;

	// Prepare data
	const int AMP = 127;
	const int PERIOD = 100;
	const double PI_2 = 3.14159*2;

	int count = 0;
	
	while(1)
	{
		::WaitForViewer(INFINITE);

		for (int i = 0; i < BUF_SIZE; i++)
		{
			double re;
			double im;

			if (1)
			{
				double phase = PI_2 * (count++) / PERIOD;
				re = AMP * sin(phase);
				im = AMP * cos(phase); 
			}
			else
				re = im = 0.0;

			dataBuf[i].re = reBuf[i] = short(re);
			dataBuf[i].im = imBuf[i] = short(im);
		}
		
		// plot
		PlotLine("real", reBuf, BUF_SIZE);
		PlotLine("imaginary", imBuf, BUF_SIZE);
		PlotDots("constellation", dataBuf, BUF_SIZE);
		PlotText("sample count", "%d\n", count);
	}

	DebugPlotDeinit();
}
