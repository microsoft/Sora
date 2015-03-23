#include <Windows.h>
#include "SharedNameManagement.h"

#define _CRT_NON_CONFORMING_SWPRINTFS

#define NAME_PREFIX L"/Sora/DbgPlot/"
#define CHANNEL_NAME_PREFIX NAME_PREFIX L"channel/"
#define PROCESS_NAME_PREFIX NAME_PREFIX L"process/"
#define SOURCE_NAME_PREFIX NAME_PREFIX L"source/"
#define EVENT_NAME_PREFIX NAME_PREFIX L"event/"
#define RAWDATA_EVENT_NAME_PREFIX NAME_PREFIX L"rawdataevent/"
#define SINGLETON_NAME_PREFIX NAME_PREFIX L"singleton/"
#define NAME_SERIAL_NUM_GENERATOR SINGLETON_NAME_PREFIX L"serialNumGenerator"

bool __stdcall SharedNameManager::GenChannelName(int id, wchar_t * name, size_t & bufSize)
{
	size_t prefixLen = wcslen(CHANNEL_NAME_PREFIX);
	size_t numLen = 10;
	size_t requiredLen = prefixLen + numLen + 1;
	if (bufSize < requiredLen)
	{
		bufSize = requiredLen;
		return false;
	}
	else
	{
		swprintf(name, L"%s%d", CHANNEL_NAME_PREFIX, id);
		return true;
	}
}

const wchar_t * __stdcall SharedNameManager::GetSerialNumGeneratorName()
{
	return NAME_SERIAL_NUM_GENERATOR;
}

bool __stdcall SharedNameManager::GenProcessName(int pid, wchar_t * name, size_t & bufSize)
{
	size_t prefixLen = wcslen(PROCESS_NAME_PREFIX);
	size_t numLen = 10;
	size_t requiredLen = prefixLen + numLen + 1;
	if (bufSize < requiredLen)
	{
		bufSize = requiredLen;
		return false;
	}
	else
	{
		swprintf(name, L"%s%d", PROCESS_NAME_PREFIX, pid);
		return true;
	}
}

bool _stdcall SharedNameManager::GenSourceName(int pid, wchar_t * name, size_t & bufSize)
{
	size_t prefixLen = wcslen(SOURCE_NAME_PREFIX);
	size_t numLen = 10;
	size_t requiredLen = prefixLen + numLen + 1;
	if (bufSize < requiredLen)
	{
		bufSize = requiredLen;
		return false;
	}
	else
	{
		swprintf(name, L"%s%d", SOURCE_NAME_PREFIX, pid);
		return true;
	}
}

wchar_t * _stdcall SharedNameManager::GenEventName()
{
	size_t prefixLen = wcslen(EVENT_NAME_PREFIX);
	size_t numLen = 10;
	size_t requiredLen = prefixLen + numLen + 1;
	wchar_t * name = new wchar_t[requiredLen];

	swprintf(name, L"%s", EVENT_NAME_PREFIX);
	return name;
}

wchar_t * _stdcall SharedNameManager::GenRawDataEventName(int pid)
{
	size_t prefixLen = wcslen(RAWDATA_EVENT_NAME_PREFIX);
	size_t numLen = 10;
	size_t requiredLen = prefixLen + numLen + 1;
	wchar_t * name = new wchar_t[requiredLen];

	swprintf(name, L"%s%d", RAWDATA_EVENT_NAME_PREFIX, pid);
	return name;
}
