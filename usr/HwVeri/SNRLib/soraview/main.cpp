#include <windows.h>
#include <stdio.h>
#include <dsptool.h>

/****************************************
Sora Viewer - Yet another gui tool for 
              Baseband Test
              
Should it be a replacement of MatLab?              
****************************************/

#include "const.hpp"
#include "context.hpp"
#include "decoder11a.hpp"
#include "sora-stream.hpp"

#include "configure.hpp"

#include "sora.h"

void Usage () {
	Printf ( "Soraview - by K. Tan\n"
		     "Usage - soraview sigfile-40MHz\n" );
}

class RxBlock2Complex
{
public:
	static COMPLEX * newComplexFromRxBlock(PRX_BLOCK buf, int size)
	{
		COMPLEX * complex = new COMPLEX[size*28];
		int complexIdx = 0;
		for (int i = 0; i < size; i++)
		{
			for (int idxComplex = 0; idxComplex < 28; idxComplex++)
			{

				PCOMPLEX16 pSample = (PCOMPLEX16)&(buf[i].u.SampleBlock);

				complex[complexIdx].re = pSample[idxComplex].re;
				complex[complexIdx].im = pSample[idxComplex].im;
				complexIdx++;
			}
		}

		return complex;
	}
};

int __cdecl main ( int argc, char* argv[] ) {
    Printf ( "Software oscolliscope...\n" );
    
	//g_channelConstel = m_channelConstel;	
	
	HINSTANCE hInst = GetModuleHandle (NULL);

	//STATIC_ASSERT (true); 

	if ( argc < 2 ) {
		Usage ();
		return 0;
	}

	FILE * fp;
	//fp = fopen(argv[1], "rb");
	errno_t result = fopen_s(&fp, argv[1], "rb");

	if (result != 0)
	{
		exit(-1);
	}

	const int SIZE_16M = 16*1024*1024;
	const int RX_BLOCK_CNT = SIZE_16M/sizeof(RX_BLOCK);
	RX_BLOCK * dumpData = new RX_BLOCK[RX_BLOCK_CNT];
	fread(dumpData, sizeof(RX_BLOCK), RX_BLOCK_CNT, fp);
	fclose(fp);

	Printf("file read\n");
	//getchar();

	COMPLEX * complex = RxBlock2Complex::newComplexFromRxBlock(dumpData, RX_BLOCK_CNT);
	delete dumpData;

	Printf("complex newed");
	//getchar();

	bool ret = sBuffer.SetBuffer(complex, RX_BLOCK_CNT*28);
	//g_pLogger->Print(ILogger::PNT_FUNC_CALL, L"SetBuffer called\r\n");

	while (1) {

		//g_pLogger->Print(ILogger::PNT_NORMAL, L"Before Process\r\n");
		bool ret = sBuffer.Process();
		//g_pLogger->Print(ILogger::PNT_NORMAL, L"%d\r\n", sBuffer.idx_);
		if (!ret)
			break;
		//g_pLogger->Print(ILogger::PNT_NORMAL, L"After Process\r\n");

		if (sBuffer.ctx_.reset_ ) {
			//g_pLogger->Print(ILogger::PNT_NORMAL, L"Before Reset\r\n");
			sBuffer.Reset ();
			//g_pLogger->Print(ILogger::PNT_NORMAL, L"After Process\r\n");
		} else if ( sBuffer.ctx_.bAbort_) {
			sBuffer.ctx_.Reset();
			break;
		}


		if ( sBuffer.ctx_.bPause_ ) {
			// don't pause
			//PauseConsole ( "Pause" );
			//g_pLogger->Print(ILogger::PNT_NORMAL, L"Pause\r\n");
			sBuffer.ctx_.bPause_ = false;
		}
	}

}

