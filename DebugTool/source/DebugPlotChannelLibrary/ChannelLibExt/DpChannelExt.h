#pragma once

#include <Windows.h>
#include "sora.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	plot api
*/

HRESULT __stdcall DpPlotLine(const char * pName, const int * pBuf, int length);

HRESULT __stdcall DpPlotDots(const char * pName, const COMPLEX16 * pBuf, int length);

HRESULT __stdcall DpPlotSpectrum(const char * pName, const int * pBuf, int length);

HRESULT __stdcall DpPlotText(const char * pName, const char * format, ...);

#ifdef __cplusplus
}
#endif
