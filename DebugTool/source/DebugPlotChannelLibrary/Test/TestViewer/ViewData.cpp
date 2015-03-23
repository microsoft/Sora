#include <Windows.h>
#include <assert.h>
#include "DpViewer.h"
#include <stdio.h>
#include "ViewData.h"
#include "SearchChannel.h"

void ViewData(CHANNEL_HANDLE hChannel)
{
	printf("Start reading data");

	const int BUF_SIZE = 256;
	char buf[BUF_SIZE];

	long long precessedCount = 0;
	DWORD lastReportTickCount = 0;

	while(1)
	{
		char size;
		int sizeRead;
		HRESULT hRes = ::DpPeekChannelData(hChannel, &size, sizeof(char), &sizeRead);
		if (sizeRead == 1)
		{
			int count = (int)size & 0xFF;
			hRes = ::DpReadChannelData(hChannel, buf, count, &sizeRead);
			assert(count == sizeRead);

			for (int i = 0; i < count; ++i)
			{
				assert(buf[i] == size);
			}

			precessedCount += count;

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
	DpCloseChannel(hChannel);
}
