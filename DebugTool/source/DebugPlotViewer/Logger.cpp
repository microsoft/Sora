#include "stdafx.h"
#include "Logger.h"

COutputWnd * Logger::wnd = NULL;

void Logger::Print(wchar_t * format, ...)
{
	if (wnd == NULL)
		return;

	CString str;

	va_list ap;
	va_start(ap, format);
	str.FormatV(format, ap);

	va_end(ap);

	wnd->Print(str);
}
