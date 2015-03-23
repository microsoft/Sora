#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>
#include "Debug.h"

void DBG(const wchar_t * format, ...)
{
	wchar_t buffer[256];

	va_list ap;
	va_start(ap, format);
	_vsnwprintf_s(buffer, 256, _TRUNCATE, format, ap);
	va_end(ap);	

	OutputDebugString(buffer);
}

void Crash()
{
	char * p = (char *)0;
	*p = 'c';
}
