#pragma once

#include <windows.h>

#define E_CHANNEL_CLOSED				0x80050027L
#define E_INVALID_CHANNEL_HANDLE		0x80050031L

#define CHANNEL_BUFFER_SIZE_UNLIMITED	-1

#ifdef __cplusplus
extern "C" {
#endif

/*
	viewer api
*/

HRESULT DbgChannelViewerInit();

void DbgChannelViewerDeinit();

/*
	Channel Discovery
*/

typedef HANDLE SEARCH_HANDLE;
typedef HANDLE CHANNEL_HANDLE;

#define CHANNEL_MAX_NAME 256

struct ChannelInfo
{
	char Name[CHANNEL_MAX_NAME];
	int Type;
	int Pid;
};

SEARCH_HANDLE __stdcall DpFindFirstChannel(ChannelInfo * pChannelInfo);
BOOL __stdcall DpFindNextChannel(SEARCH_HANDLE hSearch, ChannelInfo * pChannelInfo);
void __stdcall DpCloseFindChannel(SEARCH_HANDLE hSearch);

/*
	Channel Data	
*/

CHANNEL_HANDLE __stdcall DpOpenChannel(int pid, const char * pName, int type);
void __stdcall DpCloseChannel(CHANNEL_HANDLE hChannel);
HRESULT __stdcall DpGetChannelReadableSize(CHANNEL_HANDLE hChannel, int * pSize);
HRESULT __stdcall DpReadChannelData(CHANNEL_HANDLE hChannel, char * pBuf, int lenBuf, int * pLenRead);
HRESULT __stdcall DpPeekChannelData(CHANNEL_HANDLE hChannel, char * pBuf, int lenBuf, int * pLenPeeked);
HRESULT __stdcall DpSetChannelBufferLimit(CHANNEL_HANDLE hChannel, int size);

#ifdef __cplusplus
}
#endif
