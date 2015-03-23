#include "stdafx.h"
#include "AppContext.h"

AppContext * AppContext::instance = 0;

AppContext::AppContext()
{
	DEFAULT_PLOT_WND_WIDTH = 300;
	DEFAULT_PLOT_WND_HIGHT = 300;
	GRID_SIZE = 20;

	plotWndIndex = 5;
}

AppContext * AppContext::Instance()
{
	if (instance)
		return instance;
	else
	{
		instance = new AppContext();
		return instance;
	}
}
