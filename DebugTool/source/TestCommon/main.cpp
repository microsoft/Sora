#include <stdio.h>
#include "vld.h"

extern int TestTaskSimple();
extern int TestTaskCoordinator();
extern int TestSelfDelete();

int __cdecl main()
{
	TestSelfDelete();
	printf("Test Completed\n");
	getchar();
	return 0;
}
