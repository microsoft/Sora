#include "stdafx.h"
#include "MemMapBuffer.h"

MemMapBuffer::MemMapBuffer()
{
	this->size = 0;
	this->windowSize = DEF_WIN_SIZE;
	this->hFile = INVALID_HANDLE_VALUE;
	this->hMapFile = INVALID_HANDLE_VALUE;
	this->curViewIdx = 0;
	for (int i = 0; i < NUM_VIEW; i++)
	{
		hMapView[i] = INVALID_HANDLE_VALUE;
		baseAddr[i] = 0;
		offset[i] = 0;
	}
}

MemMapBuffer::~MemMapBuffer()
{
	if (hMapFile == INVALID_HANDLE_VALUE)
		return;

	for (int i = 0; i < NUM_VIEW; i++)
	{
		if (baseAddr[i] != 0)
		{
			UnmapViewOfFile(baseAddr[i]);
		}
	}

	CloseHandle(hMapFile);
}

HRESULT MemMapBuffer::Open(int size, wchar_t * filename)
{
	HRESULT hr = S_OK;

	if (filename)
	{
		hFile = ::CreateFileW(
			filename,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD error = GetLastError();
			hr = -1;
			goto RET;
		}
	}

	hMapFile = CreateFileMappingW(hFile,
		NULL,
		PAGE_READWRITE,
		0,
		size,
		NULL);

	if (hMapFile == INVALID_HANDLE_VALUE)
	{
		hr = -1;
		goto CLOSE_FILE;
	}
	else
		this->size = size;

CLOSE_FILE:
	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

RET:
	return hr;
}

HRESULT MemMapBuffer::MapView(int offset)
{
	if ( (offset > this->size ) ||
		(offset < 0) )
	{
		return -1;
	}

	if (hMapFile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	int offsetBase = offset - offset % this->windowSize;
	int mapLen = this->size - offsetBase;
	mapLen = min(mapLen, this->windowSize);

	int mapIdx = NUM_VIEW - 1;

	if (baseAddr[mapIdx] != 0)
		UnmapViewOfFile(baseAddr[mapIdx]);

	baseAddr[mapIdx] = MapViewOfFile(hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		offsetBase,
		mapLen);

	if (baseAddr[mapIdx] != 0)
		return S_OK;
	else
		return -1;
}

HRESULT MemMapBuffer::Write(void * ptr, int len, int offset)
{
	return ReadWrite(ptr, len, offset, 1);
}

HRESULT MemMapBuffer::ReadWrite(void * ptr, int len, int offset, bool isWrite)
{
	HRESULT hr = S_OK;
	bool wDone = false;
	for (int i = 0; i < NUM_VIEW; i++)
	{
		if (baseAddr[i] == 0)
			continue;

		int offsetBase = this->offset[i];

		if ( (offset >= offsetBase) &&
			(offset - offsetBase < this->windowSize) )
		{
			curViewIdx = i;
			int lenLeft = this->windowSize - (offset - offsetBase);
			if (lenLeft > len)
			{
				if (isWrite)
					memcpy(baseAddr[i], ptr, len);
				else
					memcpy(ptr, baseAddr[i], len);

				wDone = true;
			}
			else
			{
				if (isWrite)
					memcpy(baseAddr[i], ptr, lenLeft);
				else
					memcpy(ptr, baseAddr[i], lenLeft);

				hr = ReadWrite((char *)ptr + lenLeft, (len - lenLeft), offset + lenLeft, isWrite);
				wDone = true;
			}
		}
	}

	if (wDone)
		return hr;
	else
	{
		hr = MapView(offset);
		if (SUCCEEDED(hr))
			return ReadWrite(ptr, len, offset, isWrite);
	}
}

HRESULT MemMapBuffer::Read(void * ptr, int len, int offset)
{
	return ReadWrite(ptr, len, offset, false);
}

void MemMapBuffer::UpdateViewOrder(int viewIdx)
{
	if (viewIdx == 0)
		return;

	HANDLE hMapViewTop = hMapView[viewIdx];
	LPVOID baseAddrTop = baseAddr[viewIdx];
	int offsetTop = offset[viewIdx];

	for (int i = 1; i < viewIdx; i++)
	{
		hMapView[i] = hMapView[i-1];
		baseAddr[i] = baseAddr[i-1];
		offset[i] = offset[i-1];
	}

	hMapView[0] = hMapViewTop;
	baseAddr[0] = baseAddrTop;
	offset[0] = offsetTop;
}
