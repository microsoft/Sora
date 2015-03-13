#include <time.h>
#include <conio.h>
#include "CRC32.h"
#include "bb/bbb.h"
#include "bb/DataRate.h"
#include "monitor.h"
#include "config.h"
#include "dot11_pkt.h"
#include "thread_func.h"
#include "appext.h"

static PACKET_BASE			Packet;
static MDL					Mdl;
static A16 UCHAR			DataBuffer[DataBufferSize];
static DOT11B_PLCP_TXVECTOR	TxVector;

static PVOID				SampleBuffer = NULL;
static ULONG				SampleBufferSize = 0;
static ULONG				TxID = 0;

static ULONG				TempBufferSize = _M(2);
static A16 COMPLEX8			TempBuffer[_M(2)];

// set data packet without crc16
static void Dot11BPreparePacket(const Config& config, PVOID pSymbolBuffer, ULONG SymbolBufferSize)
{
	ULONG	i;
    PUCHAR  pbIn = DataBuffer;
    unsigned int dataSize = config.GetPayloadLength();

    assert(DataBufferSize >= dataSize + 4);
	for (i = 0; i < dataSize; i++)
    {
		*pbIn = 'B';
        pbIn++;
    }

	PDOT11RFC1042ENCAP pWlanHeader = (PDOT11RFC1042ENCAP)DataBuffer;
	pWlanHeader->MacHeader.FrameControl.Type = FRAME_DATA;

    *(PULONG)pbIn = CalcCRC32(DataBuffer, dataSize);

	Mdl.Next					= NULL;
	Mdl.StartVa					= (PULONG)DataBuffer;
	Mdl.ByteOffset				= 0;
	Mdl.MappedSystemVa		    = (PULONG)DataBuffer;
	Mdl.ByteCount				= dataSize;

	Packet.pMdl					= &Mdl;
	Packet.PacketSize			= dataSize;
	Packet.pReserved			= pSymbolBuffer;
	Packet.Reserved2			= SymbolBufferSize;
	Packet.Reserved1			= *(PULONG)(DataBuffer + dataSize);

    TxVector.DateRate			= Dot11BDataRate_Kbps2Code(config.GetDataRate());
	TxVector.PreambleType		= DOT11B_PLCP_IS_LONG_PREAMBLE;
}

static int Dot11BTxInit()
{
    SampleBufferSize = _M(2);
    SampleBuffer = SoraUAllocBuffer(SampleBufferSize);
	printf("[dot11b:tx] tx buffer: %08x\n", SampleBuffer);
	printf("[dot11b:tx] tx buffer size: %08x\n", SampleBufferSize);
	if (SampleBuffer == NULL) return -1;

	HRESULT hr;
	if (SampleBufferSize < BB11B_MAX_SYMBOL_LENGTH)
	{
		printf("[dot11b:tx] Buffer size not enough.\n");
		return -1;
	}
	
	return 0;
}

static void Dot11BTxClean()
{
	SoraUReleaseBuffer((PVOID)SampleBuffer);
}

BOOLEAN DoDot11BTx(void* ctx)
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

    // Note: if we send too fast, the RX part cannot demodulate all the frames,
    // and the performance depends on the entry time, ie. not same in every experiment. 
    SoraStallWait(&tsinfo, 10000); // not to send too fast
    return TRUE;
}

void Dot11BTxApp(const Config& config)
{
	HRESULT hr;

	if (Dot11BTxInit() < 0)
		return;

	Dot11BPreparePacket(config, (PVOID)SampleBuffer, (ULONG)SampleBufferSize);

	do
	{
		// Generate Signal
		hr = BB11BPMDPacketGenSignal(&Packet, &TxVector, (PUCHAR)TempBuffer, TempBufferSize);
		printf("[dot11b:tx] GenSignal return %08x\n", hr);
        printf("[dot11b:tx] Signal bytes=%d\n", Packet.Reserved3);

		hr = SoraURadioTransferEx(TARGET_RADIO, SampleBuffer, Packet.Reserved3, &TxID);
		printf("[dot11b:tx] transfer, hr=%08x, id=%d\n", hr, TxID);
        FAILED_BREAK(hr);

        Monitor monitor;
        TxContext ctx(config, monitor);
        HANDLE hTxThread = AllocStartThread(DoDot11BTx, &ctx);

		if (SUCCEEDED(hr))
		{
            printf("\n\nPress any key to exit the program\n");
            time_t start = time(NULL);
	        while(!_kbhit())
            {
                if (config.Interval() != 0 && difftime(time(NULL), start) >= config.Interval()) break;
	        }

            StopFreeThread(hTxThread);

			hr = SoraURadioTxFree(TARGET_RADIO, TxID);
			printf("[dot11b:tx] tx free return %08x\n", hr);
		}
	} while(false);

	Dot11BTxClean();

	printf("[dot11b:tx] Tx out.\n");
}
