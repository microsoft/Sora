#pragma once

#include <memory>
#include <functional>
#include "SmProcess.h"
#include "Event.h"


namespace SoraDbgPlot { namespace SharedObj {

	class SharedProcess
	{
	public:
		SharedProcess(int pid);
		~SharedProcess();
		int Pid();
		bool IsValid();
		std::wstring ModuleName();
		std::wstring GetProcessNameByPid(int pid);
		bool TestPauseFlag();

		void PeekRawData(COMPLEX16 * buf, int size, int & start, int width, int & readPos, int & sizeValid);
		void SetReadPos(int pos);
		void RawDataPlay();
		void RawDataPause();
		void RawDataSingleStep();
		bool IsRawDataBufferUsed();

	private:
		std::shared_ptr<SmProcess> _sm;
		int _pid;
		bool _isValid;
		std::wstring _name;
	};

}}
