#pragma once

#include <string>
#include <Windows.h>
#include "ILog.h"
#include "SpinLock.h"

class LogWithBackUpFile : public ILog
{
public:
	// Factory
	static LogWithBackUpFile * __stdcall Make(const char * folder, const char * name);
	static SpinLock __lockFile;

public:
	struct IdxRecord
	{
		int offset;
		int length;
	};

	// ILog interface
	bool AddRecord(const char * logMsg);
	char * Record(int index);
	int RecordCount();
	~LogWithBackUpFile();

private:
	static int index;

private:
	LogWithBackUpFile();
	HRESULT CreateFile(const char * folder, const char * name);
	void SafeCloseHandles();
	bool CreateDirectoryRecursive(const char * folder);

	std::string foldername;
	std::string logfileName;
	std::string idxfileName;

	HANDLE logfileW;
	HANDLE idxfileW;

	HANDLE logfileR;
	HANDLE idxfileR;

	int recordCount;

	bool _bFileIsFull;

public:
	virtual bool Export(const wchar_t * filename);
	virtual void ClearData();
};


