

#define HWT_VNIC_GEN_STAT_FLAGS(_pNic)                                        \
  ( NDIS_STATISTICS_FLAGS_VALID_DIRECTED_FRAMES_RCV       |                   \
    NDIS_STATISTICS_FLAGS_VALID_MULTICAST_FRAMES_RCV      |                   \
    NDIS_STATISTICS_FLAGS_VALID_BROADCAST_FRAMES_RCV      |                   \
    NDIS_STATISTICS_FLAGS_VALID_BYTES_RCV                 |                   \
    NDIS_STATISTICS_FLAGS_VALID_RCV_DISCARDS              |                   \
    NDIS_STATISTICS_FLAGS_VALID_RCV_ERROR                 |                   \
    NDIS_STATISTICS_FLAGS_VALID_DIRECTED_FRAMES_XMIT      |                   \
    NDIS_STATISTICS_FLAGS_VALID_MULTICAST_FRAMES_XMIT     |                   \
    NDIS_STATISTICS_FLAGS_VALID_BROADCAST_FRAMES_XMIT     |                   \
    NDIS_STATISTICS_FLAGS_VALID_BYTES_XMIT                |                   \
    NDIS_STATISTICS_FLAGS_VALID_XMIT_ERROR                |                   \
    NDIS_STATISTICS_FLAGS_VALID_XMIT_DISCARDS             |                   \
    NDIS_STATISTICS_FLAGS_VALID_DIRECTED_BYTES_RCV        |                   \
    NDIS_STATISTICS_FLAGS_VALID_MULTICAST_BYTES_RCV       |                   \
    NDIS_STATISTICS_FLAGS_VALID_BROADCAST_BYTES_RCV       |                   \
    NDIS_STATISTICS_FLAGS_VALID_DIRECTED_BYTES_XMIT       |                   \
    NDIS_STATISTICS_FLAGS_VALID_MULTICAST_BYTES_XMIT      |                   \
    NDIS_STATISTICS_FLAGS_VALID_BROADCAST_BYTES_XMIT )
