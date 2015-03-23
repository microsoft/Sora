#include <Windows.h>
#include "DpViewer.h"
#include "ChannelEnum.h"
#include "ShareChannelManager.h"
#include "HashTable.h"
#include "ChannelTableStruct.h"
#include "HandleTable.h"

using namespace SharedChannelTable;

static const int MAX_SEARCH = 1024;


struct ListNode
{
	void * current;
	ListNode * next;
	ListNode * prev;
};

struct SearchSession
{
	ChannelRecord * head;
	ChannelRecord * current;
};


struct _ViewerMain
{
	ListNode * searchRecord;

	SM_SpinLock spinLockSearchHandleTable;
	HandleTable searchHandleTable;

	SM_SpinLock spinLockChannelTable;
	HandleTable channelHandleTable;

	_ViewerMain() :		searchHandleTable(MAX_SEARCH),
						channelHandleTable(MAX_CHANNEL)
	{
		Init(&spinLockSearchHandleTable, 5000);
		Init(&spinLockChannelTable, 5000);
	}

	~_ViewerMain()
	{
		ShareChannelManager::Release();
	}
};

static _ViewerMain viewerMain;

void ReleaseSession(SearchSession * session);

//static SearchSession * ToSession(SEARCH_HANDLE handle)
//{
//	return (SearchSession *)handle;
//}
//
//static SEARCH_HANDLE ToHandle(SearchSession * session)
//{
//	return (SEARCH_HANDLE)session;
//}

SEARCH_HANDLE __stdcall  DpFindFirstChannel(::ChannelInfo * pChannelInfo)
{
	ChannelRecord * records = ::FindAll();

	if (records == 0)
	{
		return INVALID_HANDLE_VALUE;
	}

	SearchSession * session = new SearchSession();
	session->head = records;
	session->current = session->head;
	
	if (pChannelInfo != 0)
	{
		pChannelInfo->Pid = session->current->channelInfo.Pid;
		pChannelInfo->Type = session->current->channelInfo.Type;
		strcpy(pChannelInfo->Name, session->current->channelInfo.Name);
	}

	session->current = session->head->next;

	Lock(&viewerMain.spinLockSearchHandleTable);
	int handle = viewerMain.searchHandleTable.Put(session);
	Unlock(&viewerMain.spinLockSearchHandleTable);
	
	if (handle == -1)
	{
		ReleaseSession(session);
		return INVALID_HANDLE_VALUE;
	}

	return (SEARCH_HANDLE)handle;
}

BOOL __stdcall DpFindNextChannel(SEARCH_HANDLE hSearch, ::ChannelInfo * pChannelInfo)
{
	Lock(&viewerMain.spinLockSearchHandleTable);
	SearchSession * session = (SearchSession *)viewerMain.searchHandleTable.Get((int)hSearch);
	Unlock(&viewerMain.spinLockSearchHandleTable);

	if (session == 0)
		return FALSE;

	if (session->current == 0)
	{
		return FALSE;
	}
	
	if (pChannelInfo != 0)
	{
		pChannelInfo->Pid = session->current->channelInfo.Pid;
		pChannelInfo->Type = session->current->channelInfo.Type;
		strcpy(pChannelInfo->Name, session->current->channelInfo.Name);
	}

	session->current = session->current->next;

	return TRUE;
}

static void ReleaseSession(SearchSession * session)
{
	::FreeList(session->head);
	//delete session->head;
	delete session;
}

void __stdcall DpCloseFindChannel(SEARCH_HANDLE hSearch)
{
	Lock(&viewerMain.spinLockSearchHandleTable);
	SearchSession * session = (SearchSession *)viewerMain.searchHandleTable.Get((int)hSearch);
	if (session != 0)
		viewerMain.searchHandleTable.Free((int)hSearch);
	Unlock(&viewerMain.spinLockSearchHandleTable);

	if (session == 0)
		return;

	ReleaseSession(session);
}


/*
	Channel Data	
*/

CHANNEL_HANDLE __stdcall DpOpenChannel(int pid, const char * name, int type)
{
	ShareChannelManager::Instance()->Gc();

	Channel * channel = ShareChannelManager::Instance()->OpenForRead(pid, name, type);

	if (channel == 0)
		return INVALID_HANDLE_VALUE;

	Lock(&viewerMain.spinLockChannelTable);
	int handle = viewerMain.channelHandleTable.Put(channel);
	Unlock(&viewerMain.spinLockChannelTable);	

	if (handle == -1)
	{
		ShareChannelManager::Instance()->CloseForRead(channel);
		return INVALID_HANDLE_VALUE;
	}

	return (CHANNEL_HANDLE)handle;
}

void __stdcall DpCloseChannel(CHANNEL_HANDLE handle)
{
	Lock(&viewerMain.spinLockChannelTable);
	Channel * channel = (Channel *)viewerMain.channelHandleTable.Get((int)handle);
	if (channel != 0)
		viewerMain.channelHandleTable.Free((int)handle);
	Unlock(&viewerMain.spinLockChannelTable);	

	if (channel != 0)
	{
		ShareChannelManager::Instance()->CloseForRead(channel);
	}
}

HRESULT __stdcall DpGetChannelReadableSize(CHANNEL_HANDLE handle, int * pSize)
{
	Lock(&viewerMain.spinLockChannelTable);
	Channel * channel = (Channel *)viewerMain.channelHandleTable.Get((int)handle);
	Unlock(&viewerMain.spinLockChannelTable);

	if (channel == 0)
	{
		if (pSize != 0)
			*pSize = 0;
		return -1;
	}
	
	if (pSize != 0)
	{
		*pSize = channel->Size();
	}

	return S_OK;
}

HRESULT __stdcall DpReadChannelData(CHANNEL_HANDLE handle, char * buf, int lenBuf, int * pLenRead)
{
	Lock(&viewerMain.spinLockChannelTable);
	Channel * channel = (Channel *)viewerMain.channelHandleTable.Get((int)handle);
	Unlock(&viewerMain.spinLockChannelTable);

	if (channel == 0)
	{
		if (pLenRead != 0)
			*pLenRead = 0;
		return E_INVALID_CHANNEL_HANDLE;
	}

	int lenRead = channel->Read(buf, lenBuf);

	if (pLenRead != 0)
		*pLenRead = lenRead;

	if (lenRead < lenBuf)
	{
		if (!channel->IsWriterAlive())
			return E_CHANNEL_CLOSED;
	}

	return S_OK;
}

HRESULT __stdcall DpPeekChannelData(CHANNEL_HANDLE handle, char * buf, int lenBuf, int * pLenPeeked)
{
	Lock(&viewerMain.spinLockChannelTable);
	Channel * channel = (Channel *)viewerMain.channelHandleTable.Get((int)handle);
	Unlock(&viewerMain.spinLockChannelTable);

	if (channel == 0)
	{
		if (pLenPeeked != 0)
		{
			*pLenPeeked = 0;
		}

		return E_INVALID_CHANNEL_HANDLE;
	}

	int lenPeeked = channel->Peek(buf, lenBuf);

	if (pLenPeeked != 0)
	{
		*pLenPeeked = lenPeeked;
	}

	if (lenPeeked < lenBuf)
	{
		if (!channel->IsWriterAlive())
			return E_CHANNEL_CLOSED;
	}

	return S_OK;
}

HRESULT __stdcall DpSetChannelBufferLimit(CHANNEL_HANDLE hChannel, int size)
{
	Lock(&viewerMain.spinLockChannelTable);
	Channel * channel = (Channel *)viewerMain.channelHandleTable.Get((int)hChannel);
	Unlock(&viewerMain.spinLockChannelTable);

	if (channel == 0)
	{
		return E_INVALID_CHANNEL_HANDLE;
	}

	channel->SetBufferLimit(size);

	return S_OK;
}
