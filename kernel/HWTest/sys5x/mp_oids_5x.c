#include "mp_5x.h"

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
        OID_PNP_ENABLE_WAKE_UP
};

PUCHAR DbgGetOidName(ULONG oid)
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


NDIS_STATUS MPQueryInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID     Oid,
    IN PVOID        InformationBuffer,
    IN ULONG        InformationBufferLength,
    OUT PULONG      UCHARsWritten,
    OUT PULONG      UCHARsNeeded)
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PHWT_ADAPTER             Adapter;
    NDIS_HARDWARE_STATUS    HardwareStatus = NdisHardwareStatusReady;
    NDIS_MEDIUM             Medium = NIC_MEDIA_TYPE;
    UCHAR                   VendorDesc[] = NIC_VENDOR_DESC;
    ULONG                   ulInfo;
    USHORT                  usInfo;                                                              
    ULONG64                 ulInfo64;
    PVOID                   pInfo = (PVOID) &ulInfo;
    ULONG                   ulInfoLen = sizeof(ulInfo);   
    
    DEBUGP(MP_LOUD, ("---> MPQueryInformation %s\n", DbgGetOidName(Oid)));
    Adapter = (PHWT_ADAPTER) MiniportAdapterContext;

    // Initialize the result
    *UCHARsWritten = 0;
    *UCHARsNeeded = 0;

    switch(Oid)
    {
        case OID_PNP_CAPABILITIES: {
				if (InformationBufferLength >= sizeof(NDIS_PNP_CAPABILITIES)) {
					NDIS_PNP_CAPABILITIES* pnp_cap;
					pnp_cap = (NDIS_PNP_CAPABILITIES*)InformationBuffer;
					pnp_cap->Flags = 0;
					pnp_cap->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
					*UCHARsWritten = sizeof(NDIS_PNP_CAPABILITIES);
					Status = NDIS_STATUS_SUCCESS;
					return Status;
				}
				else {
					*UCHARsNeeded = sizeof(NDIS_PNP_CAPABILITIES);
					Status = NDIS_STATUS_BUFFER_TOO_SHORT;				
					return Status;
				}
        	}
            break;    
		case OID_PNP_QUERY_POWER: {
				if (InformationBufferLength >= sizeof(NDIS_DEVICE_POWER_STATE)) {
					*(NDIS_DEVICE_POWER_STATE*)InformationBuffer = NdisDeviceStateD1;
					*UCHARsWritten = sizeof(NDIS_DEVICE_POWER_STATE);
					Status = NDIS_STATUS_SUCCESS;
					return Status;
				}
				else {
					*UCHARsNeeded = sizeof(NDIS_DEVICE_POWER_STATE);
					Status = NDIS_STATUS_BUFFER_TOO_SHORT;				
					return Status;
				}
			}
			break;
        case OID_GEN_SUPPORTED_LIST:
            pInfo = (PVOID) NICSupportedOids;
            ulInfoLen = sizeof(NICSupportedOids);
            break;
    
        case OID_GEN_HARDWARE_STATUS:
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
            ulInfo = 1500;
            break;
        case OID_GEN_MAXIMUM_FRAME_SIZE:
            ulInfo = 1500;
            break;
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:
            ulInfo = 1514;
            break;
    
        case OID_GEN_MAC_OPTIONS:
            ulInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
            NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
            NDIS_MAC_OPTION_NO_LOOPBACK;
            break;
        case OID_GEN_LINK_SPEED:
            ulInfo = 100000;
            break;
        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            //
            // Specify the amount of memory, in UCHARs, on the NIC that 
            // is available for buffering transmit data. A protocol can 
            // use this OID as a guide for sizing the amount of transmit 
            // data per send.            
            //
            ulInfo = 1000000;
            break;
        case OID_GEN_RECEIVE_BUFFER_SPACE:
            ulInfo = 1000000;
            break;
        case OID_GEN_VENDOR_ID:
            ulInfo = 111;
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
            ulInfo = 111;
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
            ulInfo = 5;
            break;
    
        case OID_GEN_MEDIA_CONNECT_STATUS:
            ulInfo = NdisMediaStateConnected;
            break;
        case OID_GEN_CURRENT_PACKET_FILTER:
            ulInfo = 1;
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
            ulInfo = 32;
            break;
        case OID_802_3_MAC_OPTIONS:
            ulInfo = 0;
            break;
            
        case OID_GEN_XMIT_OK:
            ulInfo64 = 1;
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
            ulInfo64 = 1;
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
            *UCHARsNeeded =  sizeof(ULONG64);                        
            break;
            
        case OID_GEN_XMIT_ERROR:
            ulInfo = 0;
            break;
        case OID_GEN_RCV_ERROR:
            ulInfo = 0;
            break;
        case OID_GEN_RCV_NO_BUFFER:
            ulInfo = 0;
            break;
        case OID_GEN_RCV_CRC_ERROR:
            ulInfo = 0;
            break;
        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            ulInfo = 0;
            break;
        case OID_802_3_RCV_ERROR_ALIGNMENT:
            ulInfo = 0;
            break;
        case OID_802_3_XMIT_ONE_COLLISION:
            ulInfo = 0;
            break;
        case OID_802_3_XMIT_MORE_COLLISIONS:
            ulInfo = 0;
            break;
    
        case OID_802_3_XMIT_DEFERRED:
            ulInfo = 0;
            break;
    
        case OID_802_3_XMIT_MAX_COLLISIONS:
            ulInfo = 0;
            break;
        case OID_802_3_RCV_OVERRUN:
            ulInfo = 0;
            break;
        case OID_802_3_XMIT_UNDERRUN:
            ulInfo = 0;
            break;
    
        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
            ulInfo = 0;
            break;
    
        case OID_802_3_XMIT_TIMES_CRS_LOST:
            ulInfo = 0;
            break;
    
        case OID_802_3_XMIT_LATE_COLLISIONS:
            ulInfo = 0;
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
	switch(Oid) {
		case OID_PNP_SET_POWER:
			if (InformationBufferLength >= sizeof(NDIS_DEVICE_POWER_STATE)) {
				*UCHARsRead = sizeof(NDIS_DEVICE_POWER_STATE);
				Status = NDIS_STATUS_SUCCESS;
				return Status;
			}
			else {
				*UCHARsNeeded = sizeof(NDIS_DEVICE_POWER_STATE);
				Status = NDIS_STATUS_BUFFER_TOO_SHORT;	
				return Status;				
			}
			break;
		default:
			break;
	};
    return(Status);

}
