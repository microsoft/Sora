#include "SimpleVector.h"
#include <assert.h>

bool PrintElement(int index, int value)
{
	printf("%d\t%d\n", index, value);
	assert(index == value);
	return true;
}

void TestVector()
{
	SimpleVector<int> vec;
	for (int i = 0; i < 1024; i++)
		vec.Append(i);
	vec.Foreach(PrintElement);
}
