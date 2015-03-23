#include "Logger.h"

std::map<std::wstring, Logger *> Logger::loggers;

Logger * Logger::GetLogger(const wchar_t * name)
{
	std::wstring str(name);
	if (loggers[str] == NULL)
		loggers[str] = new Logger();
	return loggers[str];
}

void Logger::ReleaseAll()
{
	std::map<std::wstring, Logger *>::iterator it = loggers.begin(); 
	for(; it!=loggers.end(); ++it) 
	{
		//it->first;
		delete it->second;	
	}
}

HRESULT Logger::SetLogMethod(LogMethod * method, bool autoDelete)
{
	if (implemented)
		return -1;
	else
	{
		if (method != 0)
		{
			this->method = method;
			this->autoDelete = autoDelete;
			implemented = true;
			return S_OK;
		}
		else
			return -1;
	}
}

Logger::Logger()
{
	implemented = false;
	method = 0;
	autoDelete = false;
	enabled = true;
}

Logger::~Logger()
{
	if (method && autoDelete)
	{
		delete method;
	}
}

void __cdecl Logger::Log(int type, const wchar_t * format, ...)
{
	if (implemented && enabled)
	{
		wchar_t tempBuf[256];

		va_list ap;
		va_start(ap, format);
		_vsnwprintf_s(tempBuf, 256, _TRUNCATE, format, ap);
		va_end(ap);
		method->Log(type, tempBuf);
	}
	else
	{
		return;
	}
}

void Logger::Enable(bool enable)
{
	this->enabled = enable;
}