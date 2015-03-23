#include <Windows.h>
#include <assert.h>
#include "DpViewer.h"
#include <stdio.h>
#include "ViewData.h"

void ViewData(CHANNEL_HANDLE hChannel)
{
	printf("Start reading data");

	const int BUF_SIZE = 256;
	char buf[BUF_SIZE];

	long long precessedCount = 0;
	DWORD lastReportTickCount = 0;

	while(1)
	{
		int sizePeeked;
		char peekData;

		// Just shows how to peek data
		// if peak data, the read pointer won't change
		// peak n times will get the same value
		HRESULT hRes = ::DpPeekChannelData(hChannel, &peekData, sizeof(char), &sizePeeked);
		if (sizePeeked == sizeof(char))
		{
			// if we peek for a second time, then get the same value
			char peekData2;
			hRes = ::DpPeekChannelData(hChannel, &peekData2, sizeof(char), &sizePeeked);
			assert(sizePeeked == sizeof(char));
			assert(peekData == peekData2);
		}
		else
		{
			if (hRes == E_CHANNEL_CLOSED)	// channel is closed, finish reading
				break;
			else if (hRes == S_OK)	// not enough data, sleep
				Sleep(15);			
		}

		// read data, the read pointer will be updated
		int sizeRead;
		hRes = ::DpReadChannelData(hChannel, buf, BUF_SIZE, &sizeRead);

		if (sizeRead > 0)
		{
			precessedCount += sizeRead;
			
			// report every 1 sec
			DWORD tickCount = ::GetTickCount();
			if (tickCount - lastReportTickCount > 1000)	// 1 sec
			{
				printf("\r%lld bytes processed", precessedCount);
				lastReportTickCount = tickCount;
			}
		}
		else
		{
			if (hRes == E_CHANNEL_CLOSED)	// channel is closed, finish reading
				break;
			else if (hRes == S_OK)	// not enough data, sleep
				Sleep(15);
		}
	}

	printf("\nClose channel\n");
	DpCloseChannel(hChannel);	// release resource
}
