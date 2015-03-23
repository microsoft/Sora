// TestPlotter.cpp : Defines the entry point for the console application.
//

#include <Windows.h>
#include <assert.h>
#include "DebugPlotU.h"
//#ifdef DEBUG_USE_VLD
//#include "vld.h"
//#endif

extern void Tut1();
extern void Tut2();
extern int Manual1();
extern int Manual2();
extern int Manual3();

int __cdecl main()
{
	//Manual1();
	Manual2();
	//Manual3();
	return 0;
}


