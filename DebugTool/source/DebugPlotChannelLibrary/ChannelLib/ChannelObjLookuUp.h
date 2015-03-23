#pragma once

#include "ReadWriteLock.h"

class Channel;
struct ChannelObjLookUpImpl;

enum ChannelOpenResult
{
	OK,
	ALLOCATION_FAIL
};

class ChannelObjLookUp
{
public:
	ChannelObjLookUp();
	~ChannelObjLookUp();

	ChannelOpenResult GetChannel(const char * channelName, int type, Channel ** pChannel);
	void Clean();
private:
	ChannelObjLookUpImpl * _impl;
};
