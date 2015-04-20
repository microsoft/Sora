#ifndef _UEXTK_H
#define _UEXTK_H

#define SYM_FILE_MAX_PATH_LENGTH     124

#ifndef USER_MODE

#include <ndis.h>
#include "_enlist.h"

struct PROCESS_OBJECT_MONITOR;

typedef struct _TXCB
{
    PACKET_BASE Base;
    LIST_ENTRY  TXCBList;
    ULONG       PacketID;
    WCHAR       SymFileName[SYM_FILE_MAX_PATH_LENGTH];
} TXCB, *PTXCB, **PPTXCB;

typedef struct _QUEUED_RW_LOCK
{
    LIST_ENTRY  WaitingList;
    LONG        RWSummary; //0: free, n: read count, -1: write
    KSPIN_LOCK  Lock;
} QUEUED_RW_LOCK, *PQUEUED_RW_LOCK;

typedef struct _BUF_MAP_MGR
{
    KSPIN_LOCK              BufferMapLock;
    ULONG                   FreeBitMap;
    PMDL                    BufferMDL[MAX_APP_EXT_NUM];
    PVOID                   BufferUserSpaceVA[MAX_APP_EXT_NUM];
    HANDLE					UserProcess[MAX_APP_EXT_NUM];
}BUF_MAP_MGR, *PBUF_MAP_MGR;

typedef struct _TX_SIGNAL_QUEUE
{
    NDIS_SPIN_LOCK          QueueLock;
    LIST_ENTRY              CanTxQueue;
	char 					Inited;
    NPAGED_LOOKASIDE_LIST   TxCBLookaside;
    ULONG                   IDSeqCtrl; //sequence number
    volatile ULONG          fStopTX;
}TX_SIGNAL_QUEUE, *PTX_SIGNAL_QUEUE;

typedef struct _UEXT_KE{

	struct PROCESS_OBJECT_MONITOR* Monitor;
	
    QUEUED_RW_LOCK          QueuedRxTxLock;
    BUF_MAP_MGR             RXBufMapMgr;
    BUF_MAP_MGR             TXBufMapMgr;

    TX_SIGNAL_QUEUE         TxQueue;

	KSPIN_LOCK SendPacketLock;
	struct thread_safe_enlist* SendPacketList;
	PKSEMAPHORE SendPacketControl;
}UEXT_KE, *PUEXT_KE;

void SoraKInitQueuedRWLock(PQUEUED_RW_LOCK RWLock);
HRESULT SoraKAcquireRXLock(PQUEUED_RW_LOCK RWLock);
void SoraKReleaseRXLock(PQUEUED_RW_LOCK RWLock);
HRESULT SoraKAcquireTXLock(PQUEUED_RW_LOCK RWLock);
void SoraKReleaseTXLock(PQUEUED_RW_LOCK RWLock);

void SoraKUExtKeCtor(PUEXT_KE  UExtKeObj);
void SoraKUExtKeDtor(PUEXT_KE  UExtKeObj, HANDLE TransferObj);
void SoraKUExtKeInit(IN PUEXT_KE UExtKeObj);

void SoraKTxQueueInit(IN  PTX_SIGNAL_QUEUE  TxQueue);
void SoraKTxQueueClean(IN  PTX_SIGNAL_QUEUE  TxQueue, PSORA_RADIO Radio);

VOID SoraKUExtKFreeBufMapSlot(PBUF_MAP_MGR  mgr, LONG index);
LONG SoraKUExtKGetBufMapSlot(PBUF_MAP_MGR  mgr);

void _UnmapBuffer(PVOID buf, PMDL mdl, HANDLE pid);

#if 0
void _UnmapBuffer(PBUF_MAP_MGR  mgr, LONG i);
#endif 

HRESULT SoraKMapBuf2User(PBUF_MAP_MGR mgr, ULONG Slot, PVOID SysBuf, ULONG Size);

void SoraKUExtKMapModBuffer(PUEXT_KE ExtObj, HANDLE TransferObj, PMAP_MOD_BUF_OUT Out);
HRESULT SoraKUExtKUnmapModBuffer(PUEXT_KE ExtObj, HANDLE TransferObj, PVOID UserBuffer);

void 
SoraKUExtKTxResAlloc(
    HANDLE TransferObj,
    PTX_SIGNAL_QUEUE TxQueue,
    ULONG SampleSize, 
    OUT PTX_RES_ALLOC_OUT Output);

PTXCB
SoraKAllocTXCB(
	HANDLE TransferObj,
	PTX_SIGNAL_QUEUE TxQueue,
	ULONG SampleSize);

ULONG AssignTxID(PTX_SIGNAL_QUEUE TxQueue);
PTXCB GetTxCBByID(PTX_SIGNAL_QUEUE TxQueue, ULONG sid);
PTXCB RemoveTxCBByID(PTX_SIGNAL_QUEUE TxQueue, ULONG sid);
HRESULT SoraKTxSignalByID(PTX_SIGNAL_QUEUE TxQueue, PSORA_RADIO Radio, ULONG sid, ULONG Times);

HRESULT 
SoraKMimoTxSignalByID(
	PTX_SIGNAL_QUEUE TxQueue, 
	PSORA_RADIO Radios2[MAX_RADIO_NUMBER], 
    ULONG* radio,
    ULONG* sid, 
    ULONG count,
    ULONG Times);

HRESULT SoraKTxDoneByID(PTX_SIGNAL_QUEUE TxQueue, HANDLE TransferObj, ULONG sid);

void SoraKUExtKAllocVStreamMask(PUEXT_KE ExtObj, PSORA_RADIO Radio, PVSTREAM_MASK_ALLOC_OUT Output);
HRESULT 
SoraKUExtKReleaseVStreamMask(
    PUEXT_KE ExtObj,
    PSORA_RADIO Radio, 
    ULONG VStreamMask);

HRESULT 
SoraKUExtKUnmapRxBuf(
    PUEXT_KE ExtObj,
    PSORA_RADIO Radio, 
    PVOID UserBuffer);

void SoraKUExtKMapRxBuf(PUEXT_KE ExtObj, PSORA_RADIO Radio, PMAP_RX_BUF_OUT Output);

ULONG 
SoraKHandleUExtReq(
    PUEXT_KE UExtKeObj, 
    HANDLE TransferObj,
    PSORA_RADIO Radios2[MAX_RADIO_NUMBER],
    ULONG code,
    PVOID Input, 
    ULONG InSize, 
    PVOID Output, 
    ULONG OutSize);

// just force to do it, don't care about owner process
HRESULT 
SoraKUExtKForceUnmapModBuffer(
	PUEXT_KE ExtObj, 
	PSORA_RADIO Radio, 
	PVOID UserBuffer);

// just force to do it, don't care about owner process
HRESULT 
SoraKUExtKForceUnmapRxBuf(
    PUEXT_KE ExtObj,
    PSORA_RADIO Radio, 
    PVOID UserBuffer);

struct ALLOC_KERNEL_BUFFER_DATA {

	ULONG m_size;
	PVOID m_kbuf;
	PVOID m_ubuf;
	PMDL m_mdl;
	BOOLEAN m_is_physical_continous;
	HANDLE m_map_process;
};

HRESULT 
SoraKUExtKAllocKernelBuffer(
	PUEXT_KE ExtObj,
	ULONG Size,
	struct ALLOC_KERNEL_BUFFER_DATA** alloc_kb);

NTSTATUS 
SoraKAllocateTransferObj(
	HANDLE* TransferObj);

void
SoraKFreeTransferObj(
	HANDLE TransferObj);

#endif
#endif
