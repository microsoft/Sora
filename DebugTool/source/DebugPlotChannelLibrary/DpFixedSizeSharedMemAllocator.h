#pragma once

#include <Windows.h>

struct AllocatorImpl;

class DpFixedSizeSharedMemAllocator
{
public:
	static DpFixedSizeSharedMemAllocator * Create(const wchar_t * name, int blockSize, int count);
	static void Release(DpFixedSizeSharedMemAllocator *);

public:
	~DpFixedSizeSharedMemAllocator();
	bool Allocate(void ** buf, unsigned count);
	void Free(void * rawptr);
	void * Ptr(int offset);
	int Offset(void * ptr);

	void PrepareRollBack();
	void RollBack();

	int BlockDataCapacity();

	int FreeCount();

private:
	DpFixedSizeSharedMemAllocator(int blockSize, int count);
	AllocatorImpl * _allocator;
};
