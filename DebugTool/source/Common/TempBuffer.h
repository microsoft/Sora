#pragma once

namespace SoraDbgPlot { namespace Buffer {

	class TempBuffer
	{
	public:
		TempBuffer();
		~TempBuffer();
		void ConfigKeptSize(int sizeKept);
		char * UseBuf(int size);
		void ReturnBuf();
	private:
		char * _buf;
		int _size;
		int _sizeKept;
	};

}}