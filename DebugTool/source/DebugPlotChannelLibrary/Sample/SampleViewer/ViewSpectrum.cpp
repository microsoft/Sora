#include <Windows.h>
#include <assert.h>
#include "sora.h"
#include "DpViewer.h"
#include "DpViewerExt.h"
#include "ViewSpectrum.h"

void ViewSpectrum(CHANNEL_HANDLE hChannel)
{
	printf("Start reading spectrum\n");

	long long precessedCount = 0;
	DWORD lastReportTickCount = 0;

	HRESULT hResult = S_OK;

	int * buf = 0;
	int lenBuf = 0;

	int lenRead;
	int lenRequired;

	while(1)
	{
		// first, get the length required
		hResult = DpReadSpectrum(hChannel, 0, 0, 0, &lenRequired);

		if (hResult == S_OK)
		{
			continue;	// not enough data to determine length required
		}
		else if (hResult == E_NOT_ENOUGH_BUFFER) // the buffer size we provided is 0, so, E_NOT_ENOUGH_BUFFER is retured
		{
			// then we alloc 'enough' buffer
			if (lenRequired > lenBuf)
			{
				if (buf != 0)
					delete [] buf;
				buf = new int[lenRequired];
				lenBuf = lenRequired;
			}
		}
		else if (hResult == E_CHANNEL_CLOSED) // channel is closed, so we exit
		{
			break;
		}

		// read acture data
		hResult = DpReadSpectrum(hChannel, buf, lenBuf, &lenRead, 0);
		assert(lenRead > 0);

		precessedCount += lenRead;
		
		// report every 1 sec
		DWORD tickCount = ::GetTickCount();
		if (tickCount - lastReportTickCount > 1000)	// 1 sec
		{
			printf("\r%lld ints processed", precessedCount);
			lastReportTickCount = tickCount;
		}
	}
	
	printf("\nClose channel\n");
	DpCloseChannel(hChannel);	// release resource

	delete [] buf;
}
