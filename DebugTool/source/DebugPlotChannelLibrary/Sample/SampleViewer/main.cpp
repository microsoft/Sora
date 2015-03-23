#include <stdio.h>
#include "DpViewer.h"
#include "DpViewerExt.h"
#include "ViewData.h"
#include "ViewLine.h"
#include "ViewDots.h"
#include "ViewSpectrum.h"
#include "ViewText.h"

/**********************

enumarate all channels.
if a channel is found, try open it
if succeed, set the channel info (handle, type), and return true

return false if no channel opened after enumaration has done.

**********************/

struct ChannelSearchInfo
{
	int type;
	CHANNEL_HANDLE hChannel;
};

bool SearchChannel(ChannelSearchInfo * searchInfo)
{
	// To seach channel
	// 1. DpFindFirstChannel --> begin search
	// 2. DpFindNextChannel  --> find next, repeatedly
	// 3. DpCloseFindChannel --> clean resources

	bool bFound = false;

	ChannelInfo channelInfo;

	SEARCH_HANDLE hSearch = DpFindFirstChannel(&channelInfo); // 1. start search

	if (hSearch != INVALID_HANDLE_VALUE)
	{
		do
		{
				CHANNEL_HANDLE hChannel = DpOpenChannel(channelInfo.Pid, channelInfo.Name, channelInfo.Type);
				if (hChannel != INVALID_HANDLE_VALUE)	// if INVALID_HANDLE_VALUE is returned, 
														// the channelcannot open, because writer has already exited, 
														// or the channel has opened by other reader.
				{
					searchInfo->type = channelInfo.Type;
					searchInfo->hChannel = hChannel;
					bFound = true;
					break;
				}
				else {} // writer exited, or, channel has already opened by other reader
		}
		while(DpFindNextChannel(hSearch, &channelInfo) != FALSE);	// 2. find next channel

		DpCloseFindChannel(hSearch);	// 3. finish searching, close handle
	}

	return bFound;
}

void ProcessChannel(ChannelSearchInfo * info)
{
	switch (info->type)
	{
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
		ViewData(info->hChannel);
	}	
}

int __cdecl main()
{
	ChannelSearchInfo searchInfo;

	while(1)
	{
		::Sleep(200);		// Scan the channels every 200 secs
		
		bool succ = SearchChannel(&searchInfo);

		if (succ)
		{
			ProcessChannel(&searchInfo);
		}
	}

	return 0;
}

