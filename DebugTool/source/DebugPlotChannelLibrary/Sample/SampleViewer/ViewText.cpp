#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "sora.h"
#include "DpViewer.h"
#include "DpViewerExt.h"
#include "ViewText.h"

void ViewText(CHANNEL_HANDLE hChannel)
{
	printf("Start reading text");

	long long precessedCount = 0;
	DWORD lastReportTickCount = 0;

	int lenRead;
	int lenRequired;

	char * buf = 0;
	int lenBuf = 0;

	while(1)
	{
		
		// first, get the length required
		HRESULT hRes = ::DpReadText(hChannel, 0, 0, 0, &lenRequired);

		if (hRes == S_OK)
		{
			continue;	// not enough data to determine length required
		}
		else if (hRes == E_NOT_ENOUGH_BUFFER) // the buffer size we provided is 0, so, E_NOT_ENOUGH_BUFFER is retured
		{
			// then we alloc 'enough' buffer
			if (lenRequired > lenBuf)
			{
				if (buf != 0)
					delete [] buf;
				buf = new char[lenRequired];
				lenBuf = lenRequired;
			}
		}
		else if (hRes == E_CHANNEL_CLOSED) // channel is closed, so we exit
		{
			break;
		}

		// read acture data
		hRes = ::DpReadText(hChannel, buf, lenBuf, &lenRead, 0);

		if (lenRead > 0)
		{
			precessedCount += lenRead;

			// report every 1 sec
			DWORD tickCount = ::GetTickCount();
			if (tickCount - lastReportTickCount > 1000)	// 1 sec
			{
				printf("\r%lld chars processed", precessedCount);
				lastReportTickCount = tickCount;
			}
		}
		else
		{
			if (hRes == E_CHANNEL_CLOSED)
			{
				break;
			}
			else
			{
				::Sleep(15);
			}
		}
	}

	printf("\nClose channel\n");
	DpCloseChannel(hChannel);	// release resource
}
