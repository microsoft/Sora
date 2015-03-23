/*******************************************************************
* umx_sample.cpp - 
*     an sample SDR application using Sora User Mode Extension

********************************************************************/
#include "sora.h"
#include "vector128.h"
#include "soradsp.h"

#define TARGET_RADIO 0
#define MAX_SAMPLE_SIZE (2*1024*1024)
#define _M(N)			(1024 * 1024 * (N))

ULONG PrepareSamples(const char *fname, char *SampleBuffer, ULONG SampleBufferSize)
{
    FILE *hSigFile;
    ULONG nRead = 0;
    hSigFile = fopen(fname, "rb");

    if (hSigFile)
    {
        nRead = fread(SampleBuffer, sizeof(char), SampleBufferSize, hSigFile);
    }

    return nRead;
}

void TxRoutine ( char* fname )
{
    // try to load the samples
    HRESULT hr = S_OK;
    PVOID SampleBuffer = NULL;
    ULONG SampleBufferSize = 0;
    ULONG TxID = 0;

    do
    {
        SampleBufferSize = _M(2);
        SampleBuffer = SoraUAllocBuffer(SampleBufferSize);
        if (SampleBuffer == NULL) break;

        printf("modulation buffer: %08x\n", SampleBuffer);
        printf("modulation buffer size: %08x\n", SampleBufferSize);

        ULONG SigLength = PrepareSamples(fname, (char*)SampleBuffer, SampleBufferSize);
        if (SigLength == 0)
        {
            printf("file access violation\n");
            break;
        }
        
        //
        // First Allocate Tx Resource
        // The size should be a multiple of 128
        //
		hr = SoraURadioTransferEx(TARGET_RADIO, SampleBuffer, SampleBufferSize, &TxID);

        printf("paralle tx resource allocated, hr=%08x, id=%d, length=%d\n", hr, TxID, SigLength);
        if (SUCCEEDED(hr))
        {
            //
            // Tx to the radio
            //
            hr = SoraURadioTx(TARGET_RADIO, TxID);

            printf("tx return %08x\n", hr);

            hr = SoraURadioTxFree(TARGET_RADIO, TxID);
            printf("tx resource release return %08x\n", hr);
        }

        FAILED_BREAK(hr);
    } while (FALSE);
    if (SampleBuffer)
    {
        SoraUReleaseBuffer((PVOID)SampleBuffer);
        printf("unmap mod buffer ret: %08x\n", hr); 
    }
}

void RxRoutine ()
{
    PVOID pRxBuf     = NULL;
    ULONG nRxBufSize = 0;
    HRESULT hr;
    LARGE_INTEGER Start;
    LARGE_INTEGER End, Freq;

    QueryPerformanceFrequency(&Freq); 
    
    SORA_RADIO_RX_STREAM SampleStream;
    
    //
    // Map Rx Buffer 
    //
    hr = SoraURadioMapRxSampleBuf( TARGET_RADIO, // radio id
                                & pRxBuf,     // mapped buffer pointer
                                & nRxBufSize  // size of mapped buffer
                              );

    if ( FAILED (hr) ) {
        printf ( "Error: Fail to map Rx buffer!\n" );
        return;
    }
    
    printf ( "Mapped Rx buffer at %08x size %d\n", pRxBuf, nRxBufSize );

    // Generate a sample stream from mapped Rx buffer
    SoraURadioAllocRxStream( &SampleStream,
                            TARGET_RADIO,
                            (PUCHAR)pRxBuf, 
                            nRxBufSize);

    // start reading the sample stream and compute the energy
    FLAG fReachEnd;

    int index = 0;
    SignalBlock block;

    QueryPerformanceCounter(&Start);
    for (;;)
    {

        hr = SoraRadioReadRxStream ( &SampleStream,      // current scan point
                            & fReachEnd,   // indicate if end of stream reached (you must wait for hardware)
                            block);

        if (FAILED(hr))
        {
            printf("stream ended, hr=%08x\n", hr);
            break;
        }
        QueryPerformanceCounter(&End);
        if (End.QuadPart - Start.QuadPart > Freq.QuadPart / 2)
        {
            Start = End;
            // almost 1s
            // compute the energy 
            vcs* pSamples = &block[0];

            // single block contains 28 samples or 7 vector cs
            vi sum;
            set_zero (sum);

            // this is an approximated way to calc energy
            for (int i=0; i<7; i++ ) {
                vi re, im;
                vcs s = pSamples[i];
                s = shift_right (s, 3);

                conj_mul ( re, im, s, s ); // (a+bj) * (a-bj)
                sum = add (sum, re );
            }
            
            sum = hadd (sum); // get a sum of the all element on the vector

            int energy = sum[0];
            printf("                                                                     \r"); //clean the line.
            printf ( "%d --> Energy %10d \r", index++, energy / 1000);            
        }
    }
    
    SoraURadioReleaseRxStream(&SampleStream, TARGET_RADIO);
    if (pRxBuf) {
        hr = SoraURadioUnmapRxSampleBuf(TARGET_RADIO, pRxBuf);
    }
    printf("Unmap hr:%08x\n", hr);
}


void RadioConfig()
{
    // always start radio first, it will reset radio to the default setting
    SoraURadioStart(TARGET_RADIO);

    // configure radio parameters
    SoraURadioSetRxPA(TARGET_RADIO, SORA_RADIO_DEFAULT_RX_PA);
    SoraURadioSetRxGain(TARGET_RADIO, SORA_RADIO_DEFAULT_RX_GAIN);
    SoraURadioSetTxGain(TARGET_RADIO, SORA_RADIO_DEFAULT_TX_GAIN);
    SoraURadioSetCentralFreq(TARGET_RADIO, 2422 * 1000);           // central frequency: 2422MHz
    SoraURadioSetFreqOffset(TARGET_RADIO, -5 * 1000 * 1000);       // frequency offset: -5MHz
    SoraURadioSetSampleRate(TARGET_RADIO, 40);                     // sample rate: 40MHz

}

void Usage () {
    printf ( "umx_sample.exe [rx|tx sample_file] \n" );
}

int __cdecl main(int argc, char *argv[])
{
    BOOLEAN isTx = FALSE;

    if ( argc < 2 ) {
        Usage ();
        return 0;
    }
    if ( strcmp ( argv[1], "tx") == 0 ) {
        if ( argc < 3 ) {
            Usage ();
            return 0;
        }

        isTx = TRUE;
    }


    // Initialize Sora user mode extension
    BOOLEAN succ = SoraUInitUserExtension("\\\\.\\HWTest");
    if (!succ) 
    {
        printf("Error: fail to find a Sora UMX capable device!\n");
        return -1;
    }

    RadioConfig();
    
    if ( isTx ) {
        TxRoutine ( argv[2] );
    } else {
        RxRoutine ();
    }
    
    SoraUCleanUserExtension();

    return 0;
}
