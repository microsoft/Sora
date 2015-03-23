#include "ErrMsg.h"

#include <Windows.h>
#include <stdarg.h>

using namespace SoraDbgPlot::Msg;

const int Message::MAX_MSG_SIZE = 1024;

Message::Message()
{
	_tmpBuf = new wchar_t[MAX_MSG_SIZE];
	Reset();
}

Message::~Message()
{
	if (_tmpBuf)
		delete [] _tmpBuf;
}

void Message::Append(const wchar_t * format, ...)
{
	va_list ap;
	va_start(ap, format);
	vswprintf_s(_tmpBuf, MAX_MSG_SIZE, format, ap);
	va_end(ap);	

	_tmpBuf[MAX_MSG_SIZE - 1] = 0;

	wchar_t * ptr = _tmpBuf;
	wchar_t c;
	while( (c = *ptr++) != 0 )
	{
		_msg.push_back(c);
	}
}

void Message::Reset()
{
	_msg.clear();
}

Message::operator const wchar_t *()
{
	if(_msg.size() == 0)
	{
		_msg.push_back(0);
	}
	else if (_msg[_msg.size()-1] != 0)
	{
		_msg.push_back(0);
	}

	return &_msg[0];
}
