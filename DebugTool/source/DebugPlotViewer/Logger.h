#pragma once

#include "OutputWnd.h"

class Logger
{
public:
	static void Print(wchar_t * format, ...);
	static COutputWnd * wnd;
};

