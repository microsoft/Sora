#pragma once


#include <Windows.h>
#include "sora.h"

// error code
#define E_ALLOCATION_FAIL			0x80050025L
#define E_PLOT_TYPE_MISMATCH		0x80050026L
#define E_PLOT_THREAD_MISMATCH		0x80050027L	
#define E_END_OF_BUF				0x80050028L
#define E_SPECTRUM_SIZE_INVALID		0x80050029L	

#ifdef __cplusplus
extern "C" {
#endif


HRESULT __stdcall DebugPlotInit();

void __stdcall DebugPlotDeinit();

DWORD __stdcall WaitForViewer(
	__in DWORD milliseconds
);

HRESULT __stdcall TracebufferWriteData(
	__in COMPLEX16 * pData,
	__in int inNum,
	__out int * pOutNum
);

HRESULT __stdcall TracebufferReadData(
	__in COMPLEX16 * pData,
	__in int inNum,
	__out int * pOutNum
);

void __stdcall TracebufferClear();

void __stdcall PauseViewer();

HRESULT __stdcall PlotLine(
	__in const char * channelName,
	__in int * data,
	__in int dataCount
);

HRESULT __stdcall PlotSpectrum(
	__in const char * channelName,
	__in int * data,
	__in int dataCount
);


HRESULT __stdcall PlotDots(
	__in const char * channelName,
	__in COMPLEX16 * pData,
	__in int dataCount
);

HRESULT __stdcall PlotText(
	__in const char * channelName,
	__in const char * format,
	...
);

HRESULT __stdcall Log(
	__in const char * channelName,
	__in const char * format,
	...
);

#ifdef __cplusplus
}
#endif
