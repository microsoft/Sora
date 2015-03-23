#pragma once

#include <wchar.h>

class SharedNameManager
{
public:
	static bool __stdcall GenChannelName(int id, wchar_t * name, size_t & bufSize);
	static const wchar_t * __stdcall GetSerialNumGeneratorName();
	static bool __stdcall GenProcessName(int pid, wchar_t * name, size_t & bufSize);
	static bool _stdcall GenSourceName(int pid, wchar_t * name, size_t & bufSize);
	static wchar_t * _stdcall GenEventName();
	static wchar_t * _stdcall GenRawDataEventName(int pid);
};
