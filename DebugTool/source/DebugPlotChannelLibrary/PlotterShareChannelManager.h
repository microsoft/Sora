#pragma once

#include "Channel.h"

class PlotterShareChannelManager
{
public:
	static ShareChannelManager * Instance();
	static Channel * Open(int pid, const char * name, int type);
	static void Close(Channel * pCh);
	static void Gc();
};
