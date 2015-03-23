#pragma once

#define DBG_SUCC() DBG(L"success.\n")
#define DBG_FAIL() DBG(L"failed.\n")


inline void DBG(const wchar_t * format, ...)
{
	wchar_t buffer[256];

	va_list ap;
	va_start(ap, format);
	_vsnwprintf_s(buffer, 256, _TRUNCATE, format, ap);
	va_end(ap);	

	OutputDebugString(buffer);
}

inline void Crash()
{
	char * p = (char *)0;
	*p = 'c';
}

inline void HighlightWnd(HWND hwnd)
{
	RECT rect;
	::GetWindowRect(hwnd, &rect);
	HDC hDC = ::GetWindowDC(hwnd);
	CPen pen(0, 5, RGB(200, 0, 0));
	CPen * oldPen = (CPen *)::SelectObject(hDC, &pen);
	::Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
	::SelectObject(hDC, oldPen);
}

