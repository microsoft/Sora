#include "LoggerImpl.h"


DlgLogger::DlgLogger(HWND hwnd)
{
	this->hwnd = hwnd;
}

void DlgLogger::Log(int type, const wchar_t * string)
{
	if (hwnd)
	{
		int len = wcslen(string);
		wchar_t * newstr = new wchar_t[len+1];
		wcscpy_s(newstr, len+1, string);

		::PostMessage(hwnd, WM_LOGGER, (WPARAM)type, (LPARAM)newstr);		
	}
	else
		return;
}
