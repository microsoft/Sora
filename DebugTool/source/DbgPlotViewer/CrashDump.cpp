#include "stdafx.h"
#include <DbgHelp.h>
#include "PathName.h"

#define ATOW2(x) L##x
#define ATOW(x) ATOW2(x)

#define __WDATE__ ATOW(__DATE__)
#define __WTIME__ ATOW(__TIME__)

const wchar_t * buildString = __WDATE__ __WTIME__;

static LONG WINAPI AppUnhandledExceptionFilter(
  _In_  struct _EXCEPTION_POINTERS *ExceptionInfo
)
{
	CString errMsg;

	DWORD requiredSize = GetCurrentDirectory(0, NULL);
	if (requiredSize == 0)	// error, just exit
	{
		errMsg.Format(L"DbgPlot Viewer has crashed.\n");
		::AfxMessageBox(errMsg);
		::OutputDebugString(L"DbgPlot Crash dump: error in querying directory 1\n");
		return EXCEPTION_CONTINUE_SEARCH;
	}

	wchar_t * currentPathBuffer = new wchar_t[requiredSize];
	DWORD pathLen = GetCurrentDirectory(requiredSize, currentPathBuffer);
	if (pathLen != requiredSize - 1)	// error
	{
		delete [] currentPathBuffer;
		errMsg.Format(L"DbgPlot Viewer has crashed.\n");
		::AfxMessageBox(errMsg);
		::OutputDebugString(L"DbgPlot Crash dump: error in querying directory 2\n");
		return EXCEPTION_CONTINUE_SEARCH;
	}
	
	PathName pathName;
	pathName.Append(currentPathBuffer);
	delete [] currentPathBuffer;
	pathName.Append(L"\\");

	pathName.AppendWithEscape(buildString, '_');
	FILETIME fileTime;
	::GetSystemTimeAsFileTime(&fileTime);
	CString time;
	time.Format(L"{%d%d}", fileTime.dwHighDateTime, fileTime.dwLowDateTime);
	pathName.AppendWithEscape(time, '_');
	pathName.Append(L".dmp");

	HANDLE hFile = CreateFileW(
		(const wchar_t *)pathName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		errMsg.Format(L"DbgPlot Viewer has crashed. Cannot create crash dump file in %s\n", (const wchar_t *)pathName);
		::AfxMessageBox(errMsg);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	BOOL succ = ::MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MiniDumpNormal,
		NULL,
		NULL,
		NULL);

	if (succ == TRUE)
	{
		errMsg.Format(L"DebugPlot Viewer has crashed. A crash dump file is saved at %s\n", (const wchar_t *)pathName);
		::AfxMessageBox(errMsg);
	}
	else
	{
		errMsg.Format(L"DebugPlot Viewer has crashed. Error happened in writing dump file to %s\n", (const wchar_t *)pathName);
		::AfxMessageBox(errMsg);
	}

	CloseHandle(hFile);
	
	return EXCEPTION_CONTINUE_SEARCH;
}


void SetCrashDump()
{
	::SetUnhandledExceptionFilter(AppUnhandledExceptionFilter);
}
