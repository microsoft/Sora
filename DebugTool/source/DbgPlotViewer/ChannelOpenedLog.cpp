#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <ostream>
#include "ChannelOpenedLog.h"
#include "LogWithFileBackup.h"

using namespace std;
using namespace SoraDbgPlot::Task;

int ChannelOpenedLog::logObjIdx = 0;

ChannelOpenedLog::ChannelOpenedLog()
{
	int logObjIdx = ChannelOpenedLog::logObjIdx++;

	_color = RGB(0, 255, 0);	// green;
	char dir[256];
	::GetCurrentDirectoryA(256, dir);
	std::ostringstream os;
	os << dir << "\\logs\\" << logObjIdx;

	ostringstream osName;
	osName << logObjIdx;
	_logObj = LogWithBackUpFile::Make(os.str().c_str(), osName.str().c_str());
	
	_writeFilter = new SoraDbgPlot::FrameWithSizeInfoWriter<char>();

	_writeFilter->EventFlushData.Subscribe([this](const void * sender, const SoraDbgPlot::FrameWithSizeInfoWriter<char>::FlushDataEvent & e){

		if (this->_logObj == 0)
			return;

		size_t dataLen = e.length;
		if (dataLen >= 0)
		{
			int sizeNeeded = dataLen*2 + 1;
			char * dataBuf = _newLineFilterBuffer.UseBuf(sizeNeeded);

			char * ptrSrc = e.ptr;
			char * ptrDest = dataBuf;
			char preChar;

			while(ptrSrc < e.ptr + e.length)
			{
				char c = *ptrSrc;
				if (c != '\n')
					*ptrDest++ = c;
				else
				{
					if ( (ptrSrc == e.ptr) || preChar != '\r')
					{
						*ptrDest++ = '\r';
						*ptrDest++ = '\n';
					}
				}

				ptrSrc++;
				preChar = c;
			}

			*ptrDest = 0;

			this->_logObj->AddRecord(dataBuf);

			_newLineFilterBuffer.ReturnBuf();
		}
	});

	_newLineFilterBuffer.ConfigKeptSize(16*1024);
}

ChannelOpenedLog::~ChannelOpenedLog()
{
	delete _writeFilter;

	if (_logObj)
		delete _logObj;
}

void ChannelOpenedLog::WriteData(const char * data, size_t length)
{
	size_t sizeDummy;
	this->_writeFilter->Write(data, length, sizeDummy);	
	delete [] data;
}

size_t ChannelOpenedLog::DataSize()
{
	if (_logObj)
		return _logObj->RecordCount();
	else
		return 1;
}

char * ChannelOpenedLog::GetData(size_t index, bool bFromOldest)
{
	char * txt = 0;
	auto SThis = dynamic_pointer_cast<ChannelOpenedLog, AsyncObject>(shared_from_this());
	TaskQueue()->DoTask([SThis, index, bFromOldest, &txt](){

		if (SThis->_logObj)
		{
			size_t size = SThis->_logObj->RecordCount();
			if (size <= index)
				return;

			size_t index2 = bFromOldest ? index : size - 1 - index;
			txt = SThis->_logObj->Record(index2);
		}
		else
		{
			txt = "Error creating log file.";
		}
	});

	return txt;
}

shared_ptr<TaskSimple> ChannelOpenedLog::TaskGetSize(shared_ptr<size_t> size)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedLog, AsyncObject>(shared_from_this());
	auto task = make_shared<TaskSimple>(TaskQueue(), [SThis, size](){
		if (SThis->_logObj)
			*size = SThis->_logObj->RecordCount();
		else
			*size = 1;
	});
	
	return task;
}

const wchar_t * ChannelOpenedLog::GetTypeName()
{
	return L"Log Channel";
}

bool ChannelOpenedLog::Export(const CString & filename, bool bAll)
{
	auto SThis = dynamic_pointer_cast<ChannelOpenedLog, AsyncObject>(shared_from_this());

	this->DoLater([SThis, filename, bAll](){
		if (SThis->_logObj)
			SThis->_logObj->Export(filename);
		else
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

			if (hfileOutput != INVALID_HANDLE_VALUE)
				CloseHandle(hfileOutput);		
		}
	});

	return true;
}

void ChannelOpenedLog::ClearData()
{
	if (this->_logObj)
		this->_logObj->ClearData();
}
