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

	long long precessedCount = 0;
	DWORD lastReportTickCount = 0;

	while(1)
	{
		HRESULT hResult = DpReadDots(hChannel, buf, BUF_SIZE, &lenRead); // returns S_OK, or E_CHANNEL_CLOSED
		if (lenRead > 0)	// there're still data, we don't care whether channel is closed
		{
			precessedCount += lenRead;
			
			// report every 1 sec
			DWORD tickCount = ::GetTickCount();
			if (tickCount - lastReportTickCount > 1000)	// 1 sec
			{
				printf("\r%lld ints processed", precessedCount);
				lastReportTickCount = tickCount;
			}
		}
		else	// no data is read, so we check if the channel is closed
		{
			if (hResult == E_CHANNEL_CLOSED)
				break;
			else
				::Sleep(15);
		}
	}
	
	printf("\nClose channel\n");
	DpCloseChannel(hChannel);	// release resource
}
