#pragma once

template <typename T>
class Average
{
public:
	Average();
	void Add(T data);
	void Clear();
	T GetAverage();
	int GetCount();
private:
	int count;
	T sum;
};

template <typename T>
class FIFO_Average
{
public:
	FIFO_Average(int size);
	~FIFO_Average();
	void Add(T data);
	void Clear();
	T GetAverage();
	int GetCount();
	bool IsFull();
private:
	T * histBuf;
	T * writePointer;
	int bufSize;
	int count;
	T sum;
};

#include "Average.h"
#include <stdio.h>
#include <string.h>

template <typename T>
Average<T>::Average()
{
	Clear();
}

template <typename T>
void Average<T>::Clear()
{
	count = 0;
	sum = (T)0;
}

template <typename T>
void Average<T>::Add(T data)
{
	count++;
	sum += data;
}

template <typename T>
T Average<T>::GetAverage()
{
	return sum / count;
}

template <typename T>
int Average<T>::GetCount()
{
	return count;
}


template <typename T>
FIFO_Average<T>::FIFO_Average(int count)
{
	bufSize = count;
	histBuf = new T[bufSize];
	Clear();
}

template <typename T>
FIFO_Average<T>::~FIFO_Average()
{
	delete [] histBuf;
}

template <typename T>
void FIFO_Average<T>::Clear()
{
	count = 0;
	writePointer = histBuf;
	sum = (T)0;
	memset(histBuf, 0, sizeof(T)*bufSize);
}

template <typename T>
void FIFO_Average<T>::Add(T data)
{
	T oldestData = *writePointer;
	
	sum += data;
	sum -= oldestData;

	if (count < bufSize)
		count++;

	*writePointer = data;

	writePointer++;
	if (writePointer - histBuf == bufSize)
		writePointer = histBuf;
}

template <typename T>
T FIFO_Average<T>::GetAverage()
{
	return sum/count;
}

template <typename T>
int FIFO_Average<T>::GetCount()
{
	return count;
}

template <typename T>
bool FIFO_Average<T>::IsFull()
{
	return count == bufSize;
}
