#include <assert.h>
#include "FrameWithSizeFilter.h"
#include "Writable.h"

using namespace SoraDbgPlot;

class TestWritable : public Writable
{
public:
	TestWritable(size_t count, size_t maxSize)
	{
		_count = count;
		_maxSize = maxSize;

		_bufferToCompare = new char[_count*_maxSize];
		_bufferWithSize = new char[_count*(_maxSize+sizeof(size_t))];
		_bufferTarget = new char[_count*_maxSize];
	}

	~TestWritable()
	{
		delete [] _bufferToCompare;
		delete [] _bufferWithSize;
		delete [] _bufferTarget;
	}

	bool RunTest()
	{
		
		_bufferToCompareP = _bufferToCompare;
		_bufferWithSizeP = _bufferWithSize;
		_bufferTargetP = _bufferTarget;

		for (size_t i = 0; i < _count; i++)
		{
			size_t sizeThisRound = rand() % _maxSize;
			sizeThisRound = max(1, sizeThisRound);

			*((size_t*)_bufferWithSizeP) = sizeThisRound;
			_bufferWithSizeP += sizeof(size_t);
			for (size_t j = 0; j < sizeThisRound; j++)
			{
				*_bufferWithSizeP++ = j % 26 + 'a';
				*_bufferToCompareP++ = j % 26 + 'a';
			}
		}

		FrameWithSizeInfoWriter filter(this);
		size_t sizeWritten;
		filter.Write(_bufferWithSize, _bufferWithSizeP - _bufferWithSize, sizeWritten);

		_size = _bufferToCompareP - _bufferToCompare;
		for (size_t i = 0; i < _size; i++)
		{
			if (_bufferToCompare[i] != _bufferTarget[i])
				return false;
		}
		return true;
	}

	void Write(const void * ptr, size_t size, size_t & sizeWritten)
	{
		memcpy(_bufferTargetP, ptr, size);
		_bufferTargetP += size;
		sizeWritten = size;
		_size += size;
	}

private:
	char * _bufferToCompare;
	char * _bufferToCompareP;

	char * _bufferWithSize;
	char * _bufferWithSizeP;

	char * _bufferTarget;
	char * _bufferTargetP;

	size_t _size;

	size_t _count;
	size_t _maxSize;
};

static int OneRound(size_t count, size_t maxSize)
{
	assert(TestWritable(count, maxSize).RunTest());
	return 0;
}

int TestDataFilter()
{
	return OneRound(160, 160);
}
