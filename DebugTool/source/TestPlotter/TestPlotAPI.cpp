#include <assert.h>
#include "DebugPlotU.h"

int TestPlotAPI()
{

	DebugPlotInit();
	DebugPlotDeinit();
	DebugPlotInit();
	DebugPlotDeinit();

	DebugPlotInit();

	COMPLEX16 complex;
	int data;

	::PlotDots("dots", &complex, 1);
	::PlotSpectrum("spectrum", &data, 1);
	::PlotText("text", "hi"); 
	::PlotLine("line", &data, 1);
	::Log("log", "a log");

	[](){

		auto f = [](int BUF_SIZE){
			int * dataBuf = new int[BUF_SIZE];
			assert(::PlotLine("line", dataBuf, BUF_SIZE) == S_OK);

			// type mismatch error code
			assert(::PlotLine("dots", dataBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotLine("spectrum", dataBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotLine("text", dataBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotLine("log", dataBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);

			// invalid parameters
			assert(::PlotLine(0, dataBuf, BUF_SIZE) == E_INVALID_PARAMETER);
			assert(::PlotLine("line", 0, BUF_SIZE) == E_INVALID_PARAMETER);
			assert(::PlotLine("line", dataBuf, 0) == E_INVALID_PARAMETER);
			assert(::PlotLine("line", dataBuf, -1) == E_INVALID_PARAMETER);

			delete [] dataBuf;
		};

		f(512);
		f(1024);
		f(4096);
		f(8192);
	}();


	[](){
		auto f = [](int BUF_SIZE){
		COMPLEX16 * dataBuf = new COMPLEX16[BUF_SIZE];
		assert(::PlotDots("dots", dataBuf, BUF_SIZE) == S_OK);

		// type mismatch error code
		assert(::PlotDots("line", dataBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
		assert(::PlotDots("spectrum", dataBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
		assert(::PlotDots("text", dataBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
		assert(::PlotDots("log", dataBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);

		// invalid parameters
		assert(::PlotDots(0, dataBuf, BUF_SIZE) == E_INVALID_PARAMETER);
		assert(::PlotDots("dots", 0, BUF_SIZE) == E_INVALID_PARAMETER);
		assert(::PlotDots("dots", dataBuf, 0) == E_INVALID_PARAMETER);
		assert(::PlotDots("dots", dataBuf, -1) == E_INVALID_PARAMETER);

		delete [] dataBuf;
		};
		
		f(512);
		f(1024);
		f(4096);
		f(8192);
	}();

	[](){

		const int BUF_SIZE = 1024;
		int * spectrumBuf = new int[BUF_SIZE];
		assert(::PlotSpectrum("spectrum2", spectrumBuf, BUF_SIZE) == S_OK);
		assert(::PlotSpectrum("spectrum2", spectrumBuf, 1) == E_SPECTRUM_SIZE_INVALID);

		// type mismatch error code
		assert(::PlotSpectrum("line", spectrumBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
		assert(::PlotSpectrum("dots", spectrumBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
		assert(::PlotSpectrum("text", spectrumBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);
		assert(::PlotSpectrum("log", spectrumBuf, BUF_SIZE) == E_PLOT_TYPE_MISMATCH);

		// invalid parameters
		assert(::PlotSpectrum(0, spectrumBuf, BUF_SIZE) == E_INVALID_PARAMETER);
		assert(::PlotSpectrum("spectrum2", 0, BUF_SIZE) == E_INVALID_PARAMETER);
		assert(::PlotSpectrum("spectrum2", spectrumBuf, 0) == E_INVALID_PARAMETER);
		assert(::PlotSpectrum("spectrum2", spectrumBuf, -1) == E_INVALID_PARAMETER);

		delete [] spectrumBuf;

	}();

	[](){
		auto f = [](int BUF_SIZE){
			wchar_t * dataBuf = new wchar_t[BUF_SIZE];
			for (int i = 0; i < 1024; i++)
			{
				dataBuf[i] = L'a';
			}
			dataBuf[BUF_SIZE-1] = 0;

			assert(::PlotText("text", "%s", dataBuf) == S_OK);

			// type mismatch error code
			assert(::PlotText("line", "%s", dataBuf) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotText("dots", "%s", dataBuf) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotText("spectrum", "%s", dataBuf) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotText("log", "%s", dataBuf) == E_PLOT_TYPE_MISMATCH); 

			// invalid parameters
			assert(::PlotText(0, "%s", dataBuf) == E_INVALID_PARAMETER);
			assert(::PlotText("text", 0, dataBuf) == E_INVALID_PARAMETER);

			delete [] dataBuf;		
		};

		f(1024);
		f(4096);
		f(8192);
	}();

	[](){
		auto f = [](int BUF_SIZE){
			wchar_t * dataBuf = new wchar_t[BUF_SIZE];
			for (int i = 0; i < 1024; i++)
			{
				dataBuf[i] = L'a';
			}
			dataBuf[BUF_SIZE-1] = 0;

			assert(::PlotText("text", "%s", dataBuf) == S_OK);

			// type mismatch error code
			assert(::PlotText("line", "%s", dataBuf) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotText("dots", "%s", dataBuf) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotText("spectrum", "%s", dataBuf) == E_PLOT_TYPE_MISMATCH);
			assert(::PlotText("log", "%s", dataBuf) == E_PLOT_TYPE_MISMATCH); 

			// invalid parameters
			assert(::PlotText(0, "%s", dataBuf) == E_INVALID_PARAMETER);
			assert(::PlotText("text", 0, dataBuf) == E_INVALID_PARAMETER);

			delete [] dataBuf;
		};

		f(1024);
		f(4096);
		f(8192);
	}();

	[](){
	
		int sizeWritten;
		COMPLEX16 buf[128];
		HRESULT hRes = ::TracebufferWriteData(buf, 16, &sizeWritten);
		assert(hRes == S_OK);
		hRes = ::TracebufferWriteData(buf, 16, 0);
		assert(hRes == S_OK);
		hRes = ::TracebufferWriteData(0, 16, &sizeWritten);
		assert(hRes == E_INVALID_PARAMETER);
		hRes = ::TracebufferWriteData(buf, 0, &sizeWritten);
		assert(hRes == E_INVALID_PARAMETER);
		hRes = ::TracebufferWriteData(buf, -1, &sizeWritten);
		assert(hRes == E_INVALID_PARAMETER);
	}();

	[](){
	
		int sizeRead;
		COMPLEX16 buf[128];
		HRESULT hRes = ::TracebufferReadData(buf, 16, &sizeRead);
		assert(hRes == S_OK);
		hRes = ::TracebufferReadData(buf, 16, 0);
		assert(hRes == S_OK);
		hRes = ::TracebufferReadData(0, 16, &sizeRead);
		assert(hRes == E_INVALID_PARAMETER);
		hRes = ::TracebufferReadData(buf, 0, &sizeRead);
		assert(hRes == E_INVALID_PARAMETER);
		hRes = ::TracebufferReadData(buf, -1, &sizeRead);
		assert(hRes == E_INVALID_PARAMETER);
	}();

	[](int countToTest){
		const int BUF_CAPACITY = 16*1024*1024 / sizeof(COMPLEX16);
		const int BUF_SIZE = 32*1024*1024 / sizeof(COMPLEX16);
		COMPLEX16 * buffer = new COMPLEX16[BUF_SIZE];
		COMPLEX16 * rBuffer = new COMPLEX16[BUF_SIZE];
		for (int i = 0; i < BUF_SIZE; i++)
		{
			buffer[i].re = (short)i;
			buffer[i].im = (short)(255-i);
		}
	
		for (int count = 0; count < countToTest; ++count)
		{

			::TracebufferClear();

			COMPLEX16 * wPtr = buffer;
			while(1)
			{
				int sizeToWrite = max(1, rand() % BUF_CAPACITY);
				int sizeWritten;
				HRESULT hRes = ::TracebufferWriteData(wPtr, sizeToWrite, &sizeWritten);
				if (hRes == S_OK)
					assert(sizeToWrite == sizeWritten);
				if (hRes == E_END_OF_BUF)
					assert(sizeToWrite > sizeWritten);
				if (sizeToWrite != sizeWritten)
					assert(hRes == E_END_OF_BUF);

				wPtr += sizeWritten;

				if (hRes == E_END_OF_BUF)
					break;
			}

			COMPLEX16 * rPtr = rBuffer;
			while(1)
			{
				int sizeToRead = max(1, rand() % BUF_CAPACITY);
				int sizeRead;
				HRESULT hRes = ::TracebufferReadData(rPtr, sizeToRead, &sizeRead);
				if (hRes == S_OK)
					assert(sizeToRead = sizeRead);
				if (hRes == E_END_OF_BUF)
					assert(sizeToRead > sizeRead);
				if (sizeToRead != sizeRead)
					assert(hRes == E_END_OF_BUF);

				rPtr += sizeRead;

				if (hRes == E_END_OF_BUF)
					break;
			}

			assert(wPtr - buffer == BUF_CAPACITY);
			assert(rPtr - rBuffer == BUF_CAPACITY);

			for (int i = 0; i < BUF_CAPACITY; ++i)
			{
				assert(buffer[i].re == rBuffer[i].re);
				assert(buffer[i].im == rBuffer[i].im);
			}
		}

		delete [] buffer;
		delete [] rBuffer;
	}(16);

	[](){
		::WaitForViewer(INFINITE);
		::WaitForViewer(INFINITE);
		::PauseViewer();
	}();

	::DebugPlotDeinit();

	return 0;
}
