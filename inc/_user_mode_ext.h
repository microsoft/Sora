#ifndef _USER_MODE_EXT_H
#define _USER_MODE_EXT_H

#ifdef USER_MODE
#include <winioctl.h> //CTL_CODE
#endif

#include "thread_if.h"

#define MAX_APP_EXT_NUM         32

typedef __int64	PACKET_HANDLE;

#define E_BUF_MAP_EXCEEDED      ((HRESULT)0x800a0001L)
#define E_LOCK_WAIT_TIMEOUT     ((HRESULT)0x800a0002L)
#define E_FILE_UNKNOWN          ((HRESULT)0x800a6001)
#define E_CONTEXT_ERR           ((HRESULT)0x800a6002)
#define E_UNKOWN_ID             ((HRESULT)0x800a6003)

typedef enum _APP_EXT_CMD
{
    MAP_MOD_BUFFER = 0,
    UNMAP_MOD_BUFFER,
    TX_RES_ALLOC,
    TX_RES_RELEASE,
    TX_SIGNAL,
    PARALLEL_TX_AND_ALLOC,
    TX_LOCK_ACQUIRE,
    TX_LOCK_RELEASE,
    VSTREAM_MASK_ALLOC,
    VSTREAM_MASK_RELEASE,
    MAP_RX_BUFFER, 
    UNMAP_RX_BUFFER,
    RX_LOCK_ACQUIRE,
    RX_LOCK_RELEASE,
    MOD_BUF_LOCK_ACQUIRE,
    MOD_BUF_LOCK_RELEASE,
    RADIO_START,
    SET_CEN_FREQ,
    SET_FREQ_OFF,
    SET_RX_PA,
    SET_RX_GAIN, 
    SET_TX_GAIN, 
    SET_SAMPLE_RATE,
    WRITE_RADIO_REG,
    READ_RADIO_REG,
    INDICATE_PACKET,
    ALLOC_KERNEL_BUFFER,
    RELEASE_KERNEL_BUFFER,
    REDIRECT_IRP_MJ_CREATE,
    REDIRECT_IRP_MJ_CLOSE,
    GET_SEND_PACKET,
    COMPLETE_SEND_PACKET,
    ENABLE_GET_SEND_PACKET,
	TRANSFER_CONTINUOUS,
	TRANSFER_DISCONTINUOUS,	
	MIMO_TX,
} APP_EXT_CMD;

#define UEXT_CMD(cmd) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900 + (cmd),METHOD_BUFFERED,FILE_ANY_ACCESS)

#define UEXT_CMD_DIRECTIN(cmd) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900 + (cmd), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#ifdef USER_MODE

BOOLEAN SoraUInitUserExtension(const char * szDevName);

VOID SoraUCleanUserExtension();

HRESULT SoraURadioStart(IN ULONG RadioNo);

HRESULT 
SoraURadioMapTxSampleBuf(
    IN ULONG RadioNo, 
    OUT PVOID *ppUserBuffer, 
    OUT PULONG Size);

HRESULT SoraURadioUnmapTxSampleBuf(IN ULONG RadioNo, IN PVOID UserBuffer);

HRESULT 
SoraURadioTransfer(
    IN ULONG RadioNo, 
    IN ULONG SampleSize,
    OUT PULONG TxID);

HRESULT 
SoraURadioTransferEx(
    IN ULONG RadioNo,
    IN PVOID SampleBuffer,
    IN ULONG SampleSize,
    OUT PULONG TxID);

HRESULT SoraURadioTxFree(IN ULONG RadioNo, IN ULONG TxID);

HRESULT SoraURadioTx(IN ULONG RadioNo, IN ULONG TxID);

HRESULT SoraURadioMimoTx(IN ULONG* RadioNo, IN ULONG* TxID, ULONG Count);

HRESULT 
SoraURadioMapRxSampleBuf(
    ULONG RadioNo, 
    OUT PVOID *ppUserBuffer,
    OUT PULONG Size);

HRESULT 
SoraURadioUnmapRxSampleBuf(
    IN ULONG RadioNo,
    IN PVOID UserBuffer);

HRESULT 
SoraURadioSetRxPA(
    IN ULONG RadioNo,
    IN ULONG RxPa);

HRESULT 
SoraURadioSetRxGain(
    IN ULONG RadioNo,
    IN ULONG RxGain);

HRESULT
SoraURadioSetTxGain(
    IN ULONG RadioNo,
    IN ULONG TxGain);

HRESULT
SoraURadioSetCentralFreq(
    IN ULONG RadioNo,
    IN ULONG KHz);

HRESULT
SoraURadioSetFreqOffset(
    IN ULONG RadioNo,
    IN LONG Hz);

HRESULT SoraURadioSetSampleRate(IN ULONG RadioNo, ULONG MHz);

HRESULT SoraUWriteRadioRegister(ULONG RadioNo, ULONG Addr, ULONG  Value);
HRESULT SoraUReadRadioRegister(ULONG RadioNo, ULONG Addr, ULONG* Value); 

HRESULT SoraUIndicateRxPacket(UCHAR* Buffer, ULONG BufferLength); 

HANDLE SoraUThreadAlloc();
VOID SoraUThreadFree(HANDLE Thread);
BOOLEAN SoraUThreadStart(HANDLE Thread, PSORA_UTHREAD_PROC User_Routine, PVOID User_Context);
BOOLEAN SoraUThreadStartEx(HANDLE Thread, PSORA_UTHREAD_PROC* User_Routine, PVOID* User_Context, ULONG Count, SCHEDULING_TYPE Type);
VOID SoraUThreadStop(HANDLE Thread);

PVOID SoraUAllocBuffer(ULONG Size);
PVOID SoraUForceAllocContinuousBuffer(ULONG Size);
VOID SoraUReleaseBuffer(PVOID Buff);

HRESULT SoraUGetTxPacket(PACKET_HANDLE* Packet, VOID** Addr, UINT* Length, DWORD Timeout);
HRESULT SoraUCompleteTxPacket(PACKET_HANDLE Packet, HRESULT hResult);
HRESULT SoraUEnableGetTxPacket();
HRESULT SoraUDisableGetTxPacket();

#endif

///
/// IO ctrl output structure
///

#pragma pack(push)
#pragma pack(8)

typedef struct _MAP_MOD_BUF_OUT
{
    HRESULT     hResult;
	union {
	    PVOID       ModBuf;
		__int64		ModBuf_Reserved64;
	};
    ULONG       ModBufSize;
} MAP_MOD_BUF_OUT, *PMAP_MOD_BUF_OUT;

typedef struct _UNMAP_MOD_BUF_IN
{
    ULONG       RadioNo;
	union {
	    PVOID       ModBuf;
		__int64		ModBuf_Reserved64;
	};
}UNMAP_MOD_BUF_IN, *PUNMAP_MOD_BUF_IN;

typedef struct _TX_RES_ALLOC_IN
{
    ULONG       RadioNo;
    ULONG       SampleSize;
} TX_RES_ALLOC_IN, *PTX_RES_ALLOC_IN;

typedef struct _TRANSFER_EX_IN_ {

	union {	
		PVOID 		Buffer;
		__int64		Buffer_Reserved64;
	};
	ULONG Size;
} TRANSFER_EX_IN, *PTRANSFER_EX_IN;

typedef struct _TX_RES_ALLOC_OUT
{
    HRESULT     hResult;
    ULONG       TxID;
}TX_RES_ALLOC_OUT, *PTX_RES_ALLOC_OUT, TRANSFER_EX_OUT;

typedef struct _TX_IN
{
    ULONG       RadioNo;
    ULONG       TxID;
}TX_IN, TX_RES_REL_IN, *PTX_IN, *PTX_RES_REL_IN; 

typedef struct _VSTREAM_MASK_ALLOC_IN
{
    ULONG       RadioNo;
}VSTREAM_MASK_ALLOC_IN, *PVSTREAM_MASK_ALLOC_IN;

typedef struct _VSTREAM_MASK_ALLOC_OUT
{
    HRESULT     hResult;
    ULONG       VStreamMask;
}VSTREAM_MASK_ALLOC_OUT, *PVSTREAM_MASK_ALLOC_OUT;

typedef struct _VSTREAM_MASK_RELEASE_IN
{
    ULONG       RadioNo;
    ULONG       VStreamMask;
}VSTREAM_MASK_RELEASE_IN, *PVSTREAM_MASK_RELEASE_IN;

typedef struct _MAP_RX_BUF_OUT
{
    HRESULT     hResult;
	union {
	    PVOID       RxBuf;
		__int64		RxBuf_Reserved64;
	};
    ULONG       RxBufSize;
}MAP_RX_BUF_OUT, *PMAP_RX_BUF_OUT;

typedef struct _UNMAP_RX_BUF_IN
{
    ULONG       RadioNo;
	union {
	    PVOID       UserBuffer;
		__int64		UserBuffer_Reserved64;
	};
}UNMAP_RX_BUF_IN, *PUNMAP_RX_BUF_IN;

typedef struct _SET_VALUE_IN
{
    ULONG RadioNo;
    union{
        ULONG   uValue;
        LONG    iValue;
    }u;
} SET_VALUE_IN, *PSET_VALUE_IN;

typedef struct _RADIO_REGISTER_IO {

	ULONG radio;
	s_uint32 addr;
	union {
		s_uint32 out;
		s_uint32 in;		
	} value;
	HRESULT hr;
} RADIO_REGISTER_IO, *PRADIO_REGISTER_IO;

typedef struct _ALLOC_KERNEL_BUFFER_IN_ {

	ULONG Size;
	BOOLEAN force_Continuous;
} ALLOC_KERNEL_BUFFER_IN, *PALLOC_KERNEL_BUFFER_IN;

typedef struct _ALLOC_KERNEL_BUFFER_OUT_ {

	union {
		PVOID 		Buff;
		__int64		Buff_Reserved64;
	};
	ULONG HiPhyBuff;
	ULONG LoPhyBuff;
	BOOLEAN is_Continuous;
} ALLOC_KERNEL_BUFFER_OUT, *PALLOC_KERNEL_BUFFER_OUT;

typedef struct _RELEASE_KERNEL_BUFFER_IN_ {

	union {
		PVOID 		Buff;
		__int64		Buff_Reserved64;
	};
} RELEASE_KERNEL_BUFFER_IN, *PRELEASE_KERNEL_BUFFER_IN;

typedef struct _GET_SEND_PACKET_OUT {

	HRESULT hResult;
	PACKET_HANDLE	Packet;
	union {
		PVOID 		Addr;
		__int64		Addr_Reserved64;
	};
	UINT Size;
} GET_SEND_PACKET_OUT, *PGET_SEND_PACKET_OUT;

typedef struct _COMPLETE_SEND_PACKET_IN {

	HRESULT hResult;
	PACKET_HANDLE	Packet;
} COMPLETE_SEND_PACKET_IN, *PCOMPLETE_SEND_PACKET_IN;

typedef struct _ENABLE_GET_SEND_PACKET_IN {

	union {
		HANDLE 		Semaph;
		__int64		Semaph_Reserved64;
	};
} ENABLE_GET_SEND_PACKET_IN, *PENABLE_GET_SEND_PACKET_IN;

typedef struct _RADIO_START_IN {

	ULONG RadioNo;

} RADIO_START_IN, *PRADIO_START_IN;

typedef struct _TIMES_ID_PAIRS
{
	ULONG 		Count;
	ULONG		RadioNo[8];    
    ULONG       PacketID[8];
	ULONG       Times;
} TIMES_ID_PAIRS, *PTIMES_ID_PAIRS;

#pragma pack(pop)

#endif
