#pragma once

#include <windows.h>
#include <vector>

class CPlotWnd;
class CChannelExplorerWnd;
class CPlayControlWnd;

class AppContext
{
public:
	static AppContext * Instance();

private:
	AppContext();
	static AppContext * instance;

public:

	std::vector<CPlotWnd *> plotWnds;
	CChannelExplorerWnd * channelExplorerWnd;
	CPlayControlWnd * playControlWnd;

	int DEFAULT_PLOT_WND_WIDTH;
	int DEFAULT_PLOT_WND_HIGHT;
	int GRID_SIZE;

	int plotWndIndex;

	DWORD plotWndStyle;
	DWORD plotWndExStyle;
};
