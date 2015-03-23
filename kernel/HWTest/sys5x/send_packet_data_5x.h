#pragma once

#include "send_packet_data.h"

struct Send_Packet_Data {

	PNDIS_PACKET m_ndis_packet;
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
	packet_data = (struct Send_Packet_Data*)rdata->m_associated.m_get_send_packet_out.Packet;
	Adapter = (PHWT_ADAPTER)context;	
	NDIS_SET_PACKET_STATUS(packet_data->m_ndis_packet, 
		NDIS_STATUS_FAILURE);
	NdisMSendComplete(Adapter->AdapterHandle,
		packet_data->m_ndis_packet,
		NDIS_STATUS_FAILURE);
	free_send_packet_data(packet_data);
}

PHWT_ADAPTER GetMPAdapter(PDEVICE_OBJECT device) {

	PHWT_ADAPTER adapter;
	NdisAcquireSpinLock(&GlobalData.Lock);
	for(adapter=(PHWT_ADAPTER)GlobalData.AdapterList.Flink; &adapter->List!=&GlobalData.AdapterList; adapter=(PHWT_ADAPTER)adapter->List.Flink) {
		if (adapter->Fdo != device->NextDevice)
			continue;
		NdisReleaseSpinLock(&GlobalData.Lock);
		return adapter;
	}
	NdisReleaseSpinLock(&GlobalData.Lock);
	return NULL;
}

