#include <stdio.h>
#include "CRC32.h"
#include "bb/bba.h"
#include "bb/DataRate.h"
#include "dot11_pkt.h"
#include "bb_test.h"
#include "atx.h"

#define _K(N) (1024 * (N))
#define _M(N) (1024 * 1024 * (N))

#define SYMBOLBUF_SIZE (_M(1))

static PACKET_BASE          Packet;
static MDL                  Mdl;
static UCHAR                DataBuffer[INPUT_MAX];
static MEM_ALIGN(16) UCHAR  SymbolBuffer[SYMBOLBUF_SIZE];

#define RUN_TIMES 100
int TestMod11A(PDOT11_MOD_ARGS pArgs)
{
    BB11A_TX_VECTOR      TxVector;
    BB11ATxContextInit(&TxVector, pArgs->SampleRate);

    if (!Dot11ARate_KbpsValid(pArgs->nBitRate))
    {
        printf("Data rate %d kbps is not supported by 802.11a mode\n", pArgs->nBitRate);
        return -1;
    }
    TxVector.ti_uiDataRate = Dot11ADataRate_Kbps2Code(pArgs->nBitRate);

    if (PreparePacket(pArgs->pcInFileName, &Mdl, &Packet, DataBuffer, SymbolBuffer, SYMBOLBUF_SIZE) < 0)
        return -1;

    TIMINGINFO  ti;
    double dTimeSum = 0.0;

    // Cache warm-up for first 2 rounds
    for (int i = 0; i < 2; i++)
    {
        BB11ATxFrameMod(&TxVector, &Packet);
    }

    // Measure modulation speed for many rounds
    TimerStart(&ti);
    for (int i = 0; i < RUN_TIMES; i++)
    {
        BB11ATxFrameMod(&TxVector, &Packet);
    }
    TimerStop(&ti);
    dTimeSum = TimerRead(&ti) * 1000;

    printf("Signal data rate:    %d kbps\n", Dot11ADataRate_Code2Kbps(TxVector.ti_uiDataRate));
    printf("Signal packet size:  %d\n", Packet.PacketSize);
    printf("Signal encoded size: %d\n", Packet.Reserved3);
    printf("Time cost average:   %.3f us \n", dTimeSum / RUN_TIMES);

    FILE* pOut = NULL;
#pragma warning (push)
#pragma warning (disable:4996)
    pOut = fopen(pArgs->pcOutFileName, "w+b");
#pragma warning (pop)
    if (!pOut)
    {
        printf("Cannot create output file.\n");
        return -1;
    }

    fwrite(Packet.pReserved, Packet.Reserved3, 1, pOut);
    fclose(pOut);

    return 0;
}

BOOLEAN Test11AACK(PDOT11_MOD_ARGS pArgs)
{
    static A16 COMPLEX8 AckBuffer[16 * 1024];

	MAC_ADDRESS RecvMacAddress;
    RecvMacAddress.Address[0] = 0x00;
    RecvMacAddress.Address[1] = 0x14;
    RecvMacAddress.Address[2] = 0x6c;
    RecvMacAddress.Address[3] = 0xe2;
    RecvMacAddress.Address[4] = 0x00;
    RecvMacAddress.Address[5] = 0xe5;

    ULONG OutputLengthInBytes = BB11AModulateACK(pArgs->SampleRate, &RecvMacAddress, AckBuffer);

    DOT11_MAC_ACK_FRAME  AckFrame           = {0};
    BB11A_TX_VECTOR      Dot11ATxVector = {0};
    ULONG ulCRC;

    AckFrame.FrameControl.Subtype       = SUBT_ACK;
    AckFrame.FrameControl.Type          = FRAME_CTRL;
    AckFrame.RecvAddress                = RecvMacAddress;
    AckFrame.Duration                   = 0;
    AckFrame.FCS                        = ulCRC = CalcCRC32((PUCHAR)&AckFrame, sizeof(DOT11_MAC_ACK_FRAME) - sizeof(ULONG));

    Dot11ATxVector.ti_uiBufferLength    = sizeof(DOT11_MAC_ACK_FRAME);
    Dot11ATxVector.ti_uiDataRate        = DOT11A_RATE_6M;
    Dot11ATxVector.SampleRate           = pArgs->SampleRate;

    Mdl.Next                = NULL;
    Mdl.StartVa             = (PULONG)&AckFrame;
    Mdl.ByteOffset          = 0;
    Mdl.MappedSystemVa      = (PULONG)&AckFrame;
    Mdl.ByteCount           = sizeof(DOT11_MAC_ACK_FRAME) - 4;

	Packet.pReserved        = (PVOID)SymbolBuffer;
	Packet.Reserved2        = SYMBOLBUF_SIZE;
	Packet.pMdl             = &Mdl;
	Packet.Reserved1        = ulCRC;
	Packet.PacketSize       = sizeof(DOT11_MAC_ACK_FRAME) - 4;

	BB11ATxFrameMod(&Dot11ATxVector, &Packet);

	if(memcmp(AckBuffer, SymbolBuffer, OutputLengthInBytes) == 0)
	{
		printf("802.11a ACK test OK!(ACK size: %d)\n", OutputLengthInBytes);
		return TRUE;
	}

	printf("802.11a ACK test Fail!(ACK size: %d)\n", OutputLengthInBytes);
	return FALSE;
}

#define DUMP_BUFFER_MAX     (16 * 1024 * 1024)
char g_dumpBuffer[DUMP_BUFFER_MAX];

// The input file is in COMPLEX8 format
BOOLEAN ConvertModFile2DumpFile_8b(const char * inputFilename, const char * outputFilename)
{
    FILE * fin, * fout;

    const size_t COMPLEX_PER_DESC = 28;
    char rgbDescHeader[16] = { 0x01, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned int uiCount;
    unsigned int i, j;
    COMPLEX8 * pcInput;

#pragma warning (push)
#pragma warning (disable:4996)
    fin = fopen(inputFilename, "rb");

    if (!fin)
    {
        printf("Cannot find input file.\n");
        return FALSE;
    }

    fout = fopen(outputFilename, "wb");
    if (!fout)
    {
        fclose(fin);
        printf("Cannot create output file.\n");
        return FALSE;
    }
#pragma warning (pop)

    memset(g_dumpBuffer, DUMP_BUFFER_MAX, 0);
    uiCount = fread(g_dumpBuffer, sizeof(COMPLEX8), 
            DUMP_BUFFER_MAX / sizeof(COMPLEX8), fin);
    uiCount += COMPLEX_PER_DESC - (uiCount % COMPLEX_PER_DESC); // pad to integer multiplies of COMPLEX_PER_DESC
    uiCount += 2 * COMPLEX_PER_DESC; // pad some silence here
    if ((uiCount / COMPLEX_PER_DESC) % 2)
        uiCount += COMPLEX_PER_DESC; // pad to even integer multiplies of COMPLEX_PER_DESC

    pcInput = (COMPLEX8 *) g_dumpBuffer;

    for (i = 0; i < uiCount; i += COMPLEX_PER_DESC)
    {
        fwrite(rgbDescHeader, sizeof(char), 16, fout);
        for (j = 0; j < COMPLEX_PER_DESC; j++)
        {
            COMPLEX16 c;
            // c = (*pcInput);
            
            c.re = pcInput->re << 8;
            c.im = pcInput->im << 8;
            
            fwrite(&c, sizeof(COMPLEX16), 1, fout);
            pcInput++;
        }
    }

    fclose(fout);
    fclose(fin);

    return 0;
}

// The input file is in COMPLEX16 format
BOOLEAN ConvertModFile2DumpFile_16b(const char * inputFilename, const char * outputFilename)
{
    char rgbDescHeader[16] = { 0x01, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00};

#pragma warning (push)
#pragma warning (disable:4996)
    // Open iput file
    FILE * fin, * fout;
    fin = fopen(inputFilename, "rb");
    if (!fin)
    {
        printf("Cannot find input file.\n");
        return FALSE;
    }

    // Open output file
    fout = fopen(outputFilename, "wb");
    if (!fout)
    {
        fclose(fin);
        printf("Cannot create output file.\n");
        return FALSE;
    }
#pragma warning (pop)

    // Copy ipnut data to output, and insert RX_BLOCK headers
    const size_t data_size = sizeof(SORA_SAMPLE_BLOCK);
    A16 char buf[data_size];
    int rd;
    for (;;)
    {
        rd = fread(buf, 1, data_size, fin);
        if (rd < data_size) break;
        fwrite(rgbDescHeader, sizeof(rgbDescHeader), 1, fout);
        fwrite(buf, sizeof(buf), 1, fout);
    }

    // Copy remaining data
    if (rd > 0)
    {
        fwrite(rgbDescHeader, sizeof(rgbDescHeader), 1, fout);
        fwrite(buf, rd, 1, fout);
    }

    fclose(fout);
    fclose(fin);

    return 0;
}
