#include "HandleTable.h"

struct HandleTableImpl
{
	void **ptrTable;
	int maxSize;
};

HandleTable::HandleTable(int maxSize)
{
	_impl = new HandleTableImpl();
	_impl->maxSize = maxSize;
	_impl->ptrTable = new void*[maxSize];

	for (int i = 0; i < _impl->maxSize; ++i)
	{
		_impl->ptrTable[i] = 0;
	}
}

HandleTable::~HandleTable()
{
	delete [] _impl->ptrTable;
	delete _impl;
}

int HandleTable::Put(void * ptr)
{
	for (int i = 0; i < _impl->maxSize; ++i)
	{
		if (_impl->ptrTable[i] == 0)
		{
			_impl->ptrTable[i] = ptr;
			return i;
		}
	}

	return -1;
}

void * HandleTable::Get(int handle)
{
	return _impl->ptrTable[handle];
}

void HandleTable::Free(int handle)
{
	_impl->ptrTable[handle] = 0;
}
