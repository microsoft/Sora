/*++
Copyright (c) Microsoft Corporation

Module Name: Sora Radio manager 

Abstract: This header file defines structs, macros, interfaces to manage Radio attached
          to PCIE board. Management operation includes radio init, destroy, assignment 
          and release. 
--*/

#ifndef _RADIO_MANAGER
#define _RADIO_MANAGER

#include "_reg_conf.h"

typedef struct __PACKET_BASE *PPACKET_BASE;

#define __RADIO_STATUS_ALIVE        0x00000001

typedef struct __SORA_RADIO_CONFIG
{
    ULONG   Reserved1;
    ULONG   Reserved2;
}SORA_RADIO_CONFIG, *PSORA_RADIO_CONFIG;

typedef struct __SORA_RADIO_STATUS
{
    volatile LONG   __fFree;
    PCWSTR          __upperDriverName;
    ULONG           __fDumpMode;
    ULONG           __Flags; // only for Radio hardware status;
} __SORA_RADIO_STATUS, *__PSORA_RADIO_STATUS;

// IOCTL Codes
#define __IOCTL_REQUEST_RADIOS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_DUMP_RADIO_RX \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_OUT_DIRECT, FILE_READ_DATA)

#define __IOCTL_RELEASE_RADIOS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_NEITHER, FILE_ANY_ACCESS)

#define __IOCTL_MAP_SORA_REGISTER \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define __IOCTL_ACQUIRE_RADIO_MANAGER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_NEITHER, FILE_ANY_ACCESS)

#define __IOCTL_RELEASE_RADIO_MANAGER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_NEITHER, FILE_ANY_ACCESS)

#define __IOCTL_RELEASE_RESOURCE \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
	
#define SORA_GET_RADIO_FROM_LIST_ENTRY(pEntry) \
    CONTAINING_RECORD(pEntry, SORA_RADIO, RadiosList)

#define SORA_GET_RX_QUEUE(pRadio)    \
    (&(pRadio)->__rx_queue)

#define SORA_GET_RX_STREAM(pRadio) \
    (&(pRadio)->__RxStream)

#define SORA_RADIO_WELL_CONFIGED2(pRadio) \
    ((NULL != (pRadio)->pTxResMgr) && \
     (NULL != (pRadio)->__rx_queue.__pVirtualStartAddress))

#define SORA_RADIO_STARTED(pRadio)     ((pRadio)->__fCanWork)

#define SORA_RADIO_GET_RX_GAIN(pRadio) \
    ((pRadio)->__uRxGain)

#define SORA_RADIO_GET_RX_PA(pRadio)    \
    ((pRadio)->__uRxPa)

#define SORA_RADIO_GET_TX_GAIN(pRadio) \
    ((pRadio)->__uTxGain)

#define SORA_RADIO_IS_ALIVE(pRadio) \
    ((pRadio)->__status.__Flags & __RADIO_STATUS_ALIVE)


/*
    SORA_RADIO defines the basic abstract to a hardware radio.
    A Sora radio contains mainly three parts:
    1) A Control channel - control registers
    2) A Rx channel - Rx queues, further wrapped as rx_stream
    3) A Tx channel - Tx buffer, further with Tx resources.
 */
typedef struct __SORA_RADIO
{
    LIST_ENTRY                      RadiosList;


    // control registers
    __HW_REGISTER_FILE              __ctrl_reg; 

    // RX Channel
    __RX_QUEUE_MANAGER              __rx_queue;

    // Reference to shared Tx Resource manager
    PTX_RM                          pTxResMgr;

    ULONG                           __radio_no; //radio index
    ULONG                           RadioID;    //unique radio id
    
    __SORA_RADIO_STATUS             __status;   // radio status
    ULONG                           __uRxGain;
    ULONG                           __uRxPa;
    ULONG                           __uTxGain;
    
    KSPIN_LOCK                      __HWOpLock;//DMA upload and TX lock
    LONG                            __Lock;

    KEVENT                          ForceReleaseEvent;
    KEVENT                          PnPEvent;

    /* Context - usually linked to a PHY bond on the radio */
    PVOID                    	    __pContextExt; 
    
    // RX_STREAM to access Rx queue DMA Buf
    SORA_RADIO_RX_STREAM            __RxStream;

    ULONG                           __fCanWork;
    volatile BOOLEAN                __fRxEnabled;

/*
    // TX Channel - Tx Sample Buffer
    PTXSAMPLE                       __TxSampleBufVa;    
    PHYSICAL_ADDRESS                __TxSampleBufPa;
    ULONG                           __TxSampleBufSize;
    LONG                           __TxBufLock;    // Lock to access the Tx sample buffer    
*/

	BOOL 							__initialized;
	
    //FAST_MUTEX                      __ModSampleBufMutex;    
} SORA_RADIO, *PSORA_RADIO, **PPSORA_RADIO, __SORA_RADIO, *__PSORA_RADIO;



typedef struct ___SORA_RADIO_MANAGER *__PSORA_RADIO_MANAGER, *PSORA_RADIO_MANAGER;

HRESULT SoraInitRadioManager2(
    IN PHYSICAL_ADDRESS         MemStartAddr,
    IN ULONG                    nMemSegLength,
    OUT __PSORA_RADIO_MANAGER   pRadioMgr
    );


HRESULT SoraCleanupRadioManager2(IN OUT __PSORA_RADIO_MANAGER pRadioMgr);

/*
 *   Radio manuplication APIs
 */
HRESULT 
SORAAPI
SoraAllocateRadioFromDevice(
    IN PCWSTR           RCBDeviceName,
    IN OUT LIST_ENTRY   *pRadios,
    IN ULONG            nRadio,
    IN PCWSTR           upperDevName);

__inline 
HRESULT 
SORAAPI
SoraAllocateRadioFromRCBDevice(
    IN OUT LIST_ENTRY   *pRadios,
    IN ULONG            nRadio,
    IN PCWSTR           userName)
{
    return SoraAllocateRadioFromDevice(SORA_DEVICE_NAME, pRadios, nRadio, userName);
}


VOID SORAAPI SoraReleaseRadios(IN LIST_ENTRY   *pRadiosHead);

HRESULT 
SORAAPI
SoraRadioInitialize(
    IN OUT PSORA_RADIO          pRadio, 
    IN PVOID                    pReserved,
    IN ULONG                    nTxSampleBufSize,
    IN ULONG                    nRxSampleBufSize);

HRESULT 
SORAAPI
SoraRadioStart(
    IN OUT PSORA_RADIO      pRadio, 
    IN ULONG                RXGain,
    IN ULONG                TXGain, 
    IN PSORA_RADIO_CONFIG   pConfig );

/*++
SoraRadioGetModulateBuffer returns the modulate buffer of the radio.

Note: Before modulating packet, application driver must call it to get 
      the buffer to hold the result samples. If the radio is not well 
      configured, it returns NULL.
--*/
/*
__inline PVOID SORAAPI SoraRadioGetModulateBuffer(IN PSORA_RADIO pRadio)
{
	return pRadio->__TxSampleBufVa;
}

__inline ULONG SORAAPI SoraRadioGetModulateBufferSize(IN PSORA_RADIO pRadio)
{
	return pRadio->__TxSampleBufSize;
}
*/

__inline PSORA_RADIO SORAAPI SoraGetRadioFromList(LIST_ENTRY *pRadiosHead, ULONG index)
{
    PSORA_RADIO pRadio = NULL;
    LIST_ENTRY  *pEntry = pRadiosHead->Flink;

    while (pEntry != pRadiosHead)
    {
        if (0 == index)
        {
            pRadio = CONTAINING_RECORD(pEntry, SORA_RADIO, RadiosList);
            break;
        }
        else
        {
            index--;
            pEntry = pEntry->Flink;
        }
    }
    return pRadio;
}

__inline 
BOOLEAN SORAAPI SoraRadioCheckStartState(LIST_ENTRY *pRadiosHead)
{
    LIST_ENTRY  *pEntry;
    PSORA_RADIO pRadio;

    FOREACH(pRadiosHead, pEntry)
    {
        pRadio = CONTAINING_RECORD(pEntry, SORA_RADIO, RadiosList);
        if (!(pRadio->__fCanWork))
        {
            return FALSE;
        }
    }
    return TRUE;
}

__inline BOOLEAN SORAAPI SoraRadioCheckRxState(PSORA_RADIO pRadio)
{
    return pRadio->__fRxEnabled;
}

__inline 
PSORA_RADIO_RX_STREAM SORAAPI SoraRadioGetRxStream(PSORA_RADIO pRadio)
{
    return &pRadio->__RxStream;
}

VOID SoraStopRadio2(IN OUT PSORA_RADIO pRadio);

#endif 
