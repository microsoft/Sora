#include "IChannelBuffer.h"
#include "ChannelBufferImpl.h"

#define MY_NAME L"fjslafhlshglsajf0983093850-32570935780932"
#define MY_SIZE 1024*1024
#define BLOCK_SIZE 128
#define DATA_SIZE (512)

bool __stdcall AProcessFunc(const char * buffer, int size, void * userData, __int32 userId)
{
	for (int i = 0; i < size; i++)
	{
		printf("%c\n", buffer[i]);
	}

	return true;
}

#define MAX_NUM 1024
char dataForTest[MAX_NUM];

void PrepareData()
{
	for (int i = 0; i < MAX_NUM; i++)
	{
		dataForTest[i] = rand() % 256;
	}
}
//
//struct TaskInfo
//{
//	int id;
//};

//DWORD WINAPI ThreadProc(__in  LPVOID lpParameter)
//{
//	TaskInfo * info = (TaskInfo *)lpParameter;
//
//	IChannelBufferWritable * writable = ChannelBuffer::OpenForWrite(MY_NAME, MY_SIZE/BLOCK_SIZE, BLOCK_SIZE, 0);
//	for (int i = 0; i < MAX_NUM; i++)
//	{
//		writable->WriteData(dataForTest, i);
//	}
//
//	return 0;
//}

int TestChannelBuffer()
{
	_int32 i = ChannelBuffer::SIZE;
	IChannelBufferWritable * writable = ChannelBuffer::OpenForWrite(MY_NAME, MY_SIZE/BLOCK_SIZE, BLOCK_SIZE, 0);
	
	for (int i = 0; i < DATA_SIZE; i++)
	{
		char c = i % 26 + 'a';
		writable->WriteData(&c, 1);
	}

	IChannelBufferReadable * readable = ChannelBuffer::OpenForRead(MY_NAME, MY_SIZE/BLOCK_SIZE, BLOCK_SIZE);
	readable->ReadData(AProcessFunc, 0);

	delete writable;
	delete readable;


	return 0;
}
