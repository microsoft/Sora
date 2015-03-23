#pragma once

#include <windows.h>

/*
	error code
*/

#define E_ALLOCATION_FAIL			0x80050025L

#ifdef __cplusplus
extern "C" {
#endif

/*
	plot api
*/
HRESULT __stdcall DpPlotData(const char * pName, int type, const char * pBuf, int length);

#ifdef __cplusplus
}
#endif
