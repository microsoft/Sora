#pragma once

struct HandleTableImpl;

class HandleTable
{
public:
	HandleTable(int maxSize);
	~HandleTable();
	int Put(void * ptr);
	void * Get(int handle);
	void Free(int handle);
private:
	HandleTableImpl * _impl;
};
