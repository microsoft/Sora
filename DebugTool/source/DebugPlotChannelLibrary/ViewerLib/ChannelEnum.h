#pragma once

#include "ChannelTableStruct.h"
#include "DpViewer.h"

struct ChannelRecord
{
	::ChannelInfo channelInfo;
	ChannelRecord * next;
};

ChannelRecord * FindAll();

void FreeList(ChannelRecord * record);

