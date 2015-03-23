#include <Windows.h>
#include "sora.h"
#include "DpChannel.h"
#include "DpChannelExt.h"
#include "PlotText.h"

#define BUF_SIZE 8128

static char buf[BUF_SIZE];

void PlotText()
{
	int num = 0;
	while(1)
	{
		// Call dp API
		int lenWritten = 0;
		HRESULT hRes = ::DpPlotText("text", "number: %d", num);

		// Check result
		if (hRes == S_OK) {
			num++;
		}
		else if (hRes == E_ALLOCATION_FAIL) {}		// not enough memory
	}
}
