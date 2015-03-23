#include "stdafx.h"
#include "sora.h"
#include "AppSettings.h"

size_t SettingDotMaxDataCount()
{
	return 4096;
}

size_t SettingLineTypeMaxDataCount()
{
	return ::SettingGetReplayBufferSize() / sizeof(_int32);
}
