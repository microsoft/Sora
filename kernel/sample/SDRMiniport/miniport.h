/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:
   Miniport.H

Abstract:
    This module contains NDIS miniport adapter structure definitons 
    and function prototypes.

Revision History:
    Created by senxiang, 7/Apr/2009

Notes:

--*/

#ifndef _MINIPORT_H
#define _MINIPORT_H

#define WIN9X_COMPAT_SPINLOCK
#include <ntifs.h>
#include <ntddk.h>
#pragma warning(disable:4201)  // nameless struct/union warning
#pragma warning(disable:4127) // avoid conditional expression is constant error with W4

#ifdef __cplusplus
extern "C"
{
#endif

// Need to be compatiable with sora.h
#define WIN9X_COMPAT_SPINLOCK
#include <ntifs.h>

#include <ndis.h>
#ifdef __cplusplus
}
#endif

#include <wdm.h>

#include "sora.h"

//this file defines header format for ethnet packet and wlan packet
#include "dot11_pkt.h"

#include "sdr_mac_send_queue.h"
#include "sdr_mac_recv_queue.h"

#include "sdr_phy.h"
#include "sdr_mac.h"
#include "sdr_ll.h"

#include "ioctrl.h"

#include "sora_utility.h"

#include "sdr_ll.h"

#include "sendrcv.h"

#define TX_PARAMETERS_CT                    5
#define NIC_MAXIMUM_TOTAL_SIZE              1500
#define NIC_MAX_LOOKAHEAD                   1500
#define NIC_ETH_MAX_DATA_SIZE               BB11B_MAX_TRANSMIT_UNIT

#define ETH_HEADER_SIZE                     sizeof(ETHERNET_HEADER)
#define NIC_ETH_MAXIMUM_FRAME_SIZE          NIC_ETH_MAX_DATA_SIZE
#define NIC_MAX_PACKET_SIZE                 NIC_ETH_MAXIMUM_FRAME_SIZE + ETH_HEADER_SIZE
#define NIC_LINK_SPEED                      1000000    

#define NIC_RECEIVE_BUFFER_SPACE            RX_BUFFER_SIZE
#define NIC_VENDOR_ID                       0x00FFFFFF
#define NIC_VENDOR_DRIVER_VERSION           0x00010000
#define NIC_MAX_MCAST_LIST                  32
#define NIC_802_3_MAC_OPTION                0
#define NIC_MAXIMUM_SEND_PACKETS            32
#define NIC_CURRENT_PACKET_FILTER           1
#define NIC_MAXIMUN_LIST_SZIE               32
#define NIC_802_3_MAC_OPTION                0

#define NIC_RCV_ERROR_ALIGNMENT             0
#define NIC_802_3_XMIT_ONE_COLLISION        0
#define NIC_802_3_XMIT_MORE_COLLISIONS      0
#define NIC_802_3_XMIT_DEFERRED             0
#define NIC_802_3_XMIT_MAX_COLLISIONS       0
#define NIC_802_3_RCV_OVERRUN               0
#define NIC_802_3_XMIT_UNDERRUN             0
#define NIC_802_3_XMIT_HEARTBEAT_FAILURE    0
#define NIC_802_3_XMIT_TIMES_CRS_LOST       0
#define NIC_802_3_XMIT_LATE_COLLISIONS      0

#define NIC_TX_PARAMETER_LENGTH             4

#define NIC_EXTENSION_SIZE                  10

#define NIC_TAG                             ((ULONG)'NIMV')
#define NIC_MEDIA_TYPE                      NdisMedium802_3
#define NIC_VENDOR_DESC                     "Microsoft"
#define NIC_INTERFACE_TYPE                  NdisInterfaceInternal     

#define MP_INC_REF(_A)                      NdisInterlockedIncrement(&(_A)->RefCount)
#define MP_DEC_REF(_A)                      {\
                                                NdisInterlockedDecrement(&(_A)->RefCount);\
                                                ASSERT(_A->RefCount >= 0);\
                                                if((_A)->RefCount == 0){\
                                                    NdisSetEvent(&(_A)->RemoveEvent);\
                                                }\
                                            }

#define NIC_IS_DEVICE_CAN_WORK(ADAPTER)     \
    (ADAPTER->Mac.bMacInitOK && ADAPTER->Phy.bRadiosInitOK)

#define MP_LOUD                             4
#define MP_INFO                             3
#define MP_TRACE                            2
#define MP_WARNING                          1
#define MP_ERROR                            0

#define MP_RADIOS_REQ_RETRY_INTERVAL        (-4 * 1000 * 1000 * 10) //4s

#if defined(NDIS50_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION           5
    #define MP_NDIS_MINOR_VERSION           0
#elif defined(NDIS51_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION           5
    #define MP_NDIS_MINOR_VERSION           1
#elif defined(NDIS620_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION           6
    #define MP_NDIS_MINOR_VERSION           20
    #define MP_NDIS_MAJOR_DRIVER_VERSION    2
    #define MP_NDIS_MINOR_DRIVER_VERSION    0
#else
#error Unsupported NDIS version
#endif

#define MPT_NDIS_OBJ_INIT(_phdr, _type, _rev, _size) \
    do { (_phdr)->Revision = _rev; (_phdr)->Type = _type; (_phdr)->Size = _size; } while(FALSE);

#define MP_ALLOCATE_MEMORY(pVA, szSizeInBytes, status) \
    do {pVA = ExAllocatePoolWithTag(NonPagedPool, szSizeInBytes, NIC_TAG); status = (pVA) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;} while(FALSE);

#ifdef NDIS620_MINIPORT
#define MPFreePacketOrNBLPool NdisFreeNetBufferListPool
#else
#define MPFreePacketOrNBLPool NdisFreePacketPool
#endif

typedef struct _MP_ADAPTER{
    LIST_ENTRY              List;

    LONG                    RefCount;
    NDIS_EVENT              RemoveEvent; 
    PDEVICE_OBJECT          Pdo;    //physical device object
    PDEVICE_OBJECT          Fdo;    //functional device object
    PDEVICE_OBJECT          NextDeviceObject; //next device object in device stack.
    NDIS_HANDLE             AdapterHandle;  
    
    NDIS_HANDLE             RecvPacketOrNetBufferListPoolHandle; //NDIS packet pool or net buffer list pool
    UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];
            
    SORA_THREAD             PHYInitThread; //if radios are not available, try to allocate some later.
    
    SDR_CONTEXT             SdrContext; //Software Defined Radio referenced context.
    
    LL                      Lnk; //link layer
    MAC                     Mac; //MAC
    PHY                     Phy; //PHY
    
    UEXT_KE                 KeAppExtObj;

    /* statistics for MiniportQuery*/
    struct {
        volatile LONGLONG             ullGoodTransmits;
        volatile LONGLONG             ullGoodReceives;
        volatile LONGLONG             ullDuplicatedReceives;

        volatile LONGLONG             ullTransmitFail;
        volatile LONGLONG             ullReceiveNoBuffers;
        volatile LONGLONG             ullReceiveErrors;
    };


    //TX related parameters
    //0 for DataRate
    //1 for ModSelect, 2 for PreambleType
    //3 for SampleRate, 4 for 11ADataRate
    unsigned int            TxParameters[TX_PARAMETERS_CT];

#ifdef NDIS620_MINIPORT
    //Port for NDIS6.x
    NDIS_PORT_NUMBER        PortNumber;
#endif

	HANDLE					TransferObj;

}MP_ADAPTER, *PMP_ADAPTER;

SELECTANY PMP_ADAPTER               g_pGlobalAdapter;
SELECTANY PDEVICE_OBJECT            g_pDeviceObj;
SELECTANY NDIS_HANDLE               g_NdisDeviceHandle; // From NdisMRegisterDevice

SORA_EXTERN_C BSSID g_BSSID;
SORA_EXTERN_C INT MPDebugLevel;    

#define DEBUGP(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint Fmt; \
    } \
}

SORA_EXTERN_C
NDIS_STATUS 
DriverEntry(
    PVOID DriverObject,
    PVOID RegistryPath);

SORA_EXTERN_C
NDIS_STATUS
MPSetInformation(
    IN NDIS_HANDLE                                  MiniportAdapterContext,
    IN NDIS_OID                                     Oid,
    __in_bcount(InformationBufferLength) IN PVOID   InformationBuffer,
    IN ULONG                                        InformationBufferLength,
    OUT PULONG                                      UCHARsRead,
    OUT PULONG                                      UCHARsNeeded);

SORA_EXTERN_C
NDIS_STATUS 
MPQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG UCHARsWritten,
    OUT PULONG UCHARsNeeded);

SORA_EXTERN_C
NDIS_STATUS
EthToWlan(
    IN PMDL pEthBufferList, 
    OUT PMDL* ppWlanBufferList, 
    OUT PULONG pWlanHeaderSize, 
    OUT PUCHAR* ppWlanHeaderVa, 
    IN USHORT SendSeqNo, 
    IN ULONG ulNBDataOffset);

SORA_EXTERN_C
void 
CleanWLanPacket(
    IN PUCHAR pPushedWlanHeader, 
    IN ULONG HeaderSize, 
    IN PMDL pWlanPacketBuffers);

SORA_EXTERN_C
PMDL
WlanToEth(IN PMDL pWlanFirstBuffer);

#if (defined NDIS50_MINIPORT) || (defined NDIS51_MINIPORT)

SORA_EXTERN_C
NDIS_STATUS 
MPInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext);

SORA_EXTERN_C
VOID 
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext);

SORA_EXTERN_C
VOID 
MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET Packet);

SORA_EXTERN_C
VOID 
MPSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets);

SORA_EXTERN_C
VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject);

#ifdef NDIS51_MINIPORT

SORA_EXTERN_C
VOID 
MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId);

SORA_EXTERN_C
VOID
MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength);

SORA_EXTERN_C
VOID 
MPShutdown(
    IN NDIS_HANDLE MiniportAdapterContext);

#endif

#else

SORA_EXTERN_C MINIPORT_INITIALIZE MPInitialize;

SORA_EXTERN_C MINIPORT_HALT MPHalt;

SORA_EXTERN_C MINIPORT_UNLOAD MPUnload;

MINIPORT_PAUSE MiniportPause;
MINIPORT_RESTART MiniportRestart;
MINIPORT_SHUTDOWN MPShutdown;

SORA_EXTERN_C MINIPORT_DEVICE_PNP_EVENT_NOTIFY MPPnPEventNotify;

MINIPORT_OID_REQUEST MPOidRequest;
MINIPORT_CANCEL_OID_REQUEST MPCancelOidRequest;

MINIPORT_SEND_NET_BUFFER_LISTS MPSendNetBufferLists;
MINIPORT_CANCEL_SEND MPCancelSendNetBufferLists;
MINIPORT_RETURN_NET_BUFFER_LISTS MPReturnNetBufferLists;

MINIPORT_CHECK_FOR_HANG MiniportCheckForHangEx;

SORA_EXTERN_C MINIPORT_RESET MiniportResetEx;

SORA_EXTERN_C
NDIS_STATUS
MPDirectOidRequest(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNDIS_OID_REQUEST       NdisOidRequest);

SORA_EXTERN_C
VOID 
MPCancelDirectOidRequest(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PVOID                   RequestId);

SORA_EXTERN_C
MINIPORT_SET_OPTIONS MiniportSetOptions;

SORA_EXTERN_C
VOID
MpQuerySupportedOidsList(
    IN OUT PNDIS_OID *SupportedOidList,
    IN OUT PULONG SupportedOidListLength);

SORA_EXTERN_C
NDIS_STATUS
NICRegisterDevice(VOID);

SORA_EXTERN_C
VOID FreeNdisPort(IN PMP_ADAPTER Adapter);

#endif

SORA_EXTERN_C
NTSTATUS MPDeviceIoControl(IN PDEVICE_OBJECT pDevObj,IN PIRP pIrp);


SORA_EXTERN_C
VOID FreeAdapter(IN PMP_ADAPTER pAdapter);

#endif

