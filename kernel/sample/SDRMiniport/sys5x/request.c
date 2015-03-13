/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: 
   Request.C

Abstract: 
    NDIS 5.x Miniport driver for 802.11b Sora Sample. This is 
    MiniportQueryInformation routine for NDIS 5.x.

Revision History:
    Created by senxiang, 7/Apr/2009

Notes: 

--*/


#include "miniport.h"

NDIS_OID NICSupportedOids[] =
{
        OID_GEN_SUPPORTED_LIST,
        OID_GEN_HARDWARE_STATUS,
        OID_GEN_MEDIA_SUPPORTED,
        OID_GEN_MEDIA_IN_USE,
        OID_GEN_MAXIMUM_LOOKAHEAD,
        OID_GEN_MAXIMUM_FRAME_SIZE,
        OID_GEN_LINK_SPEED,
        OID_GEN_TRANSMIT_BUFFER_SPACE,
        OID_GEN_RECEIVE_BUFFER_SPACE,
        OID_GEN_TRANSMIT_BLOCK_SIZE,
        OID_GEN_RECEIVE_BLOCK_SIZE,
        OID_GEN_VENDOR_ID,
        OID_GEN_VENDOR_DESCRIPTION,
        OID_GEN_VENDOR_DRIVER_VERSION,
        OID_GEN_CURRENT_PACKET_FILTER,
        OID_GEN_CURRENT_LOOKAHEAD,
        OID_GEN_DRIVER_VERSION,
        OID_GEN_MAXIMUM_TOTAL_SIZE,
        OID_GEN_PROTOCOL_OPTIONS,
        OID_GEN_MAC_OPTIONS,
        OID_GEN_MEDIA_CONNECT_STATUS,
        OID_GEN_MAXIMUM_SEND_PACKETS,
        OID_GEN_XMIT_OK,
        OID_GEN_RCV_OK,
        OID_GEN_XMIT_ERROR,
        OID_GEN_RCV_ERROR,
        OID_GEN_RCV_NO_BUFFER,
        OID_GEN_RCV_CRC_ERROR,
        OID_GEN_TRANSMIT_QUEUE_LENGTH,
        OID_802_3_PERMANENT_ADDRESS,
        OID_802_3_CURRENT_ADDRESS,
        OID_802_3_MULTICAST_LIST,
        OID_802_3_MAC_OPTIONS,
        OID_802_3_MAXIMUM_LIST_SIZE,
        OID_802_3_RCV_ERROR_ALIGNMENT,
        OID_802_3_XMIT_ONE_COLLISION,
        OID_802_3_XMIT_MORE_COLLISIONS,
        OID_802_3_XMIT_DEFERRED,
        OID_802_3_XMIT_MAX_COLLISIONS,
        OID_802_3_RCV_OVERRUN,
        OID_802_3_XMIT_UNDERRUN,
        OID_802_3_XMIT_HEARTBEAT_FAILURE,
        OID_802_3_XMIT_TIMES_CRS_LOST,
        OID_802_3_XMIT_LATE_COLLISIONS,
        OID_PNP_CAPABILITIES,
        OID_PNP_SET_POWER,
        OID_PNP_QUERY_POWER,
        OID_PNP_ADD_WAKE_UP_PATTERN,
        OID_PNP_REMOVE_WAKE_UP_PATTERN,
        OID_PNP_ENABLE_WAKE_UP,
        OID_GEN_TRANSMIT_QUEUE_LENGTH
};

PCHAR DbgGetOidName(ULONG oid)
{
    PCHAR oidName;

    switch (oid){

        #undef MAKECASE
        #define MAKECASE(oidx) case oidx: oidName = #oidx; break;

        MAKECASE(OID_GEN_SUPPORTED_LIST)
        MAKECASE(OID_GEN_HARDWARE_STATUS)
        MAKECASE(OID_GEN_MEDIA_SUPPORTED)
        MAKECASE(OID_GEN_MEDIA_IN_USE)
        MAKECASE(OID_GEN_MAXIMUM_LOOKAHEAD)
        MAKECASE(OID_GEN_MAXIMUM_FRAME_SIZE)
        MAKECASE(OID_GEN_LINK_SPEED)
        MAKECASE(OID_GEN_TRANSMIT_BUFFER_SPACE)
        MAKECASE(OID_GEN_RECEIVE_BUFFER_SPACE)
        MAKECASE(OID_GEN_TRANSMIT_BLOCK_SIZE)
        MAKECASE(OID_GEN_RECEIVE_BLOCK_SIZE)
        MAKECASE(OID_GEN_VENDOR_ID)
        MAKECASE(OID_GEN_VENDOR_DESCRIPTION)
        MAKECASE(OID_GEN_CURRENT_PACKET_FILTER)
        MAKECASE(OID_GEN_CURRENT_LOOKAHEAD)
        MAKECASE(OID_GEN_DRIVER_VERSION)
        MAKECASE(OID_GEN_MAXIMUM_TOTAL_SIZE)
        MAKECASE(OID_GEN_PROTOCOL_OPTIONS)
        MAKECASE(OID_GEN_MAC_OPTIONS)
        MAKECASE(OID_GEN_MEDIA_CONNECT_STATUS)
        MAKECASE(OID_GEN_MAXIMUM_SEND_PACKETS)
        MAKECASE(OID_GEN_VENDOR_DRIVER_VERSION)
        MAKECASE(OID_GEN_SUPPORTED_GUIDS)
        MAKECASE(OID_GEN_NETWORK_LAYER_ADDRESSES)
        MAKECASE(OID_GEN_TRANSPORT_HEADER_OFFSET)
        MAKECASE(OID_GEN_MEDIA_CAPABILITIES)
        MAKECASE(OID_GEN_PHYSICAL_MEDIUM)
        MAKECASE(OID_GEN_XMIT_OK)
        MAKECASE(OID_GEN_RCV_OK)
        MAKECASE(OID_GEN_XMIT_ERROR)
        MAKECASE(OID_GEN_RCV_ERROR)
        MAKECASE(OID_GEN_RCV_NO_BUFFER)
        MAKECASE(OID_GEN_DIRECTED_BYTES_XMIT)
        MAKECASE(OID_GEN_DIRECTED_FRAMES_XMIT)
        MAKECASE(OID_GEN_MULTICAST_BYTES_XMIT)
        MAKECASE(OID_GEN_MULTICAST_FRAMES_XMIT)
        MAKECASE(OID_GEN_BROADCAST_BYTES_XMIT)
        MAKECASE(OID_GEN_BROADCAST_FRAMES_XMIT)
        MAKECASE(OID_GEN_DIRECTED_BYTES_RCV)
        MAKECASE(OID_GEN_DIRECTED_FRAMES_RCV)
        MAKECASE(OID_GEN_MULTICAST_BYTES_RCV)
        MAKECASE(OID_GEN_MULTICAST_FRAMES_RCV)
        MAKECASE(OID_GEN_BROADCAST_BYTES_RCV)
        MAKECASE(OID_GEN_BROADCAST_FRAMES_RCV)
        MAKECASE(OID_GEN_RCV_CRC_ERROR)
        MAKECASE(OID_GEN_TRANSMIT_QUEUE_LENGTH)
        MAKECASE(OID_GEN_GET_TIME_CAPS)
        MAKECASE(OID_GEN_GET_NETCARD_TIME)
        MAKECASE(OID_GEN_NETCARD_LOAD)
        MAKECASE(OID_GEN_DEVICE_PROFILE)
        MAKECASE(OID_GEN_INIT_TIME_MS)
        MAKECASE(OID_GEN_RESET_COUNTS)
        MAKECASE(OID_GEN_MEDIA_SENSE_COUNTS)
        MAKECASE(OID_PNP_CAPABILITIES)
        MAKECASE(OID_PNP_SET_POWER)
        MAKECASE(OID_PNP_QUERY_POWER)
        MAKECASE(OID_PNP_ADD_WAKE_UP_PATTERN)
        MAKECASE(OID_PNP_REMOVE_WAKE_UP_PATTERN)
        MAKECASE(OID_PNP_ENABLE_WAKE_UP)
        MAKECASE(OID_802_3_PERMANENT_ADDRESS)
        MAKECASE(OID_802_3_CURRENT_ADDRESS)
        MAKECASE(OID_802_3_MULTICAST_LIST)
        MAKECASE(OID_802_3_MAXIMUM_LIST_SIZE)
        MAKECASE(OID_802_3_MAC_OPTIONS)
        MAKECASE(OID_802_3_RCV_ERROR_ALIGNMENT)
        MAKECASE(OID_802_3_XMIT_ONE_COLLISION)
        MAKECASE(OID_802_3_XMIT_MORE_COLLISIONS)
        MAKECASE(OID_802_3_XMIT_DEFERRED)
        MAKECASE(OID_802_3_XMIT_MAX_COLLISIONS)
        MAKECASE(OID_802_3_RCV_OVERRUN)
        MAKECASE(OID_802_3_XMIT_UNDERRUN)
        MAKECASE(OID_802_3_XMIT_HEARTBEAT_FAILURE)
        MAKECASE(OID_802_3_XMIT_TIMES_CRS_LOST)
        MAKECASE(OID_802_3_XMIT_LATE_COLLISIONS)

        default: 
            oidName = "<** UNKNOWN OID **>";
            break;
    }

    return oidName;
}


/*++

Routine Description:

    Entry point called by NDIS to query for the value of the specified OID.

Arguments:

    MiniportAdapterContext      Pointer to the adapter structure
    Oid                         Oid for this query
    InformationBuffer           Buffer for information
    InformationBufferLength     Size of this buffer
    BytesWritten                Specifies how much info is written
    BytesNeeded                 In case the buffer is smaller than 
                                what we need, tell them how much is needed


Return Value:

    Return code from the NdisRequest below.
    
Notes:

IRQL: PASSIVE_LEVEL
--*/
NDIS_STATUS MPQueryInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID     Oid,
    IN PVOID        InformationBuffer,
    IN ULONG        InformationBufferLength,
    OUT PULONG      UCHARsWritten,
    OUT PULONG      UCHARsNeeded)
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER             Adapter;
    NDIS_HARDWARE_STATUS    HardwareStatus = NdisHardwareStatusReady;
    NDIS_MEDIUM             Medium = NIC_MEDIA_TYPE;
    UCHAR                   VendorDesc[] = NIC_VENDOR_DESC;
    ULONG                   ulInfo;
    USHORT                  usInfo;                                                              
    ULONG64                 ulInfo64;
    PVOID                   pInfo = (PVOID) &ulInfo;
    ULONG                   ulInfoLen = sizeof(ulInfo);   
    
    DEBUGP(MP_LOUD, ("---> MPQueryInformation %s\n", DbgGetOidName(Oid)));
    Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    // Initialize the result
    *UCHARsWritten = 0;
    *UCHARsNeeded = 0;

    switch(Oid)
    {
        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            ulInfo = TCB_MAX_COUNT - Adapter->Mac.SendQueueManager.nFreeTCB;
        break;

        case OID_GEN_SUPPORTED_LIST:
            pInfo = (PVOID) NICSupportedOids;
            ulInfoLen = sizeof(NICSupportedOids);
            break;
     
        case OID_GEN_HARDWARE_STATUS:
            if(Adapter->Phy.bRadiosInitOK)
                HardwareStatus = NdisHardwareStatusReady;
            else
                HardwareStatus = NdisHardwareStatusNotReady;

            pInfo = (PVOID) &HardwareStatus;
            ulInfoLen = sizeof(NDIS_HARDWARE_STATUS);
            break;
        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
            //
            // Specifiy a complete list of the media types that the NIC
            // currently uses. 
            //
            pInfo = (PVOID) &Medium;
            ulInfoLen = sizeof(NDIS_MEDIUM);
            break;
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_MAXIMUM_LOOKAHEAD:

               ulInfo = NIC_MAX_LOOKAHEAD;
            break;
        case OID_GEN_MAXIMUM_FRAME_SIZE:
            ulInfo = NIC_ETH_MAX_DATA_SIZE;
            break;
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:

            ulInfo = NIC_MAX_PACKET_SIZE;
            break;
    
        case OID_GEN_MAC_OPTIONS:
            ulInfo =    NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
                        NDIS_MAC_OPTION_NO_LOOPBACK;
            break;
        case OID_GEN_LINK_SPEED:
            ulInfo = NIC_LINK_SPEED;
            break;
        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            //
            // Specify the amount of memory, in UCHARs, on the NIC that 
            // is available for buffering transmit data. A protocol can 
            // use this OID as a guide for sizing the amount of transmit 
            // data per send.            
            //
            ulInfo = NIC_MAX_PACKET_SIZE * TCB_MAX_COUNT;
            break;
        case OID_GEN_RECEIVE_BUFFER_SPACE:
            ulInfo = NIC_RECEIVE_BUFFER_SPACE;
            break;
        case OID_GEN_VENDOR_ID:
            ulInfo = NIC_VENDOR_ID;
            break;
        case OID_GEN_VENDOR_DESCRIPTION:
            pInfo = VendorDesc;
            ulInfoLen = sizeof(VendorDesc);
            break;
        case OID_GEN_VENDOR_DRIVER_VERSION:
            //
            // Specify the vendor-assigned version number of the NIC driver. 
            // The low-order half of the return value specifies the minor 
            // version; the high-order half specifies the major version.
            //
            ulInfo = NIC_VENDOR_DRIVER_VERSION;
            break;
        case OID_GEN_DRIVER_VERSION:
            //
            // Specify the NDIS version in use by the NIC driver. The high 
            // UCHAR is the major version number; the low UCHAR is the minor 
            // version number.
            //
            usInfo = (USHORT) (MP_NDIS_MAJOR_VERSION<<8) + MP_NDIS_MINOR_VERSION;
            pInfo = (PVOID) &usInfo;
            ulInfoLen = sizeof(USHORT);
            break;    
        case OID_GEN_MAXIMUM_SEND_PACKETS:
            ulInfo = NIC_MAXIMUM_SEND_PACKETS;
            break;
    
        case OID_GEN_MEDIA_CONNECT_STATUS:
            ulInfo = NdisMediaStateConnected;
            break;
        case OID_GEN_CURRENT_PACKET_FILTER:
            ulInfo = NIC_CURRENT_PACKET_FILTER;
            break;
        case OID_PNP_CAPABILITIES:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
            //
            // Return the MAC address of the NIC burnt in the hardware.
            //
            pInfo = Adapter->CurrentAddress;
            ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;
    
        case OID_802_3_MAXIMUM_LIST_SIZE:
            ulInfo = NIC_MAXIMUN_LIST_SZIE;
            break;
        case OID_802_3_MAC_OPTIONS:
            ulInfo = NIC_802_3_MAC_OPTION;
            break;
        

        case OID_GEN_XMIT_OK:
             ulInfo64 = Adapter->ullGoodTransmits;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);
            }
            else
            {
                ulInfoLen = sizeof(ULONG);
            }
            break;
        
        case OID_GEN_RCV_OK:
            ulInfo64 = Adapter->ullGoodReceives;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);
               
            }
            else
            {
                ulInfoLen = sizeof(ULONG);
                
            }
                                    
            break;
            
        case OID_GEN_XMIT_ERROR:
            ulInfo64 = Adapter->ullTransmitFail;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);
               
            }
            else
            {
                ulInfoLen = sizeof(ULONG);
                
            }
            break;
        case OID_GEN_RCV_ERROR:
            ulInfo64 = Adapter->ullReceiveErrors;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);

            }
            else
            {
                ulInfoLen = sizeof(ULONG);

            }
            break;
        case OID_GEN_RCV_NO_BUFFER:
            ulInfo64 = Adapter->ullReceiveNoBuffers;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);

            }
            else
            {
                ulInfoLen = sizeof(ULONG);

            }
            break;
        case OID_GEN_RCV_CRC_ERROR:
            ulInfo64 = RxCRC32ErrorNumber(&Adapter->Phy);
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);

            }
            else
            {
                ulInfoLen = sizeof(ULONG);

            }
            break;
        case OID_802_3_RCV_ERROR_ALIGNMENT:
            ulInfo = NIC_RCV_ERROR_ALIGNMENT;
            break;
        case OID_802_3_XMIT_ONE_COLLISION:
            ulInfo = NIC_802_3_XMIT_ONE_COLLISION;
            break;
        case OID_802_3_XMIT_MORE_COLLISIONS:
            ulInfo = NIC_802_3_XMIT_MORE_COLLISIONS;
            break;
    
        case OID_802_3_XMIT_DEFERRED:
            ulInfo = NIC_802_3_XMIT_DEFERRED;
            break;
    
        case OID_802_3_XMIT_MAX_COLLISIONS:
            ulInfo = NIC_802_3_XMIT_MAX_COLLISIONS;
            break;
        case OID_802_3_RCV_OVERRUN:
            ulInfo = NIC_802_3_RCV_OVERRUN;
            break;
        case OID_802_3_XMIT_UNDERRUN:
            ulInfo = NIC_802_3_XMIT_UNDERRUN;
            break;
    
        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
            ulInfo = NIC_802_3_XMIT_HEARTBEAT_FAILURE;
            break;
    
        case OID_802_3_XMIT_TIMES_CRS_LOST:
            ulInfo = NIC_802_3_XMIT_TIMES_CRS_LOST;
            break;
    
        case OID_802_3_XMIT_LATE_COLLISIONS:
            ulInfo = NIC_802_3_XMIT_LATE_COLLISIONS;
            break;
        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;            
    }
    if(Status == NDIS_STATUS_SUCCESS)
    {
        if(ulInfoLen <= InformationBufferLength)
        {
            // Copy result into InformationBuffer
            *UCHARsWritten = ulInfoLen;
            if(ulInfoLen)
            {
                NdisMoveMemory(InformationBuffer, pInfo, ulInfoLen);
            }
        }
        else
        {
            // too short
            *UCHARsNeeded = ulInfoLen;
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
        }
    }

    DEBUGP(MP_LOUD, ("<--- MPQueryInformation Status = 0x%08x\n", Status));
    
    return(Status);
}


/*++

Routine Description:

    This is the handler for an OID set operation. 

Arguments:

    MiniportAdapterContext      Pointer to the adapter structure
    Oid                         Oid for this query
    InformationBuffer           Buffer for information
    InformationBufferLength     Size of this buffer
    BytesRead                   Specifies how much info is read
    BytesNeeded                 In case the buffer is smaller than what 
                                we need, tell them how much is needed

Return Value:

IRQL: PASSIVE_LEVEL
--*/

NDIS_STATUS MPSetInformation(
    IN NDIS_HANDLE                                  MiniportAdapterContext,
    IN NDIS_OID                                     Oid,
    __in_bcount(InformationBufferLength) IN PVOID   InformationBuffer,
    IN ULONG                                        InformationBufferLength,
    OUT PULONG                                      UCHARsRead,
    OUT PULONG                                      UCHARsNeeded)
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    DEBUGP(MP_LOUD, ("---> MPSetInformation %s\n", DbgGetOidName(Oid)));
    DEBUGP(MP_LOUD, ("<-- MPSetInformation Status = 0x%08x\n", Status));
    return(Status);

}
