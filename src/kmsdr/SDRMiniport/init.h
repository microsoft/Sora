/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:
   Init.h

Abstract:
    This file defines function for adapter initialization and async radio request.

History:
    Created by yichen, 23/Apr/2009
    Modified by senxiang, 5/Dec/2009

Notes:

--*/

#ifndef    _INIT_H
#define    _INIT_H

SORA_EXTERN_C
NDIS_STATUS
NICAllocAdapter(PMP_ADAPTER *pAdapter); //new a MP_ADAPTER object.

SORA_EXTERN_C
NDIS_STATUS
NICInitializeAdapter(
    IN PMP_ADAPTER Adapter,
    IN NDIS_HANDLE NdisHandle);//initialization

SORA_EXTERN_C
NDIS_STATUS
MPAllocatePacketPool(IN PMP_ADAPTER Adapter);

SORA_EXTERN_C
NDIS_STATUS 
NICReadRegistryConfiguration(
    IN PMP_ADAPTER Adapter,
    IN NDIS_HANDLE NdisHandle);

SORA_EXTERN_C
NDIS_STATUS 
NICInitializeSdrContext(IN PMP_ADAPTER Adapter);

SORA_EXTERN_C
VOID 
NICRequestRadiosThreadFunc(IN PVOID pContext);

SORA_EXTERN_C
VOID 
NICAdapterCtor(PMP_ADAPTER pAdapter); //constructor for MP_ADAPTER

SORA_EXTERN_C
VOID 
ReadTxParameters(
    IN PMP_ADAPTER pAdapter,
    IN NDIS_HANDLE ConfigurationHandle);

#ifdef NDIS620_MINIPORT

SORA_EXTERN_C
NDIS_STATUS
MpSetRegistrationAttributes(
    IN PMP_ADAPTER Adapter);

SORA_EXTERN_C
NDIS_STATUS
MpSetMiniportAttributes(
    IN PMP_ADAPTER Adapter);

SORA_EXTERN_C
NDIS_STATUS 
VNicAllocateNdisPort(
    IN NDIS_HANDLE MiniportAdapterHandle, 
    OUT PNDIS_PORT_NUMBER AllocatedPortNumber);

#endif

#define XMIT_SPEED (100 * 1000000)
#define RECV_SPEED (100 * 1000000)

#define NIC_RECV_POOL_SIZE   256

#define NET_BUFFER_DATA_SIZE 2048
#define CHECK_FOR_HANG_TIME_IN_SECONDS 6
#define DATA_BACK_FILL_SIZE 8
#define REGISTRY_ADDRESS_NUM_OFFSET 3

#define NIC_SET_PHY_NOT_READY(nic)\
{\
    (nic)->Phy.bRadiosInitOK = FALSE;\
}

#define NIC_SET_MAC_NOT_READY(nic)\
{\
    (nic)->Mac.bMacInitOK = FALSE;\
}

#define NIC_SET_PHY_READY(nic)\
{\
    (nic)->Phy.bRadiosInitOK = TRUE;\
}

#define NIC_SET_MAC_READY(nic)\
{\
    (nic)->Mac.bMacInitOK = TRUE;\
}

#define NIC_IS_PHY_READY(nic)\
    ((nic)->Phy.bRadiosInitOK)

#define NIC_IS_MAC_READY(nic)\
    ((nic)->Mac.bMacInitOK)

#define MP_NDIS_OBJ_INIT(_phdr, _type, _rev, _size) \
    do { (_phdr)->Revision = _rev; (_phdr)->Type = _type; (_phdr)->Size = _size; } while(FALSE);

#define NIC_REGISTER_DEVICE_INIT_PARAMETERS(DispatchTable, devName, symLinkName) \
    do \
    { \
        NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION + 1) * sizeof(PDRIVER_DISPATCH)); \
        RtlInitUnicodeString(&(devName), NIC_DEVICE_NAME); \
        RtlInitUnicodeString(&(symLinkName), NIC_LINK_NAME); \
        (DispatchTable)[IRP_MJ_CREATE]          = MPDeviceIoControl; \
        (DispatchTable)[IRP_MJ_CLEANUP]         = MPDeviceIoControl; \
        (DispatchTable)[IRP_MJ_CLOSE]           = MPDeviceIoControl; \
        (DispatchTable)[IRP_MJ_DEVICE_CONTROL]  = MPDeviceIoControl; \
    } while(FALSE);

#endif
