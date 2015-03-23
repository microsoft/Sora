#include "Config.h"

int Config::ALLOC_BLOCK_COUNT()
{
	return 4 * 1024;
}

int Config::ALLOC_BLOCK_SIZE()
{
	return 16*1024 + sizeof(int);
}

int Config::CHANNEL_BUFFER_LIMIT_INITIAL_VALUE()
{
	return 16*1024;
}
