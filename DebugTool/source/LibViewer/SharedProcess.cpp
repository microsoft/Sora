#include <Windows.h>
#include <Psapi.h>
#include <memory>
#include "SharedProcess.h"
#include "SmProcess.h"

using namespace std;
using namespace SoraDbgPlot::SharedObj;

SharedProcess::SharedProcess(int pid)
{
	_pid = pid;
	_sm = make_shared<SmProcess>(pid);
	_isValid = _sm->IsValid();

	if (_isValid)
	{
		_name = GetProcessNameByPid(pid);
		if (_name.size() == 0)
			_isValid = false;
	}
}

SharedProcess::~SharedProcess()
{
	_sm.reset();
}

int SharedProcess::Pid()
{
	return _pid;
}

bool SharedProcess::IsValid()
{
	return _isValid;
}


std::wstring SharedProcess::ModuleName()
{
	return _name;
}

std::wstring SharedProcess::GetProcessNameByPid(int pid)
{
	std::wstring ret = L"";

	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ, FALSE, pid);

	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		TCHAR nameBuffer[MAX_PATH];

		if(::EnumProcessModules(hProcess, &hMod, 
			sizeof(hMod), &cbNeeded))
		{
			::GetModuleBaseName(hProcess, hMod, nameBuffer, 
				sizeof(nameBuffer)/sizeof(TCHAR));

			ret = nameBuffer;
		}

		::CloseHandle(hProcess);
	}	

	return ret;
}

bool SharedProcess::TestPauseFlag()
{
	return _sm->TestPauseFlag();
}

void SharedProcess::PeekRawData(COMPLEX16 * buf, int size, int & start, int width, int & readPos, int & sizeValid)
{
	_sm->PeekRawData(buf, size, start, width, readPos, sizeValid);
}

void SharedProcess::SetReadPos(int pos)
{
	_sm->SetReadPos(pos);
}

void SharedProcess::RawDataPlay()
{
	_sm->RawDataPlay();
}

void SharedProcess::RawDataPause()
{
	_sm->RawDataPause();
}

void SharedProcess::RawDataSingleStep()
{
	_sm->RawDataSingleStep();
}

bool SharedProcess::IsRawDataBufferUsed()
{
	return _sm->IsRawDataBufferUsed();
}
