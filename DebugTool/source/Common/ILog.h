#pragma once

#include <wchar.h>

class ILog
{
public:
	virtual bool AddRecord(const char * logMsg) = 0;
	virtual char * Record(int index) = 0;
	virtual int RecordCount() = 0;
	virtual ~ILog() = 0;
	virtual bool Export(const wchar_t * filename) = 0;
	virtual void ClearData() = 0;
};

