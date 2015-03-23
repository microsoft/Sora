#include <Windows.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <memory>
#include "LogWithFileBackup.h"

using namespace std;

int LogWithBackUpFile::index = 0;
SpinLock LogWithBackUpFile::__lockFile;

LogWithBackUpFile * LogWithBackUpFile::Make(const char * folder, const char * name)
{
	LogWithBackUpFile * logObj = new LogWithBackUpFile();
	
	__lockFile.Lock();
	HRESULT hr = logObj->CreateFile(folder, name);
	__lockFile.Unlock();

	if (SUCCEEDED(hr))
		return logObj;
	else
	{
		delete logObj;
		return nullptr;
	}
}

LogWithBackUpFile::LogWithBackUpFile() :
	logfileW(INVALID_HANDLE_VALUE),
	logfileR(INVALID_HANDLE_VALUE),
	idxfileW(INVALID_HANDLE_VALUE),
	idxfileR(INVALID_HANDLE_VALUE),
	recordCount(0),
	_bFileIsFull(false)
{
}


LogWithBackUpFile::~LogWithBackUpFile()
{
	SafeCloseHandles();
}

HRESULT LogWithBackUpFile::CreateFile(const char * folder, const char * name)
{
	do {
		this->foldername = folder;
		if (!CreateDirectoryRecursive(this->foldername.c_str()))
			break;

		ostringstream os;
		os << folder << "\\" << name << ".txt";
		this->logfileName = os.str();

		ostringstream os2;
		os2 << folder << "\\" << name << ".idx";
		this->idxfileName = os2.str();

		auto CreateForAppend = [](const string & name){
			return CreateFileA(
				name.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);		
		};

		logfileW = CreateForAppend(this->logfileName);
		if (logfileW == INVALID_HANDLE_VALUE)
			break;

		idxfileW = CreateForAppend(this->idxfileName);
		if (idxfileW == INVALID_HANDLE_VALUE)
			break;

		auto OpenForReadIfExist = [](const string & name){
			return CreateFileA(
				name.c_str(),
				GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
		};

		logfileR = OpenForReadIfExist(this->logfileName);
		if (logfileR == INVALID_HANDLE_VALUE)
			break;

		idxfileR = OpenForReadIfExist(this->idxfileName);
		if (idxfileR == INVALID_HANDLE_VALUE)
			break;

		return 0;

	} while(0);

	SafeCloseHandles();

	return -1;
}

bool LogWithBackUpFile::AddRecord(const char * logMsg)
{
	if (_bFileIsFull)
	{
		return false;
	}

	if (this->logfileW == INVALID_HANDLE_VALUE)
		return false;
	if (this->idxfileW == INVALID_HANDLE_VALUE)
		return false;

	bool bFileIsFull = false;

	int lenLogMsg = strlen(logMsg);
	int logFileSize = GetFileSize(this->logfileW, NULL);
	int indexFileSize = GetFileSize(this->idxfileW, NULL);

	const int FILE_SIZE_LIMIT = INT_MAX;

	if ( (((__int64)logFileSize + lenLogMsg) > FILE_SIZE_LIMIT) ||
		((__int64)indexFileSize + sizeof(IdxRecord)) > FILE_SIZE_LIMIT)
	{
		bFileIsFull = true;
	}
	else do {
		DWORD numWritten = 0;
		
		::SetFilePointer(this->logfileW, 0, 0, 2); 
		::WriteFile(this->logfileW, logMsg, lenLogMsg, &numWritten, NULL);
		if (numWritten < lenLogMsg)	// log file is full
		{
			bFileIsFull = true;
			break;
		}

		IdxRecord idxRecord;
		idxRecord.offset = logFileSize;
		idxRecord.length = lenLogMsg;

		::SetFilePointer(this->idxfileW, 0, 0, 2);
		::WriteFile(this->idxfileW, &idxRecord, sizeof(idxRecord), &numWritten, NULL);
		if (numWritten < sizeof(idxRecord))
		{
			bFileIsFull = true;
			break;
		}

		++recordCount;

	} while(0);

	if (bFileIsFull)
	{
		_bFileIsFull = true;
		++recordCount;	// for the last error message
		return false;
	}

	return true;
}

char * LogWithBackUpFile::Record(int index)
{
	if (_bFileIsFull)
	{
		if (index == recordCount - 1)
		{
			return strdup("Error: log buffer is full\n");
		}
	}

	if (this->logfileR == INVALID_HANDLE_VALUE)
		return 0;
	if (this->idxfileR == INVALID_HANDLE_VALUE)
		return 0;
	if (index >= recordCount)
		return 0;

	IdxRecord idxRecord;
	DWORD numRead;

	if (::SetFilePointer(this->idxfileR, index*sizeof(IdxRecord), 0, 0) == INVALID_SET_FILE_POINTER)
		return 0;

	BOOL bSucc = ::ReadFile(this->idxfileR, &idxRecord, sizeof(IdxRecord), &numRead, NULL);
	if (!bSucc)
		return 0;

	if (::SetFilePointer(this->logfileR, idxRecord.offset, 0, 0) == INVALID_SET_FILE_POINTER)
		return 0;

	char * readBuf = new char[idxRecord.length+1];
	readBuf[idxRecord.length] = 0;
	bSucc = ::ReadFile(this->logfileR, readBuf, idxRecord.length, &numRead, NULL);
	if (bSucc)
		return readBuf;
	else
	{
		delete [] readBuf;
		return 0;
	}

}

int LogWithBackUpFile::RecordCount()
{
	return recordCount;
}

void LogWithBackUpFile::SafeCloseHandles()
{
	auto SafeCloseHandle = [](HANDLE& handle){
		if (handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
		}
	};

	SafeCloseHandle(logfileW);
	SafeCloseHandle(logfileR);
	SafeCloseHandle(idxfileW);
	SafeCloseHandle(idxfileR);
}


bool LogWithBackUpFile::CreateDirectoryRecursive(const char * __folder)
{
	bool bSucc;

	char * folderDup = _strdup(__folder);
	int strlength = strlen(folderDup);
	
	// remove the last '\'
	if ( *(folderDup + strlength - 1) == '\\' )
		folderDup[strlength - 1] = 0;
	
	char * ptr = (char *)(folderDup + strlength - 1);

	while(ptr > folderDup && *ptr != '\\')
		--ptr;
	
	if (ptr == folderDup)	// root
		bSucc = true;
	else
	{
		*ptr = 0;
		bSucc = CreateDirectoryRecursive(folderDup);

		if (bSucc)
		{
			*ptr = '\\';
			BOOL bRes = CreateDirectoryA(folderDup, NULL);
			if (!bRes && GetLastError() != ERROR_ALREADY_EXISTS)
				bSucc = false;
			bSucc = true;
		}
		else
			bSucc = false;
	}

	delete [] folderDup;
	
	return bSucc;
}

bool LogWithBackUpFile::Export(const wchar_t * filename)
{
	auto hfileOutput = ::CreateFileW(
				filename,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);		

	if (hfileOutput == INVALID_HANDLE_VALUE)
		return false;

	const int MAX_COUNT =  1*1024*1024;
	std::unique_ptr<char []> readBuf(new char[MAX_COUNT]);


	DWORD numRead;
	DWORD numWrite;
	int offset = 0;

	do {
		::SetFilePointer(this->logfileR, offset, 0, 0);
		BOOL bSucc = ::ReadFile(this->logfileR, readBuf.get(), MAX_COUNT, &numRead, NULL);
		if (bSucc == FALSE)
			break;
		if (numRead == 0)
			break;

		::SetFilePointer(hfileOutput, 0, 0, 2);
		bSucc = ::WriteFile(hfileOutput, readBuf.get(), numRead, &numWrite, NULL);
		if (bSucc == FALSE)
			break;
		if (numRead < MAX_COUNT || numWrite < numRead)
			break;
		offset += MAX_COUNT;
	} while(1);

	CloseHandle(hfileOutput);

	return true;
}

void LogWithBackUpFile::ClearData()
{
	auto ResetFile = [](HANDLE hFile){
		::SetFilePointer(hFile, 0, 0, 0);
		::SetEndOfFile(hFile);		
	};

	ResetFile(this->idxfileR);
	ResetFile(this->idxfileW);
	ResetFile(this->logfileR);
	ResetFile(this->logfileW);

	this->recordCount = 0;
	this->_bFileIsFull = false;
}
