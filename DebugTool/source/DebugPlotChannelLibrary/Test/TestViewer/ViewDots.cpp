#include <Windows.h>
#include <assert.h>
#include "sora.h"
#include "DpViewer.h"
#include "DpViewerExt.h"
#include "ViewDots.h"

void ViewDots(CHANNEL_HANDLE hChannel)
{
	printf("Start reading dots\n");

	const int BUF_SIZE = 1024;
	COMPLEX16 buf[BUF_SIZE];
	int lenRead = 0;
	HRESULT hResult = S_OK;

	bool firstRead = true;
	int lastNum = -1;

	long long precessedCount = 0;
	DWORD lastReportTickCount = 0;

	while(1)
	{
		hResult = DpReadDots(hChannel, buf, BUF_SIZE, &lenRead);
		if (lenRead > 0)
		{
			if (firstRead)
			{
				lastNum = buf[0].re & 0xffff;
				firstRead = false;	
			}

			for (int i = 0; i < lenRead; ++i)
			{
				assert((buf[i].re & 0xffff) == lastNum);
				assert((buf[i].im & 0xffff) == lastNum);
				lastNum = (lastNum+1) & 0xffff;
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
