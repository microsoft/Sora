#pragma once

#include <Windows.h>
#include <functional>
#include <vector>

namespace SoraDbgPlot { namespace DataQueue {

	class DataQueue
	{
	public:
		struct DataItem
		{
			char * ptr;
			size_t length;
		};

		DataQueue(size_t size)
		{
			::InitializeCriticalSection(&_cs);
			_index = 0;
			_size = 0;
			_list = new DataItem[size];
			for (size_t i = 0; i < size; i++)
			{
				_list[i].ptr = 0;
				_list[i].length = 0;
			}
			_capacity = size;
		}

		~DataQueue()
		{
			Clear();
			delete [] _list;
			::DeleteCriticalSection(&_cs);
		}

		void Clear()
		{
			this->Read([this](const char * ptr, size_t length){
				delete [] ptr;
			});
		}

		void Write(const char * ptr, size_t length)
		{
			Lock();

			if (_size < _capacity)
			{
				_index = (_index + 1) % _capacity;
				_size = min(_size + 1, _capacity);

				DataItem & item = _list[_index];

				item.ptr = new char[length];
				item.length = length;
				memcpy(item.ptr, ptr, length);
			}

			Unlock();
		}

		void Read(const std::function<void(const char * ptr, const size_t length)> & f)
		{
			Lock();

			size_t start = _index - _size + 1;
			while(_size > 0)
			{
				DataItem & item = _list[start % _capacity];
				ASSERT(item.ptr);
				f(item.ptr, item.length);
				//delete [] item.ptr;
				//item.ptr = 0;
				//item.length = 0;
				start++;
				_size--;
			}

			Unlock();
		}

	private:
		void Lock() {
			::EnterCriticalSection(&_cs);
		}

		void Unlock()
		{
			::LeaveCriticalSection(&_cs);
		}

		DataItem * _list;
		size_t _index;
		size_t _size;
		size_t _capacity;
		CRITICAL_SECTION _cs;
	};

}}
