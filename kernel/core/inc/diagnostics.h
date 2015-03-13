// Header file to interactive with debugger
#pragma once
#include <stdio.h>
#include <stdarg.h>

#ifdef USER_MODE
inline void TraceOutput(const char* fmt, ...)
{
   char buf[1000];
   va_list args;
   va_start(args, fmt);
   vsprintf_s(buf, sizeof(buf)/sizeof(buf[0]), fmt, args);
   va_end(args);

   OutputDebugString(buf);
}
#else
#define TraceOutput DbgPrint
#endif
