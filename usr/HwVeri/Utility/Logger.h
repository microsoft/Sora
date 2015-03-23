#pragma once

#include <Windows.h>
#include <map>
#include <string>

enum LOG_TYPE
{
	LOG_STATUS = -1,
	LOG_DEFAULT = 0,
	LOG_INFO,
	LOG_SUCCESS,
	LOG_ERROR,
	LOG_DEBUG,
	LOG_FUNC_CALL,
	LOG_USER,
};

class LogMethod
{
public:
	virtual void Log(int type, const wchar_t * string) = 0;
};

class Logger
{
public:
	static Logger * GetLogger(const wchar_t * name);
	static void ReleaseAll();

	~Logger();
	HRESULT SetLogMethod(LogMethod * method, bool autoDelete);
	void __cdecl Log(int type, const wchar_t * format, ...);
	void Enable(bool enable);
private:
	static std::map<std::wstring, Logger *> loggers;
	
	Logger();
	bool implemented;
	LogMethod * method;
	bool autoDelete;
	bool enabled;
};


