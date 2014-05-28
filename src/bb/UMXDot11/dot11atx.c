#include <assert.h>
#include <conio.h>
#include <time.h>
#include "CRC32.h"
#include "bb/bba.h"
#include "dot11_pkt.h"
#include "monitor.h"
#include "bb/DataRate.h"
#include "appext.h"
#include "thread_func.h"

static PACKET_BASE			Packet;
static MDL					Mdl;
static A16 UCHAR			DataBuffer[DataBufferSize];
static BB11A_TX_VECTOR		TxVector;

static PVOID				SampleBuffer = NULL;
static ULONG				SampleBufferSize = 0;
static ULONG				TxID = 0;

void PreparePacket(const Config& config, PVOID pSymbolBuffer, ULONG SymbolBufferSize)
{
	UINT    i;
    PUCHAR  pbIn = DataBuffer;
    //ULONG   CRC32;

    unsigned int DataSize = config.GetPayloadLength();
    assert(DataBufferSize >= DataSize + 4);
	for (i = 0; i < DataSize; i++)
    {
		*pbIn = 'A';
        pbIn++;
    }

	PDOT11RFC1042ENCAP pWlanHeader = (PDOT11RFC1042ENCAP)DataBuffer;
	pWlanHeader->MacHeader.FrameControl.Type = FRAME_DATA;

    *(PULONG)pbIn = CalcCRC32(DataBuffer, DataSize);

	Mdl.Next					= NULL;
	Mdl.StartVa					= (PULONG)DataBuffer;
	Mdl.ByteOffset				= 0;
	Mdl.MappedSystemVa		    = (PULONG)DataBuffer;
	Mdl.ByteCount				= DataSize;

	Packet.pMdl					= &Mdl;
	Packet.PacketSize			= DataSize;
	Packet.pReserved			= pSymbolBuffer;
	Packet.Reserved2			= SymbolBufferSize;
	Packet.Reserved1			= *(PULONG)(DataBuffer + DataSize);

    TxVector.ti_uiDataRate		= Dot11ADataRate_Kbps2Code(config.GetDataRate());
	TxVector.ti_uiBufferLength	= DataSize;
    TxVector.SampleRate         = config.GetSampleRate();
}

BOOLEAN DoDot11ATx(void* ctx)
{
    // Alias
    Monitor& monitor = ((TxContext *)ctx)->monitor;
    const Config& config = ((TxContext *)ctx)->config;

    monitor.Query(true);

	HRESULT hr = SoraURadioTx(TARGET_RADIO, TxID);
	if (FAILED(hr))
	{
		printf("[dot11b:tx] tx failed\n");
		//break;
	}
    else
    {
        monitor.IncGoodCounter();
        monitor.IncThroughput(config.GetPayloadLength());
    }
    SoraStallWait(&tsinfo, 10000); // not to send too fast
    return TRUE;
}

void Dot11ATxApp(const Config& config)
{
    SampleBufferSize = _M(2);
    SampleBuffer = SoraUAllocBuffer(SampleBufferSize);
	printf("tx buffer: %08x\n", SampleBuffer);
	printf("tx buffer size: %08x\n", SampleBufferSize);
	if (SampleBuffer == NULL) return;

	PreparePacket(config, (PVOID)SampleBuffer, (ULONG)SampleBufferSize);

    HRESULT hr;
	do
	{
		//Generate Signal
		hr = BB11ATxFrameMod(&TxVector, &Packet);
		printf("GenSignal return %08x\n", hr);
		printf("Signal bytes = %d\n", Packet.Reserved3);

        /*{
            PCOMPLEX8 pSampleBuffer = (PCOMPLEX8)SampleBuffer;
            for (i = 0; i < Packet.Reserved3; i++)
                printf("(%5d, %5d)\t", pSampleBuffer[i].re, pSampleBuffer[i].im);
            printf("\n");
        }*/

		hr = SoraURadioTransferEx(TARGET_RADIO, SampleBuffer, Packet.Reserved3, &TxID);
		printf("transfer, hr=%08x, id=%d\n", hr, TxID);
        FAILED_BREAK(hr);

        Monitor monitor;
        TxContext ctx(config, monitor);
        HANDLE hTxThread = AllocStartThread(DoDot11ATx, &ctx);

		if (SUCCEEDED(hr) && hTxThread)
		{
            printf("\n\nPress any key to exit the program\n");			
            time_t start = time(NULL);
	        while(!_kbhit())
            {
                if (config.Interval() != 0 && difftime(time(NULL), start) >= config.Interval()) break;
	        }

            StopFreeThread(hTxThread);
			hr = SoraURadioTxFree(TARGET_RADIO, TxID);
			printf("tx free return %08x\n", hr);
		}
	} while (FALSE);

	SoraUReleaseBuffer((PVOID)SampleBuffer);
	printf("unmap tx buffer ret: %08x\n", hr);

	printf("Tx out.\n");
}
