#pragma once

struct ChannelSearchInfo
{
	int type;
	CHANNEL_HANDLE hChannel;
};

typedef void (* ChannelHandler)(CHANNEL_HANDLE, int);

bool SearchChannel(ChannelSearchInfo * searchInfo);

