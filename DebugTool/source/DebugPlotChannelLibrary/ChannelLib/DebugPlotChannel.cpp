#include <Windows.h>
#include <stdio.h>
#include "Channel.h"
#include "ShareChannelManager.h"
#include "__DebugPlotChannel.h"
#include "DpChannel.h"
#include "ChannelObjLookuUp.h"

struct _PlotterMain
{
	ChannelObjLookUp channelCache;

	_PlotterMain()
	{
	}

	~_PlotterMain()
	{
		channelCache.Clean();
		ShareChannelManager::Release();
	}
};

static _PlotterMain main;

HRESULT __stdcall __PlotData(const char * name, int type, const char * buf, int length)
{
	Channel * pChannel;
	ChannelOpenResult res = main.channelCache.GetChannel(name, type, &pChannel);

	if (pChannel != 0)
	{
		bool succ = pChannel->Write(buf, length);
		if (succ)
			return S_OK;
		else
			return E_ALLOCATION_FAIL;
	}

	switch(res)
	{
	case ALLOCATION_FAIL:
		return E_ALLOCATION_FAIL;
	}

	return S_OK;
}

HRESULT __stdcall DpPlotData(const char * name, int type, const char * buf, int length)
{
	int type2 = type & 0x7FFFFFFF;
	return __PlotData(name, type2, buf, length);
}
