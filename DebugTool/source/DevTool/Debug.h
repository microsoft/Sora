#pragma once

#define DBG_SUCC() DBG(L"success.\n")
#define DBG_FAIL() DBG(L"failed.\n")


void __stdcall DBG(const wchar_t * format, ...);
void __stdcall Crash();

