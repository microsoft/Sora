#ifndef _MP_6X_H
#define _MP_6X_H

#include "..\hwtest_miniport.h"

#define MP_MAJOR_NDIS_VERSION       6
#define MP_MINOR_NDIS_VERSION       20

#define HWT_MAJOR_DRIVER_VERSION    2
#define HWT_MINOR_DRIVER_VERSION    0


#define MP_NDIS_VERSION_NEEDS_COMPATIBILITY   0x00060001

#define HWT_NDIS_OBJ_INIT(_phdr, _type, _rev, _size) \
    do { (_phdr)->Revision = _rev; (_phdr)->Type = _type; (_phdr)->Size = _size; } while(FALSE);

#define NET_BUFFER_LIST_BUFFER_COUNT(nbl)		\
		((ULONG)NET_BUFFER_LIST_MINIPORT_RESERVED(nbl)[1])

#define NET_BUFFER_BUFFER_LIST(nb)				\
		((PNET_BUFFER_LIST)NET_BUFFER_MINIPORT_RESERVED(nb)[1])

#define XMIT_SPEED                  (100 * 1000000)
#define RECV_SPEED                  (100 * 1000000)

#define HWT_VNIC_PKT_FILTERS \
  ( NDIS_PACKET_TYPE_DIRECTED      |                                          \
    NDIS_PACKET_TYPE_MULTICAST     |                                          \
    NDIS_PACKET_TYPE_ALL_MULTICAST |                                          \
    NDIS_PACKET_TYPE_BROADCAST     |                                          \
    NDIS_PACKET_TYPE_PROMISCUOUS   )


MINIPORT_INITIALIZE MPInitializeEx;
MINIPORT_HALT MPHalt;

MINIPORT_UNLOAD DriverUnload;

MINIPORT_PAUSE MPPause;
MINIPORT_RESTART MPRestart;

MINIPORT_SHUTDOWN MPShutdownEx;
MINIPORT_DEVICE_PNP_EVENT_NOTIFY MPDevicePnPEvent;

MINIPORT_OID_REQUEST MPOidRequest;
MINIPORT_CANCEL_OID_REQUEST MPCancelOidRequest;

MINIPORT_SET_OPTIONS MPSetOptions;

MINIPORT_SEND_NET_BUFFER_LISTS MPSendNetBufferLists;

MINIPORT_CANCEL_OID_REQUEST MPCancelSendNetBufferLists;

MINIPORT_RETURN_NET_BUFFER_LISTS MPReturnNetBufferLists;

MINIPORT_CHECK_FOR_HANG MPCheckForHangEx;
MINIPORT_RESET MPResetEx;

MINIPORT_DIRECT_OID_REQUEST MPDirectOidRequest;
MINIPORT_CANCEL_DIRECT_OID_REQUEST MPCancelDirectOidRequest;

VOID
MpQuerySupportedOidsList(
    __inout PNDIS_OID            *SupportedOidList,
    __inout PULONG               SupportedOidListLength
    );

#endif