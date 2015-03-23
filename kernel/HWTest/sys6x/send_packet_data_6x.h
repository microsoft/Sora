#pragma once

#include "send_packet_data.h"

struct Send_Packet_Data {

	PNET_BUFFER m_net_buffer;
	UINT m_len; 
	PMDL m_mdl;
	PVOID m_addr;
	unsigned char m_locked;
	unsigned char* m_data;
	HANDLE m_map_process;
};

#define free_send_packet_data(packet_data) {								\
	if (packet_data) {														\
		if (packet_data->m_mdl) { 											\
			if (packet_data->m_addr)										\
				if (packet_data->m_map_process == PsGetCurrentProcessId())	\
					MmUnmapLockedPages(packet_data->m_addr,					\
						packet_data->m_mdl);								\
			if (packet_data->m_locked)										\
				MmUnlockPages(packet_data->m_mdl);							\
			IoFreeMdl(packet_data->m_mdl);									\
		}																	\
		ExFreePoolWithTag(packet_data, 'tkip');								\
		packet_data = NULL;													\
	}																		\
}																			

void free_get_send_packet(
	PVOID context, 
	struct RESOURCE_OBJECT_DATA* rdata) {

	struct Send_Packet_Data* packet_data;
	PHWT_ADAPTER Adapter;
	PNET_BUFFER_LIST NetBufferList;
	packet_data = (struct Send_Packet_Data*)rdata->m_associated.m_get_send_packet_out.Packet;
	Adapter = (PHWT_ADAPTER)context;
	NetBufferList = NET_BUFFER_BUFFER_LIST(packet_data->m_net_buffer);
	NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_FAILURE;
	switch(InterlockedDecrement(&NET_BUFFER_LIST_BUFFER_COUNT(NetBufferList))) {
	case 0:
		NdisMSendNetBufferListsComplete(Adapter->AdapterHandle,
			NetBufferList,
			0);
		break;
	default:
		break;
	}	
	free_send_packet_data(packet_data);
}

PHWT_ADAPTER GetMPAdapter(PDEVICE_OBJECT device) {

	return (PHWT_ADAPTER)GlobalData.AdapterList.Flink;
}

