#pragma once

#include <stdio.h>
#include <string.h>

template <typename ValueType>
class SimpleVector
{
public:
	SimpleVector();
	~SimpleVector();
	void Append(ValueType value);
	void Foreach(bool (*VisitorFunc)(int index, ValueType value));
private:
	void Realloc();

	static const int DEFAULT_SIZE = 64;
	ValueType * _buffer;
	int _size;
	int _index;
};

template <typename ValueType>
SimpleVector<ValueType>::SimpleVector()
{
	_size = DEFAULT_SIZE;
	_buffer = new ValueType[_size];
	_index = 0;
}

template <typename ValueType>
SimpleVector<ValueType>::~SimpleVector()
{
	delete [] _buffer;
}

template <typename ValueType>
void SimpleVector<ValueType>::Append(ValueType value)
{
	if (_index == _size)
		Realloc();

	_buffer[_index++] = value;
}

template <typename ValueType>
void SimpleVector<ValueType>::Realloc()
{
	_size *= 2;
	ValueType * newBuf = new ValueType[_size];
	if (_index > 0)
		memcpy(newBuf, _buffer, _index*sizeof(ValueType));
	delete [] _buffer;
	_buffer = newBuf;
}

template <typename ValueType>
void SimpleVector<ValueType>::Foreach(bool (*VisitorFunc)(int index, ValueType value))
{
	for (int i = 0; i < _index; i++)
	{
		bool continueLoop = VisitorFunc(i, _buffer[i]);
		if (!continueLoop)
			break;
	}
}
