#include <Windows.h>
#include <set>
#include "DpViewer.h"
#include "SearchChannel.h"

/**********************

enumarate all channels.
if a channel is found, try open it
if succeed, set the channel info (handle, type), and return true

return false if no channel opened after enumaration has done.

**********************/

bool SearchChannel(ChannelSearchInfo * searchInfo)
{	
	bool bFound = false;

	ChannelInfo channelInfo;

	// 1. start search
	SEARCH_HANDLE hSearch = DpFindFirstChannel(&channelInfo);

	if (hSearch != INVALID_HANDLE_VALUE)
	{
		do
		{
			auto hChannel = DpOpenChannel(channelInfo.Pid, channelInfo.Name, channelInfo.Type);
			if (hChannel != INVALID_HANDLE_VALUE)
			{
				searchInfo->type = channelInfo.Type;
				searchInfo->hChannel = hChannel;
				bFound = true;
				break;
			}
			else {} // writer exited, or, channel has already opened by other reader
		}
		while(DpFindNextChannel(hSearch, &channelInfo) != FALSE);	// 2. find next channel

		// 3. finish searching, close handle
		DpCloseFindChannel(hSearch);
	}

	return bFound;
}
