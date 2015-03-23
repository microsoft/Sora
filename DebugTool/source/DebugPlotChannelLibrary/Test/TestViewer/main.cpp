#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "sora.h"
#include "DpViewer.h"
#include "DpViewerExt.h"
#include <hash_map>
#include <set>
#include <process.h>
#include "ViewData.h"
#include "ViewLine.h"
#include "ViewSpectrum.h"
#include "ViewDots.h"
#include "ViewText.h"
//#include "vld.h"
#include "SearchChannel.h"

int sizeLimit = CHANNEL_BUFFER_SIZE_UNLIMITED;

void Default(CHANNEL_HANDLE hChannel)
{
	printf("Unknown channel type\n");
	DpCloseChannel(hChannel);
}

void ProcessChannel(ChannelSearchInfo * info)
{
	DpSetChannelBufferLimit(info->hChannel, sizeLimit);

	switch (info->type)
	{
	case 1:	// data
		ViewData(info->hChannel);
		break;
	case CH_LINE_TYPE:
		ViewLine(info->hChannel);
		break;
	case CH_SPECTRUM_TYPE:
		ViewSpectrum(info->hChannel);
		break;
	case CH_DOTS_TYPE:
		ViewDots(info->hChannel);
		break;
	case CH_TEXT_TYPE:
		ViewText(info->hChannel);
		break;
	default:
		Default(info->hChannel);
	}	
}

int main(int argc, char ** argv)
{
	printf("TestViewer started\n");

	if (argc == 2)
	{
		sizeLimit = atoi(argv[1]);
	}

	ChannelSearchInfo searchInfo;

	while(1)
	{
		::Sleep(200);
		
		bool succ = SearchChannel(&searchInfo);

		if (succ)
		{
			ProcessChannel(&searchInfo);
		}
	}

	return 0;
}

//void Test1(ChannelFunc func)
//{
//	printf("Viewer started\n", ::GetCurrentProcessId());
//	printf("pid: %d\n", ::GetCurrentProcessId());
//
//	std::set<int> setChannel;
//	std::vector<HANDLE> vecThrHandle;
//
//	while(1)
//	{
//		::Sleep(1000);
//		
//		ChannelInfo * pChannelInfo;
//
//		printf("search channel\n");
//
//		SEARCH_HANDLE hSearch = DpFindFirstChannel(&pChannelInfo);
//
//		if (hSearch != INVALID_HANDLE_VALUE)
//		{
//			do
//			{
//				if (pChannelInfo != 0)
//				{
//					if (setChannel.find(pChannelInfo->Pid) == setChannel.end())
//					{
//						printf("find channel\n");
//						printf("name %s\n", pChannelInfo->Name);
//						printf("pid %d\n", pChannelInfo->Pid);
//						printf("type %d\n\n", pChannelInfo->Type);
//						
//						auto hChannel = DpOpenChannel(pChannelInfo->Pid, pChannelInfo->Name);
//						if (hChannel != INVALID_HANDLE_VALUE)
//						{
//							//setChannel.insert(pChannelInfo->Pid);
//							ThreadContext *context = new ThreadContext();
//							context->hChannel = hChannel;
//							context->type = pChannelInfo->Type;
//
//							//auto hThread = (HANDLE) ::_beginthread(ThreadPlot, 0, context);
//							//vecThrHandle.push_back(hThread);
//
//							func(context);
//						}
//					}
//				}
//			}
//			while(DpFindNextChannel(hSearch, &pChannelInfo) != FALSE);
//
//			DpCloseFindChannel(hSearch);
//		}
//	}
//}
//
//void __cdecl ThreadPlot(PVOID param)
//{
//	ThreadContext * context = (ThreadContext *)param;
//	CHANNEL_HANDLE hChannel = context->hChannel;
//	delete context;
//
//	const int BUF_SIZE = 1024;
//	char buf[BUF_SIZE];
//	int lenRead;
//
//	int lenTotal = 0;
//	int reportCounter = 0;
//	const int reportThreshold = 1*1024*1024;
//
//	while (1)
//	{
//		HRESULT res = DpPeekChannelData(hChannel, buf, 1, &lenRead);
//		if (res == E_CHANNEL_CLOSED)
//			goto EXIT;
//
//		if (lenRead != 1)
//			continue;
//
//		unsigned count = (unsigned)buf[0];
//
//		while(1)
//		{
//			int size;
//			res = DpGetChannelReadableSize(hChannel, &size);
//			if (res == E_CHANNEL_CLOSED && size < count)
//				goto EXIT;
//
//			if (size >= count)
//				break;
//		}
//
//		res = DpReadChannelData(hChannel, buf, count, &lenRead);
//		assert(res == S_OK);
//		assert(lenRead == count);
//
//		// verify result
//		char content = buf[0];
//
//		for (int i = 0; i < count; ++i)
//		{
//			if (content != buf[i])
//			{
//				printf("error");
//				return;
//			}
//		}
//
//		if (lenRead > 0)
//		{
//			lenTotal += lenRead;
//			reportCounter += lenRead;
//			if (reportCounter > reportThreshold)
//			{
//				reportCounter -= reportThreshold;
//				printf("total: %d\n", lenTotal);
//			}
//
//			//buf[lenRead] = 0;
//			//printf("data read\n");
//			//printf("size: %d\n", lenRead);
//			//printf("data: %s\n", buf);
//			//printf("total: %d\n", lenTotal);
//		}
//		else
//		{
//			::Sleep(1);
//		}
//	}
//
//EXIT:
//	DpCloseChannel(hChannel);
//
//	//_endthread();
//
//
//}
//
//void ReadLine(CHANNEL_HANDLE hChannel)
//{
//	const int BUF_SIZE = 1024;
//	int buf[BUF_SIZE];
//	int lenRead = 0;
//	HRESULT hResult = S_OK;
//	while(1)
//	{
//		hResult = ReadLine(hChannel, buf, BUF_SIZE, &lenRead);
//		if (lenRead > 0)
//		{
//			for (int i = 0; i < lenRead; ++i)
//			{
//				printf("%d ", buf[i]);
//			}
//			printf("\n");
//		}
//		else
//		{
//			if (hResult == E_CHANNEL_CLOSED)
//				break;
//			else
//				::Sleep(1);
//		}
//	}
//
//	DpCloseChannel(hChannel);
//}
//
//void __cdecl ThreadPlotExt(PVOID param)
//{
//	ThreadContext * context = (ThreadContext *)param;
//	CHANNEL_HANDLE hChannel = context->hChannel;
//	int type = context->type;
//	delete context;
//
//	switch(type)
//	{
//	case CH_LINE_TYPE:
//		ReadLine(hChannel);
//		break;
//	}
//}
