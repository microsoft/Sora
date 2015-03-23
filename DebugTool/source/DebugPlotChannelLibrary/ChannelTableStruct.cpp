#include "ChannelTableStruct.h"
#include "RollbackManager.h"

namespace SharedChannelTable
{
	void PreTableOperation(ChannelTableItem * item)
	{
		Lock(&item->spinLockTableItem);
		if (item->dirtyFlagTableItem != 0)
		{
			Rollback_Table(item);
			item->dirtyFlagTableItem = 0;
		}

		PrepareRollback_Table(item);
		item->dirtyFlagTableItem = 1;
	}

	void PostTableOperation(ChannelTableItem * item)
	{
		item->dirtyFlagTableItem = 0;
		Unlock(&item->spinLockTableItem);
	}
}
