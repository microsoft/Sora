#include <assert.h>
#include "HashTable.h"
#include <cstring>

unsigned long HashCodeString(const char * str)
{
	int hashVal = 5381;
	int c;

	while (c = *str++)
		hashVal = ((hashVal << 5) + hashVal) + c;

	return hashVal;
}

bool EqualString(const char * s1, const char * s2)
{
	return strcmp(s1, s2) == 0;
}

unsigned long HashCodeInt(int n)
{
	return n;
}

bool EqualInt(int n1, int n2)
{
	return n1 == n2;
}

void TestHashTable()
{
	SimpleHashTable<const char *, int, 1, HashCodeString, EqualString> hashTableStr;
	HashEntry<const char *, int> * pHashEntry;

	hashTableStr.Insert("123", 123);
	pHashEntry = hashTableStr.Find("123");
	assert(pHashEntry);
	assert(pHashEntry->value == 123);
	
	hashTableStr.Insert("abc", 456);
	pHashEntry = hashTableStr.Find("abc");
	assert(pHashEntry);
	assert(pHashEntry->value == 456);

	hashTableStr.Insert("abc1", 789);
	pHashEntry = hashTableStr.Find("abc1");
	assert(pHashEntry);
	assert(pHashEntry->value == 789);

	pHashEntry = hashTableStr.Find("non-exist");
	assert(pHashEntry == 0);
}
