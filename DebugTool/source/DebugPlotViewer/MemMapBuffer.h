#pragma once

#include <Windows.h>

class MemMapBuffer
{
public:
	MemMapBuffer();
	virtual ~MemMapBuffer();
	HRESULT Open(int size, wchar_t * filename);
	void SetWindowSize(int windowSize);
	HRESULT Read(void * ptr, int len, int offset);
	HRESULT Write(void * ptr, int len, int offset);

private:
	HRESULT MapView(int offset);

	static const int DEF_WIN_SIZE = 1*1024*1024;
	int windowSize;
	int size;

	HANDLE hFile;
	HANDLE hMapFile;

	static const int NUM_VIEW = 3;
	HANDLE hMapView[NUM_VIEW];
	LPVOID baseAddr[NUM_VIEW];
	int offset[NUM_VIEW];
	int curViewIdx;

	HRESULT ReadWrite(void * ptr, int len, int offset, bool isWrite);
	void UpdateViewOrder(int viewIdx);
};

