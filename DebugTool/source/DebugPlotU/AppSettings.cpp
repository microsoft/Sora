#include <Windows.h>
#include <stdio.h>
#include "AppSettings.h"
#include "sora.h"

#define DEFAULT_SOURCE_BUFFER_SIZE	(16*1024*1024)	// 16M bytes
#define DEFAULT_SNAPSHOT_BUFFER_SIZE (4*1024)		// 4k bytes

// Shared Buffer
#define GLOBAL_BUFFER_NAME L"Sora DbgView Global Buffer"

#ifdef USE_DEBUG_DATA_SIZE
#define GLOBAL_BUFFER_BLOCK_SIZE (4*1024)
#define GLOBAL_BUFFER_BLOCK_NUM (64)

#define GLOBAL_BUFFER_THREADHOLD_LOW (64*1024)
#define GLOBAL_BUFFER_THREADHOLD_HIGH (128*1024)

#else
#define GLOBAL_BUFFER_BLOCK_SIZE (4*1024)
#define GLOBAL_BUFFER_BLOCK_NUM (16*1024)

#define GLOBAL_BUFFER_THREADHOLD_LOW (16*1024*1024)
#define GLOBAL_BUFFER_THREADHOLD_HIGH (32*1024*1024)
#endif

// replay buffer
#define REPLAY_BUFFER_SIZE (4*1024*1024)

// speed
#define REPLAY_SPEED_MAX (1*1024*1024)

int SettingGetSourceBufferSize()
{
	return DEFAULT_SOURCE_BUFFER_SIZE;
}

int SettingGetSourceBufferComplex16Count()
{
	return ::SettingGetSourceBufferSize() / sizeof(COMPLEX16);
}

const wchar_t * SettingGetGlobalBufferName()
{
	return GLOBAL_BUFFER_NAME;
}

int SettingGetGlobalBufferBlockSize()
{
	return GLOBAL_BUFFER_BLOCK_SIZE;
}

int SettingGetGlobalBufferBlockNum()
{
	return GLOBAL_BUFFER_BLOCK_NUM;
}

int SettingGetGlobalBufferThreadHoldLow()
{
	return GLOBAL_BUFFER_THREADHOLD_LOW;
}

int SettingGetGlobalBufferThreadHoldHigh()
{
	return GLOBAL_BUFFER_THREADHOLD_HIGH;
}

int SettingGetReplayBufferSize()
{
	return REPLAY_BUFFER_SIZE;
}

int SettingGetMaxSpeed()
{
	return REPLAY_SPEED_MAX;
}
