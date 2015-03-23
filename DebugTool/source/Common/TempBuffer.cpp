#include "TempBuffer.h"

using namespace SoraDbgPlot::Buffer;

static const int DEFAULT_KEPT_SIZE = 4*1024;

TempBuffer::TempBuffer()
{
	_buf = 0;
	_size = 0;
	_sizeKept = DEFAULT_KEPT_SIZE;
}

TempBuffer::~TempBuffer()
{
	if (_buf)
		delete [] _buf;
}

void TempBuffer::ConfigKeptSize(int sizeKept)
{
	_sizeKept = sizeKept;
}

char * TempBuffer::UseBuf(int size)
{
	if (size <= _size)
	{
		return _buf;
	}
	else
	{
		if (_size > 0)
		{
			delete [] _buf;
			_size = 0;
		}
		_buf = new char[size];
		_size = size;
	}

	return _buf;
}

void TempBuffer::ReturnBuf()
{
	if (_size > _sizeKept)
	{
		delete [] _buf;
		_size = 0;
	}
}
