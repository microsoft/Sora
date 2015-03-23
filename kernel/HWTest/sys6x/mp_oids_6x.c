#include "mp_6x.h"
#include "mp_oids_6x.h"

NDIS_OID MPSupportedOids[] =
{
    // NDIS OIDs
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_INTERRUPT_MODERATION,
    OID_GEN_LINK_PARAMETERS,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_SUPPORTED_GUIDS,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_TRANSMIT_QUEUE_LENGTH,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_VENDOR_ID,
    OID_802_3_CURRENT_ADDRESS,          // This is needed for compatibility with some apps

    /* Non-802.11 specific statistics are handled by the nwifi filter, but we
     * report support for them
    */
    OID_GEN_RCV_OK,
    OID_GEN_STATISTICS,
    OID_GEN_XMIT_OK,

    // PNP handlers
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,

    /* Driver does not support wake up from patterns
        OID_PNP_ADD_WAKE_UP_PATTERN,
        OID_PNP_REMOVE_WAKE_UP_PATTERN,
        OID_PNP_ENABLE_WAKE_UP,
    */
};

VOID
MpQuerySupportedOidsList(
    __inout PNDIS_OID            *SupportedOidList,
    __inout PULONG               SupportedOidListLength
    )
{
    *SupportedOidList = MPSupportedOids;
    *SupportedOidListLength = sizeof(MPSupportedOids);
}


VOID 
VNicQueryStatistics(
    __in NDIS_STATISTICS_INFO *pStats)
{
    HWT_NDIS_OBJ_INIT(
        &pStats->Header, 
        NDIS_OBJECT_TYPE_DEFAULT, 
        NDIS_OBJECT_REVISION_1, 
        sizeof(NDIS_STATISTICS_INFO));
    
    pStats->SupportedStatistics = HWT_VNIC_GEN_STAT_FLAGS(NULL);

    pStats->ifInDiscards            = 0;
    pStats->ifInErrors              = 0;
    pStats->ifHCInOctets            = 0;
    pStats->ifHCInUcastPkts         = 0;
    pStats->ifHCInMulticastPkts     = 0;
    pStats->ifHCInBroadcastPkts     = 0;
    pStats->ifHCOutOctets           = 0;
    pStats->ifHCOutUcastPkts        = 0;
    pStats->ifHCOutMulticastPkts    = 0;
    pStats->ifHCOutBroadcastPkts    = 0;
    pStats->ifOutErrors             = 0;
    pStats->ifOutDiscards           = 0;
    pStats->ifHCInUcastOctets       = 0;
    pStats->ifHCInMulticastOctets   = 0;
    pStats->ifHCInBroadcastOctets   = 0;
    pStats->ifHCOutUcastOctets      = 0;
    pStats->ifHCOutMulticastOctets  = 0;
    pStats->ifHCOutBroadcastOctets  = 0;

}


NDIS_STATUS
MPOidRequest(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  PNDIS_OID_REQUEST       NdisOidRequest
    )
{
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
    PHWT_ADAPTER                adapter = (PHWT_ADAPTER)MiniportAdapterContext;
    NDIS_OID                    oid;
    ULONG                       InformationBufferLength;
    PVOID                       InformationBuffer;
    ULONG                       BytesWritten = 0;
    ULONG                       BytesNeeded = 0;
    ULONG                       BytesRead = 0;
    ULONG                       InfoVal = 0;
    ULONG                       InfoLen = sizeof(InfoVal);
    PVOID                       pInfo = (PVOID) &InfoVal;
    NDIS_REQUEST_TYPE           Type;
    ULONG                       PacketFilter;
    NDIS_INTERRUPT_MODERATION_PARAMETERS IntrModr = {0};
    NDIS_MEDIUM                 Medium = NdisMedium802_3;

    oid = NdisOidRequest->DATA.QUERY_INFORMATION.Oid;
    Type = NdisOidRequest->RequestType;
    InformationBuffer = NdisOidRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    InformationBufferLength = NdisOidRequest->DATA.QUERY_INFORMATION.InformationBufferLength;

    switch (oid)
    {
    case OID_PNP_QUERY_POWER: {
			if (InformationBufferLength >= sizeof(NDIS_DEVICE_POWER_STATE)) {
				ndisStatus = NDIS_STATUS_SUCCESS;
				*(NDIS_DEVICE_POWER_STATE*)InformationBuffer = NdisDeviceStateD1;
				BytesWritten = sizeof(NDIS_DEVICE_POWER_STATE);
				InfoLen = 0;
			}
			else {
				ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
				BytesNeeded = sizeof(NDIS_DEVICE_POWER_STATE);
			}
	    }
		break;
    case OID_PNP_SET_POWER: {
			if (InformationBufferLength >= sizeof(NDIS_DEVICE_POWER_STATE)) {
				ndisStatus = NDIS_STATUS_SUCCESS;
				BytesRead = sizeof(NDIS_DEVICE_POWER_STATE);
				InfoLen = 0;
			}
			else {
				ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
				BytesNeeded = sizeof(NDIS_DEVICE_POWER_STATE);
			}
    	}
		break;
    case OID_GEN_MEDIA_SUPPORTED:
    case OID_GEN_MEDIA_IN_USE:
            ndisStatus = NDIS_STATUS_SUCCESS;   
            pInfo = (PVOID) &Medium;
            InfoLen = sizeof(NDIS_MEDIUM);
            break;
    case OID_GEN_INTERRUPT_MODERATION:
        if (Type == NdisRequestQueryInformation)
        {
            ndisStatus = NDIS_STATUS_SUCCESS;   
            HWT_NDIS_OBJ_INIT(
                &IntrModr.Header, 
                NDIS_OBJECT_TYPE_DEFAULT,
                NDIS_INTERRUPT_MODERATION_PARAMETERS_REVISION_1,
                sizeof(NDIS_INTERRUPT_MODERATION_PARAMETERS));
            IntrModr.Flags = NDIS_INTERRUPT_MODERATION_CHANGE_NEEDS_RESET;
            IntrModr.InterruptModeration = NdisInterruptModerationNotSupported; 
            pInfo = (PVOID) &IntrModr;
            InfoLen = sizeof(NDIS_INTERRUPT_MODERATION_PARAMETERS);
        }
        
        break;
    case OID_GEN_CURRENT_PACKET_FILTER:
        ndisStatus = NDIS_STATUS_SUCCESS;   
        BytesRead = InformationBufferLength;
        PacketFilter = *((PULONG) InformationBuffer);
        break;
    case OID_GEN_RCV_OK:
    case OID_GEN_XMIT_OK:
        ndisStatus = NDIS_STATUS_SUCCESS;   
        InfoVal = 0;
        InfoLen = 4;
        break;
    case OID_GEN_MAXIMUM_TOTAL_SIZE:
        InfoVal = 1500;
        ndisStatus = NDIS_STATUS_SUCCESS;   
        break;
    case OID_802_3_CURRENT_ADDRESS:
        if ( InformationBufferLength < 6 )
        {
            BytesNeeded = 6;
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
            break;
        }
        ndisStatus = NDIS_STATUS_SUCCESS;   
        BytesWritten = 6;
        InfoLen = 0;
        ((PUCHAR)InformationBuffer)[0] = adapter->CurrentAddress[0];
        ((PUCHAR)InformationBuffer)[1] = adapter->CurrentAddress[1];
        ((PUCHAR)InformationBuffer)[2] = adapter->CurrentAddress[2];
        ((PUCHAR)InformationBuffer)[3] = adapter->CurrentAddress[3];
        ((PUCHAR)InformationBuffer)[4] = adapter->CurrentAddress[4];
        ((PUCHAR)InformationBuffer)[5] = adapter->CurrentAddress[5];
        break;
    case OID_GEN_STATISTICS:
        if ( InformationBufferLength < sizeof(NDIS_STATISTICS_INFO) )
        {
            BytesNeeded = sizeof(NDIS_STATISTICS_INFO);
            ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
            break;
        }
        ndisStatus = NDIS_STATUS_SUCCESS;   
        BytesWritten = sizeof(NDIS_STATISTICS_INFO);
        InfoLen = 0;
        VNicQueryStatistics((NDIS_STATISTICS_INFO*)InformationBuffer);
        break;
    default:
        DbgPrint("[HWTest]unknown oids\n");
        break;
    }

    if ( ndisStatus == NDIS_STATUS_SUCCESS )
    {
        /*    BytesNeeded = BytesAvailable;*/
        if ( InfoLen <= InformationBufferLength )
        {
          if ( InfoLen )
          {
            BytesWritten = InfoLen;
            NdisMoveMemory(InformationBuffer, pInfo, InfoLen);
          }
        }
        else
        {
          BytesNeeded = InfoLen;
          ndisStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        }
    }
    
    if (Type == NdisRequestSetInformation)
    {
        NdisOidRequest->DATA.SET_INFORMATION.BytesRead = BytesRead;
        NdisOidRequest->DATA.SET_INFORMATION.BytesNeeded = BytesNeeded;
    }
    else
    {
        NdisOidRequest->DATA.QUERY_INFORMATION.BytesWritten = BytesWritten;
        NdisOidRequest->DATA.QUERY_INFORMATION.BytesNeeded = BytesNeeded;
    }
    return ndisStatus;
}

VOID 
MPCancelOidRequest(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  PVOID                   RequestId
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(RequestId);

}

NDIS_STATUS
MPDirectOidRequest(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  PNDIS_OID_REQUEST       NdisOidRequest
    )
{
    NDIS_OID                    oid;
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_NOT_SUPPORTED;

    oid = NdisOidRequest->DATA.QUERY_INFORMATION.Oid; // Oid is at same offset for all RequestTypes
    switch (oid)
    {
    default:
        DbgPrint("Unsupported oid\n");
        break;
    }
    return ndisStatus;
}

VOID 
MPCancelDirectOidRequest(
    __in  NDIS_HANDLE             MiniportAdapterContext,
    __in  PVOID                   RequestId
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(RequestId);
}
