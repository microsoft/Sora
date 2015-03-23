#include <Windows.h>
#include <assert.h>
#include "sora.h"
#include "DpViewer.h"
#include "DpViewerExt.h"
#include "ViewLine.h"

void ViewLine(CHANNEL_HANDLE hChannel)
{
	printf("Start reading line\n");

	const int BUF_SIZE = 1024;
	int buf[BUF_SIZE];
	int lenRead = 0;
	HRESULT hResult = S_OK;

	bool firstRead = true;
	int lastNum = -1;

	long long precessedCount = 0;
	DWORD lastReportTickCount = 0;

	while(1)
	{
		hResult = DpReadLine(hChannel, buf, BUF_SIZE, &lenRead);
		if (lenRead > 0)
		{
			if (firstRead)
			{
				lastNum = buf[0];
				firstRead = false;	
			}

			for (int i = 0; i < lenRead; ++i)
			{
				assert(buf[i] == lastNum);
				lastNum++;
			}

			precessedCount += lenRead;

			DWORD tickCount = ::GetTickCount();
			if (tickCount - lastReportTickCount > 1000)	// 1 sec
			{
				printf("\r%lld ints processed", precessedCount);
				lastReportTickCount = tickCount;
			}
		}
		else
		{
			if (hResult == E_CHANNEL_CLOSED)
				break;
			else
				::Sleep(15);
		}
	}
	
	printf("\nClose channel\n");
	DpCloseChannel(hChannel);
}
