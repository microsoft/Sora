#include <Windows.h>
#include <stdio.h>

extern int TestSharedObj();

int __cdecl main()
{
	TestSharedObj();

	printf("Test completed\n");
	getchar();
	return 0;
}
