#pragma once

#include <windows.h>
#include <WinUser.h>
#include <CommCtrl.h>
#include <Windowsx.h>
#include <wchar.h>
#include <Richedit.h>
#include "Logger.h"

#include "Message.h"

class DlgLogger : public LogMethod
{
public:
	DlgLogger(HWND hwnd);
	void Log(int type, const wchar_t * string);
private:
	HWND hwnd;
};

