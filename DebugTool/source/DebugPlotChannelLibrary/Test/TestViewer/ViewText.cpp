#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "sora.h"
#include "DpViewer.h"
#include "DpViewerExt.h"
#include "ViewText.h"
#include "SearchChannel.h"

void ViewText(CHANNEL_HANDLE hChannel)
{
	printf("Start reading text");

	const int BUF_SIZE = 8128;
	char buf[BUF_SIZE];

	long long precessedCount = 0;
	DWORD lastReportTickCount = 0;

	while(1)
	{
		int lenRead;
		int lenRequired;

		//int 
		HRESULT hRes = ::DpReadText(hChannel, buf, BUF_SIZE, &lenRead, &lenRequired);

		if (lenRead > 0)
		{
			printf(buf);

			precessedCount += lenRead;

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
	DpCloseChannel(hChannel);
}
