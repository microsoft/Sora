#pragma once

#include "sora.h"

#define E_NOT_ENOUGH_BUFFER		0x80050028L	

#define CH_UNDEFINED_TYPE	0x80000000
#define CH_LINE_TYPE		0x80000001
#define CH_DOTS_TYPE		0x80000002
#define CH_SPECTRUM_TYPE	0x80000003
#define CH_TEXT_TYPE		0x80000004

#ifdef __cplusplus
extern "C" {
#endif

/*
	plot api
*/
HRESULT __stdcall DpReadLine(CHANNEL_HANDLE hChannel, int * pBuf, int lenBuf, int * pLenRead);

HRESULT __stdcall DpReadDots(CHANNEL_HANDLE hChannel, COMPLEX16 * pBuf, int lenBuf, int * pLenRead);

HRESULT __stdcall DpReadSpectrum(CHANNEL_HANDLE hChannel, int * pBuf, int lenBuf, int * pLenRead, int * pLenRequired);

HRESULT __stdcall DpReadText(CHANNEL_HANDLE hChannel, char * pBuf, int lenBuf, int * pLenRead, int * pLenRequired);

#ifdef __cplusplus
}
#endif
