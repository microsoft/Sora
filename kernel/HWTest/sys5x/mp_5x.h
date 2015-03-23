#ifndef _MP_5X_H
#define _MP_5X_H

#include "..\hwtest_miniport.h"

#if defined(NDIS50_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION       5
    #define MP_NDIS_MINOR_VERSION       0
#elif defined(NDIS51_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION       5
    #define MP_NDIS_MINOR_VERSION       1
#endif

NDIS_STATUS 
DriverEntry(
            PVOID DriverObject,
            PVOID RegistryPath);

NDIS_STATUS 
MPInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext);

VOID 
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS
MPSetInformation(
    IN NDIS_HANDLE                                  MiniportAdapterContext,
    IN NDIS_OID                                     Oid,
    __in_bcount(InformationBufferLength) IN PVOID   InformationBuffer,
    IN ULONG                                        InformationBufferLength,
    OUT PULONG                                      UCHARsRead,
    OUT PULONG                                      UCHARsNeeded);

NDIS_STATUS 
MPQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG UCHARsWritten,
    OUT PULONG UCHARsNeeded);

VOID 
MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET Packet);

VOID 
MPSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets);

VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

NDIS_STATUS NICAllocAdapter(PHWT_ADAPTER *pAdapter);

#ifdef NDIS51_MINIPORT

VOID 
MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId
    );

VOID MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    );

VOID 
MPShutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    );

#endif

#endif