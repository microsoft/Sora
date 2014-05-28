#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <conio.h>

#include "umxsdr.h"
#include "thread_if.h"
#include "thread_func.h"

#include "phy.h"
#include "mac.h"
#include "mgmt.h"

#include "fb11bmod_config.hpp"
#include "fb11bdemod_config.hpp"
#include "fb11amod_config.hpp"
#include "fb11ademod_config.hpp"
#include "fb11nmod_config.hpp"
#include "fb11ndemod_config.hpp"

#pragma warning(disable:4345)

#define DESC	L"Microsoft Sora HW Test Ethernet Adapter"

ULONG 				debug = 0;

// RxStream for all radios
SORA_RADIO_RX_STREAM	RxStream[MAX_RADIO_NUMBER] = { { NULL } };
BB11A_RX_CONTEXT		RxContext;	// Used by 11a baseband


PVOID				RxBuffer[MAX_RADIO_NUMBER] = { NULL };	                // Mapped Rx Buffer for 802.11a/b
ULONG				RxBufferSize = 0;
FLAG				fWork = TRUE;

PVOID				SampleBuffer[MAX_RADIO_NUMBER] = { NULL };	// Mapped Tx Buffer	
ULONG				SampleSize = _M(4); 

// Sora thread handles
HANDLE				hViterbiThread = NULL;
HANDLE				hRxThread 	   = NULL;
HANDLE				hTxThread 	   = NULL;
uintptr_t			hTxNormalThread  = NULL;

BOOLEAN ViterbiThread ( void * );
BOOLEAN ViterbiThread_11n ( void * );

typedef 
    ULONG WINAPI fnGetAdaptersAddresses(
    __in	   ULONG Family,
    __in	   ULONG Flags,
    __in	   PVOID Reserved,
    __inout  PIP_ADAPTER_ADDRESSES AdapterAddresses,
    __inout  PULONG SizePointer
    );

BOOLEAN GetMACAddress(MAC_ADDRESS* MACAddress) {

    BOOLEAN result;
    result = FALSE;
    fnGetAdaptersAddresses* GetAdaptersAddresses;
    GetAdaptersAddresses = NULL;
    IP_ADAPTER_ADDRESSES* buff;
    buff = NULL;
    HMODULE hmod;
    hmod = LoadLibrary("iphlpapi.dll");
    if (hmod) {
        GetAdaptersAddresses = (fnGetAdaptersAddresses*)GetProcAddress(hmod, 
            "GetAdaptersAddresses");
        if (GetAdaptersAddresses) {			
            ULONG size;
            ULONG err;
            size = 0;
            err = GetAdaptersAddresses(AF_INET,
                0,
                NULL,
                buff,
                &size);
            if (err != ERROR_BUFFER_OVERFLOW)
                goto error_exit;

            buff = (IP_ADAPTER_ADDRESSES*)new char[size];
            err = GetAdaptersAddresses(AF_INET,
                0,
                NULL,
                buff,
                &size); 	

            if (err != ERROR_SUCCESS)
                goto error_exit;

            IP_ADAPTER_ADDRESSES* addr;
            addr = (IP_ADAPTER_ADDRESSES*)buff;
            while(addr) {
                if (wcsncmp(addr->Description, DESC, wcslen(DESC))) {
                    addr = addr->Next;
                    continue;
                }
                MACAddress->Address[0] = addr->PhysicalAddress[0];
                MACAddress->Address[1] = addr->PhysicalAddress[1];
                MACAddress->Address[2] = addr->PhysicalAddress[2];
                MACAddress->Address[3] = addr->PhysicalAddress[3];
                MACAddress->Address[4] = addr->PhysicalAddress[4];
                MACAddress->Address[5] = addr->PhysicalAddress[5];				
                result = TRUE;
                break;
            }
        }
    }

error_exit:
    if (buff) {
        delete buff;
        buff = NULL;
    }
    if (hmod) {
        FreeLibrary(hmod);
        hmod = NULL;
    }
    return result;
}

void ConfigureRadio () 
{
    size_t iss;
    ULONG radioNo;
    const size_t nstream = GetNStream();
    for (iss = 0; iss < nstream; iss++)
    {
        // Start Radio
        radioNo = RadioNo[iss];
        SoraURadioStart (radioNo);
        SoraURadioStart (radioNo);

        Sleep (10);

        SoraURadioSetCentralFreq (radioNo, gChannelFreq * 1000);

        // Set Tx Gain to a fixed value
        SoraURadioSetTxGain ( radioNo, 0x1500 ); // 21 dBm

        // Set Freq Offset
        // SoraURadioSetFreqOffset ( radioNo, FreqOffset );
    }

    // Set the Receiver Gain
    ConfigureRxGain ( RxGain );
}

void process_kb () {
    while(1)
        if ( _kbhit () ) {
            switch(_getch()) {
            case '\3':  // _getch() will capture Ctrl+C as '\3'
            case 'x':
            case 'X':
                return;
            case '+':
            case '=':
                IncreaseGain ();
                break;
            case '-':
                DecreaseGain ();
                break;
            case '[':
                IncreaseRate ();
                break;
            case ']':
                DecreaseRate ();
                break;
            case 'a':
            case 'A':
                fAutoCfg = 1 - fAutoCfg;
                break;
            case 'c':
            case 'C':			
                RequestMgmtPkt (MGMT_PKT_AUTH) ;
                break;
            case 'd':
            case 'D':			
                RequestMgmtPkt (MGMT_PKT_ASSO);
                break;				
            case 'v':
            case 'V':			
                RequestMgmtPkt (MGMT_PKT_TEST);
                break;
            case 'p':
            case 'P':
                if ( OpMode == CLIENT_MODE ) {
                    OpMode = ADHOC_MODE;
                } else {
                    OpMode = CLIENT_MODE;
                }
            case 'w':
            case 'W':
                memset ( err_stat, 0, sizeof(err_stat));
                break;
            default:
                break;
            }
        }	else {
            Sleep (100);
            if ( fAutoCfg ) {
                AutoConfigure ();
            }

            if ( gDisplayMode == DISPLAY_PAGE ) {		
                print_status ();
            }	
        }
}

bool StartBaseband11a () {
    // Initialize the 802.11a baseband
    if (gSampleRate == 40) {
        // TX
        pBB11aTxSource = CreateModGraph11a_40M ();
        pBB11aPreambleSource = CreatePreamble11a_40M ();
        // RX
        CreateDemodGraph11a_40M(pBB11aRxSource, pBB11aViterbi, pBB11aCarrierSense);
    }
    else {
        // TX
        assert(gSampleRate == 44);
        pBB11aTxSource = CreateModGraph11a_44M ();
        pBB11aPreambleSource = CreatePreamble11a_44M ();
        // RX
        CreateDemodGraph11a_44M(pBB11aRxSource, pBB11aViterbi, pBB11aCarrierSense);
    }
    BB11aDemodCtx.Init(&RxStream[0], FrameBuffer, FrameBufferSize);

    hViterbiThread = AllocStartThread ( ViterbiThread, NULL );
    if (hViterbiThread == NULL ) {
        printf("failed to start viterbi thread\n");
        return false;
    }

    // Reset processing graphs
    pBB11aRxSource->Reset();
    pBB11aPreambleSource->Reset();
    pBB11aTxSource->Reset();

    // MAC_state
    current_state = MAC_STATE_RX;

    // start processing threads
    hTxNormalThread = _beginthread ( Dot11aSendThread, 0, NULL);
    if (hTxNormalThread == NULL ) {
        printf("failed to start tx thread\n");
        return false;
    }

    hRxThread = AllocStartThread ( Dot11aRecvProc, NULL );
    if (hRxThread == NULL) {
        printf("failed to start rx thread\n");
        return false;
    }

    return true;
}

bool StartBaseband11n () {
    // Initialize the 802.11n baseband
    // TX
    CreatePreambleGraph11n(pBB11nLPreambleSource, pBB11nHtPreambleSource);
    pBB11nSigSource = CreateSigGraph11n();
    pBB11nTxSource = CreateModGraph11n();
    // RX
    BB11nDemodCtx.Init(FrameBuffer, FrameBufferSize);
    CreateDemodGraph11n(pBB11nRxSource, pBB11nViterbi, pBB11nCarrierSense);

    // Init bricks
    IQuery *q;
    size_t found = pBB11nRxSource->TraverseGraph(&q, "TRxStream2", 1);
    assert(found);
    pRxStreams = q->QueryInterface<RxStreams>();
    pRxStreams->Init(2, RxStream);
    bool rc = pRxStreams->EstablishSync();
    if (!rc) {
        printf("failed to establish sync for all RX streams, possible hardware error\n");
        return false;
    }

    hViterbiThread = AllocStartThread ( ViterbiThread_11n, NULL );
    if (ViterbiThread_11n == NULL ) {
        printf("failed to start viterbi thread\n");
        return false;
    }

    // Reset processing graphs
    pBB11nRxSource->Reset();
    pBB11nLPreambleSource->Reset();
    pBB11nHtPreambleSource->Reset();
    pBB11nSigSource->Reset();
    pBB11nTxSource->Reset();

    // MAC_state
    current_state = MAC_STATE_RX;

    // start processing threads
    hTxNormalThread = _beginthread ( Dot11nSendThread, 0, NULL);
    if (hTxNormalThread == NULL ) {
        printf("failed to start tx thread\n");
        return false;
    }

    hRxThread = AllocStartThread ( Dot11nRecvProc, NULL );
    if (hRxThread == NULL) {
        printf("failed to start rx thread\n");
        return false;
    }

    return true;
}

bool StartBaseband11b () {
    // Initialize the 802.11b baseband
    pBB11bRxSource = CreateDemodGraph11b ();
    pBB11bTxSource = CreateModGraph11b ();
    BB11bDemodCtx.init ( &RxStream[0], FrameBuffer, FrameBufferSize );

    // Reset rx graph
    pBB11bRxSource->Reset ();

    // MAC_state
    current_state = MAC_STATE_RX;

    // Start all threads
    hTxThread = AllocStartThread ( Dot11bSendProc, NULL);
    if (hTxThread == NULL ) {
        printf("failed to start tx thread\n");
        return false;
    }

    hRxThread = AllocStartThread ( Dot11bRecvProc, &RxContext );
    if (hRxThread == NULL) {
        printf("failed to start rx thread\n");
        return false;
    }

    return true;
}

void StopBaseband () {
    StopFreeThread(hRxThread);
    StopFreeThread(hViterbiThread);
    StopFreeThread(hTxThread);

    while(Root) {
        MAC2TxID* temp;
        temp = Root;
        Root = Root->m_next;
        FreeTxResource(&temp->m_tid);
        free(temp);
    }

    IReferenceCounting::Release (pBB11bRxSource);
    IReferenceCounting::Release (pBB11bTxSource);

    IReferenceCounting::Release (pBB11aTxSource);
    IReferenceCounting::Release (pBB11aPreambleSource);
    IReferenceCounting::Release (pBB11aRxSource);

    IReferenceCounting::Release (pBB11nTxSource);
    IReferenceCounting::Release (pBB11nLPreambleSource);
    IReferenceCounting::Release (pBB11nHtPreambleSource);
    IReferenceCounting::Release (pBB11nRxSource);
}

void Dot11_main () {
    HRESULT		hr;

    InitializeListHead(&SendListHead);

    // Get local MAC
    if (GetMACAddress(&MACAddress)) {
        printf("MACAddress: %02X-%02X-%02X-%02X-%02X-%02X\n",
            MACAddress.Address[0],
            MACAddress.Address[1],
            MACAddress.Address[2],
            MACAddress.Address[3],
            MACAddress.Address[4],
            MACAddress.Address[5]);
    }
    else {
        printf("Get MACAddress failed\n");
        return;
    }

    // configure radio parameters properly
    ConfigureRadio ();

    // Map Rx Sample Buffer
    const size_t nstream = GetNStream();
    size_t iss;
    for (iss = 0; iss < nstream; iss++) {
        hr = SoraURadioMapRxSampleBuf(RadioNo[iss], &RxBuffer[iss], &RxBufferSize);
        if (FAILED(hr)) {
            printf ( "Fail to map rx buffer %d!\n", iss );
            goto error_exit;
        }

        hr = SoraURadioAllocRxStream(&RxStream[iss], iss, (PUCHAR)RxBuffer[iss], RxBufferSize);
        if (FAILED(hr)) {
            printf ( "Fail to allocate a RX stream %d!\n", iss );
            goto error_exit;
        }	

        SampleBuffer[iss] = SoraUAllocBuffer(SampleSize);
        if (!SampleBuffer[iss]) {
            printf ( "Fail to allocate Tx buffer %d!\n", iss );
            goto error_exit;
        }

        AckBuffer[iss] = SoraUAllocBuffer(AckSize);
        if (!AckBuffer[iss]) {
            printf ( "Fail to allocate ACK buffer!\n" );		
            goto error_exit;
        }
    }

    hr = SoraUEnableGetTxPacket();
    if (FAILED(hr)) {
        printf ( "Fail to enable packet reflection!\n" );
        goto error_exit;
    }

    // Start baseband 
    bool rc;
    if (gPHYMode == PHY_802_11A) {
        rc = StartBaseband11a();
    } else if (gPHYMode == PHY_802_11B){
        rc = StartBaseband11b();
    } else if (gPHYMode == PHY_802_11N){
        rc = StartBaseband11n();
    }
    if (!rc) goto error_exit;

    // enter the message loop
    process_kb ();

error_exit:

    SoraUDisableGetTxPacket();

    StopBaseband ();

    for (iss = 0; iss < nstream; iss++) {
        if (AckBuffer[iss]) {
            SoraUReleaseBuffer(AckBuffer[iss]);
            AckBuffer[iss] = NULL;
        }

        if (SampleBuffer[iss]) {
            SoraUReleaseBuffer(SampleBuffer[iss]);
            SampleBuffer[iss] = NULL;
        }

        SoraURadioReleaseRxStream(&RxStream[iss], RadioNo[iss]);
        if (RxBuffer[iss]) {
            hr = SoraURadioUnmapRxSampleBuf(RadioNo[iss], RxBuffer[iss]);
        }
    }
}
