#pragma once

#include <assert.h>
#include <vector>

namespace SoraDbgPlot
{
	class DynamicArray	// a slow implementation
	{
	public:
		DynamicArray()
		{
			_capacity = 4096;
			_buffer = new char[_capacity];
			_wPtr = _buffer;
		}

		~DynamicArray()
		{
			delete [] _buffer;
		}


		void Write(const void * ptr, size_t size, size_t & countWritten)
		{
			size_t sizeUsable = SizeUsable();
			if (size > sizeUsable)
			{
				size_t sizeRequired = (size - sizeUsable) + _capacity;
				size_t sizeReallocate = 1;
				while(sizeReallocate < sizeRequired)
					sizeReallocate *= 2;
				Reallocate(sizeReallocate);
				assert(SizeUsable() >= size);
			}

			memcpy(_wPtr, ptr, size);
			_wPtr += size;
			countWritten = size;
		}

		void Reserve(size_t count)
		{
			Reallocate(count);
		}

		void Reset()
		{
			_wPtr = _buffer;
		}

		size_t Size()
		{
			return _wPtr - _buffer;
		}

		const void * Ptr()
		{
			return _buffer;
		}

	private:
		size_t SizeUsable()
		{
			return _capacity - (_wPtr - _buffer);
		}

		void Reallocate(size_t size)
		{
			if (size <= _capacity)
				return;

			char * newBuffer = new char[size];
			size_t sizeToCopy = _wPtr - _buffer;
			if (sizeToCopy > 0)
				memcpy(newBuffer, _buffer, sizeToCopy);
			delete [] _buffer;
			_buffer = newBuffer;
			_capacity = size;
			_wPtr = _buffer + sizeToCopy;
		}

		char * _buffer;
		size_t _capacity;
		char * _wPtr;
	};
}
