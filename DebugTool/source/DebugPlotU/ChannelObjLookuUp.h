#pragma once

#include "SharedChannelW.h"
#include "ReadWriteLock.h"

class ChannelObjLookUp
{
public:
	static SharedChannelW * __stdcall GetSharedChannel(const char * channelName, ChannelType chType, int spectrumSize);
	static void __stdcall Clean();
};
