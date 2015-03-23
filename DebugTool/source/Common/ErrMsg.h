#pragma once

#include <wchar.h>
#include <vector>
#include <functional>

namespace SoraDbgPlot { namespace Msg {

class Message
{
public:
	Message();
	~Message();
	void Append(const wchar_t * format, ...);
	void Reset();
	operator const wchar_t *();
private:
	std::vector<wchar_t> _msg;
	static const int MAX_MSG_SIZE;
	wchar_t * _tmpBuf;
};

}}
