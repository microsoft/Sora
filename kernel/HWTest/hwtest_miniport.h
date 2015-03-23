/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

   hwtest_miniport.h

Abstract:
    This module contains system independant structure definitons and 
    function prototypes for HWTest.

Revision History:

Notes:

--*/

#ifndef _HWTEST_MINIPORT_H
#define _HWTEST_MINIPORT_H

#include "sora.h"
#include <ndis.h>
#include "__reg_file.h"
#include "__tx_res.h"
#include "__radio_man_internal.h"
#include "__rx_internal.h"

#include "dut_entries.h"
#include "hwtest_ioctrl.h"

#include "_UExtK.h"
#include "resource_collection.h"

#define E_TEST_NOT_ALLOWED                  ((HRESULT)0x80000001)

#define NIC_TAG                             ((ULONG)'DTWH')
#define NIC_MEDIA_TYPE                      NdisMedium802_3
#define NIC_VENDOR_DESC                     "Microsoft"
#define NIC_INTERFACE_TYPE                  NdisInterfaceInternal     

#define NIC_RESOURCE_BUF_SIZE               (sizeof(NDIS_RESOURCE_LIST) + \
                                            (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))

#define LINKNAME_STRING     L"\\DosDevices\\HWTest"
#define NTDEVICE_STRING     L"\\Device\\HWTest"

#define HWTEST_USERNAME     L"HwtestMiniport"

#define MP_LOUD    4
#define MP_INFO    3
#define MP_TRACE   2
#define MP_WARNING 1
#define MP_ERROR   0

#define MDL_BUFFER_SIZE             (512 * 1024)
#define RX_BUFFER_SIZE              16*1024*1024
#define RX_BUFFER_NUM               1
#define MODULATE_BUFFER_SIZE        (2 * 1024 * 1024)

#define HWT_DEFAULT_CENTRAL_FREQ    (2422 * 1000) //kHz


typedef struct _MP_GLOBAL_DATA
{
    LIST_ENTRY      AdapterList;
    NDIS_SPIN_LOCK  Lock;
} MP_GLOBAL_DATA, *PMP_GLOBAL_DATA;

ULONG AssignTxID(PTX_SIGNAL_QUEUE TxQueue);

typedef struct _HWT_ADAPTER{
    LIST_ENTRY              List;
	char 					Attached;

#if defined (NDIS620_MINIPORT)
    NDIS_PORT_NUMBER        PortNumber;  //miniport ndis 6 special
#else
    PDEVICE_OBJECT          Pdo; //miniport ndis 5x special
    PDEVICE_OBJECT          Fdo; //miniport ndis 5x special
    PDEVICE_OBJECT          NextDeviceObject; //miniport ndis 5x special
#endif

    NDIS_HANDLE             AdapterHandle;    
    UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];

#if 0
    LIST_ENTRY              Radios;
    PSORA_RADIO             Radio;
#endif

	PSORA_RADIO_MANAGER     SoraRadioManager;
	__PSORA_REGISTERS		SoraRegs;
	TRANSFER_OBJ			TransferObj;
	PSORA_RADIO				Radios2[MAX_RADIO_NUMBER];

    SORA_ETHREAD            RxThread;
    volatile ULONG          fDump;

    //NDIS_SPIN_LOCK          QueueLock;
    //LIST_ENTRY              CanTxQueue;
    //NPAGED_LOOKASIDE_LIST   TxCBLookaside;
    //ULONG                   IDSeqCtrl; //sequence number
    //volatile ULONG          fStopTX;

    //-----------------------------------
    HWT_INFO                Infos;

    //Application Extension
    UEXT_KE                 UExtKeObj;
#if NDIS_SUPPORT_NDIS6
	NDIS_HANDLE				AllocateNetBufferListPool;
#else
	NDIS_HANDLE				AllocatePacketPool;
	NDIS_HANDLE				AllocateBufferPool;
#endif

	HANDLE					Thread;

	LONG					Reference;

	UCHAR 					Removed;
	KEVENT					Exit;

	PVOID 					WrapperPageVAddr;
	PHYSICAL_ADDRESS		WrapperPagePAddr;
	LONG					WrapperPageLock;
	
}HWT_ADAPTER, *PHWT_ADAPTER;

extern INT MPDebugLevel;    

#define DEBUGP(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint Fmt; \
    } \
}

extern MP_GLOBAL_DATA  GlobalData;
extern PDEVICE_OBJECT  g_ControlDeviceObject;
extern NDIS_HANDLE     g_NdisDeviceHandle;

NDIS_STATUS
MpAllocateAdapter(
    __in  NDIS_HANDLE             AdapterHandle,
    __deref_out_opt PHWT_ADAPTER* Adapter
    );

void MpFreeAdapter(PHWT_ADAPTER pAdapter);

void NICCleanAdapter(Adapter);

NDIS_STATUS NICInitializeAdapter(
    IN  PHWT_ADAPTER  Adapter,
    IN  NDIS_HANDLE  WrapperConfigurationContext
    );//initialization

void NICAttachAdapter(PHWT_ADAPTER Adapter);
void NICDettachAdapter(PHWT_ADAPTER Adapter);

NDIS_STATUS NICDeregisterDevice();
NDIS_STATUS NICRegisterDevice(VOID);
NTSTATUS NICDispatch(
            IN PDEVICE_OBJECT           DeviceObject,
            IN PIRP                     Irp
            );
HRESULT NICHandleHwTestIRP(
                           PHWT_ADAPTER Adapter, 
                           PVOID Command, 
                           ULONG Length, 
                           PVOID outBuffer, 
                           ULONG OutSize,
                           PULONG pOutputLen);

HRESULT NICTestFirstRadioTx(PHWT_ADAPTER Adapter);

HRESULT NICUnitTestPhyCache(PHWT_ADAPTER Adapter);

HRESULT NICTestFirstRadioTransfer(PHWT_ADAPTER Adapter, const int length, const char **argv);

//void NICTestFirstRadioTxDone(PHWT_ADAPTER Adapter);

HRESULT NICTestFirstRadioRX(
                            PHWT_ADAPTER Adapter, 
                            ULONG nBlockNum, 
                            BOOLEAN fResetPhy, 
                            char *buffer,
                            ULONG size, 
                            PULONG pOutputSize);

void NICTestFirstRadioStopRx(PHWT_ADAPTER Adapter);

HRESULT NICUnitTestTxAck(PHWT_ADAPTER Adapter);

void NICTestFirstRadioEnableRx(PHWT_ADAPTER Adapter);

HRESULT NICTestModulateTransfer(PHWT_ADAPTER Adapter);



ULONG 
SerialTXCacheInfo(
    PID_FILE_PAIR PairBuffer, 
    ULONG Count, 
    PHWT_ADAPTER Adapter);

PTXCB RemoveTxCBByID(PTX_SIGNAL_QUEUE TxQueue, ULONG sid);

PTXCB GetTxCBByID(PTX_SIGNAL_QUEUE TxQueue, ULONG sid);

void CleanQueuedSendPacket(PHWT_ADAPTER Adapter);

#endif
