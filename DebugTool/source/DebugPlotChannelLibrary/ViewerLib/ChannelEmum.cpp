#include "ChannelEnum.h"
#include "ShareChannelManager.h"

static bool ChannelEnumCallbackFunc(ChannelInfo2 * channelInfo, void * userData)
{
	ChannelRecord *** nextPtr = (ChannelRecord ***)userData;
	ChannelRecord * record = new ChannelRecord;
	**nextPtr = record;

	record->next = 0;
	record->channelInfo.Pid = channelInfo->pidPloter;
	record->channelInfo.Type = channelInfo->type;
	strcpy_s(record->channelInfo.Name, channelInfo->name);
	
	*nextPtr = &record->next;

	return true;
}

ChannelRecord * FindAll()
{
	ChannelRecord * recordHead = 0;
	ChannelRecord ** nextPtr = &recordHead;
	ShareChannelManager::Instance()->Enumerate(&ChannelEnumCallbackFunc, &nextPtr);
	return recordHead;
}

void FreeList(ChannelRecord * record)
{
	ChannelRecord * current = record;
	while(current != 0)
	{
		ChannelRecord * next =  current->next;
		delete current;
		current = next;
	}
}
