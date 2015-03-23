// TestPlotter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <assert.h>
#include "DebugPlotU.h"
#ifdef DEBUG_USE_VLD
#include "vld.h"
#endif

extern bool TestAPIs();

int _tmain(int argc, _TCHAR* argv[])
{
	TestAPIs();

	return 0;
}


