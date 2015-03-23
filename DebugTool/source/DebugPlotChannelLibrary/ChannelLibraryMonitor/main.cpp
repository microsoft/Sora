#include <Windows.h>
#include <stdio.h>

#include "ShareChannelManager.h"

struct Status
{
	int freeBlockCount;
};

void Update(Status * status)
{
	status->freeBlockCount = ShareChannelManager::Instance()->FreeBlockCount();
}

#define clear_line() printf("                                                                           \r");

void Print(Status *status)
{
	//CONSOLE_SCREEN_BUFFER_INFO info;
	//GetConsoleScreenBufferInfo ( GetStdHandle (STD_OUTPUT_HANDLE), &info );

	//COORD cord;
	//cord.X = info.srWindow.Left; cord.Y = info.srWindow.Top;
	//SetConsoleCursorPosition ( GetStdHandle (STD_OUTPUT_HANDLE), cord );

	// first line - separator
	//printf("+------------------------------------------------------------------------+\n" );
	//clear_line();
	printf("%8d  ", status->freeBlockCount);
	for (int i = 0; i < status->freeBlockCount / 200; ++i)
	{
		printf("*");
	}
	printf("\n");

	//printf("free block count: %d", status->freeBlockCount);
}

int main()
{
	Status status;
	while(1)
	{
		::Sleep(1);
		Update(&status);
		Print(&status);
	}
}
