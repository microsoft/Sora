#include "mp_5x.h"
#include "sendrcvhelp.h"
#include "dot11_pkt.h"
#include "send_packet_data_5x.h"

#define INFINITE 0xFFFFFFFF

struct Send_Packet_Data*
PreparePacketData(UINT buffer_count,
	UINT pending_len) {

	struct Send_Packet_Data* packet_data;
	packet_data = NULL;
	{
		unsigned char* data;
		data = ExAllocatePoolWithTag(NonPagedPool,
			sizeof(struct Send_Packet_Data) + pending_len,
			'tkpi');
		if (data) {
			packet_data = (struct Send_Packet_Data*)data;
			RtlZeroMemory(packet_data, sizeof(struct Send_Packet_Data));
			packet_data->m_data = data + sizeof(struct Send_Packet_Data);
		}
	}
	return packet_data;
}

struct Send_Packet_Data*
Packet2PacketData(PNDIS_PACKET Packet) {

	UINT buffer_count;
	PNDIS_BUFFER buffer;
	UINT packet_len;
	UINT offset;
	PVOID addr;
	UINT len;
	struct Send_Packet_Data* packet_data;
	packet_data = NULL;
	
	NdisQueryPacket(Packet,
		NULL,
		&buffer_count,
		&buffer,
		&packet_len);
	if (!buffer_count ||
		!buffer)
		goto error_exit;

	if (buffer_count > 1) {
		packet_data = PreparePacketData(buffer_count,
			packet_len);
		if (!packet_data)
			goto error_exit;
		
		offset = 0;
		while(1) {
			NdisQueryBufferSafe(buffer,
				&addr,
				&len,
				NormalPagePriority);
			if (!addr ||
				!len)
				goto error_exit;
		
			RtlCopyMemory(packet_data->m_data+offset, 
				addr, 
				len);
			offset += len;
		
			NdisGetNextBuffer(buffer, 
				&buffer);
			if (!buffer)
				break;
		}
		packet_data->m_ndis_packet = Packet;
		packet_data->m_len = packet_len;
	}
	else {
		packet_data = PreparePacketData(buffer_count,
			0);
		if (!packet_data)
			goto error_exit;

		NdisQueryBufferSafe(buffer,
			&addr,
			&len,
			NormalPagePriority);
		if (!addr ||
			!len)
			goto error_exit;
		
		packet_data->m_ndis_packet = Packet;
		packet_data->m_len = packet_len;		
		packet_data->m_data = addr;
	}

	packet_data->m_mdl = IoAllocateMdl(packet_data->m_data,
		packet_data->m_len,
		FALSE,
		FALSE,
		NULL);
	if (!packet_data->m_mdl)
		goto error_exit;

	try {
		MmProbeAndLockPages(packet_data->m_mdl,
			KernelMode,
			IoReadAccess);
		packet_data->m_locked = 1;
	}
	except(EXCEPTION_EXECUTE_HANDLER) {
		packet_data->m_locked = 0;
	}

	packet_data->m_addr = MmMapLockedPagesSpecifyCache(packet_data->m_mdl,
		UserMode,
		MmCached,
		NULL,
		FALSE,
		NormalPagePriority);
	if (!packet_data->m_addr)
		goto error_exit;

	packet_data->m_map_process = PsGetCurrentProcessId();

	return packet_data;
	
error_exit:
	DbgPrint("Packet2PacketData failed, force to complete the lost packet\n");
	free_send_packet_data(packet_data);
	return NULL;
}

VOID 
MPSendPackets(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PPNDIS_PACKET PacketArray,
    IN UINT NumberOfPackets) {

	PHWT_ADAPTER Adapter;
	KIRQL irql;
	UINT Index;
	Adapter = (PHWT_ADAPTER)MiniportAdapterContext;
	
	KeAcquireSpinLock(&Adapter->UExtKeObj.SendPacketLock,
		&irql);
	if (Adapter->UExtKeObj.SendPacketControl) {
		for(Index=0; Index < NumberOfPackets; Index++) {
			if (get_thread_safe_enlist_count(Adapter->UExtKeObj.SendPacketList) >= MAX_QUEUED_SEND_PACKET) {
				NDIS_SET_PACKET_STATUS(PacketArray[Index], NDIS_STATUS_FAILURE);
				NdisMSendComplete(Adapter->AdapterHandle,
					PacketArray[Index],
					NDIS_STATUS_FAILURE);
				continue;
			}
			insert_thread_safe_enlist_tail(Adapter->UExtKeObj.SendPacketList, 
				PacketArray[Index],
				1); 
			KeReleaseSemaphore(Adapter->UExtKeObj.SendPacketControl,
				IO_NO_INCREMENT,
				1,
				FALSE);
		}
	}
	else {
		for(Index=0; Index < NumberOfPackets; Index++) {
			NDIS_SET_PACKET_STATUS(PacketArray[Index], NDIS_STATUS_FAILURE);
			NdisMSendComplete(Adapter->AdapterHandle,
				PacketArray[Index],
				NDIS_STATUS_FAILURE);
		}
	}
	KeReleaseSpinLock(&Adapter->UExtKeObj.SendPacketLock,
		irql);
    return;
}

VOID 
MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET Packet)
{
	if (Packet) {
		PVOID data = NULL;
		PNDIS_BUFFER ndis_buffer = NULL;
		data = *(PVOID*)Packet->MiniportReserved;
		if (data)
			ExFreePoolWithTag(data, 'tkpi');
		while(1) {
			NdisUnchainBufferAtBack(Packet, &ndis_buffer);
			if (ndis_buffer)
				NdisFreeBuffer(ndis_buffer);
			else
				break;
		}		
		NdisFreePacket(Packet);
	}
}

NTSTATUS
MPIndicatePacket(PHWT_ADAPTER Adapter, PVOID Input, ULONG InputSize, PDEV_RET Output, ULONG OutputSize, PIRP Irp) {

	PVOID data = NULL;
	NDIS_STATUS ndis_status = NDIS_STATUS_FAILURE;
	PNDIS_BUFFER ndis_buffer = NULL;
	PNDIS_PACKET ndis_packet = NULL;	
	do {
		data = ExAllocatePoolWithTag(NonPagedPool, InputSize, 'tkpi');

		RtlCopyMemory(data, Input, InputSize);
				
		NdisAllocateBuffer(&ndis_status, 
                           &ndis_buffer, 
                           Adapter->AllocateBufferPool,
                           data,
                           InputSize);
		if (ndis_status != NDIS_STATUS_SUCCESS)
			break;

		NdisAllocatePacket(&ndis_status,
                           &ndis_packet,
                           Adapter->AllocatePacketPool);
		if (ndis_status != NDIS_STATUS_SUCCESS)
			break;

		NdisReinitializePacket(ndis_packet);
		*(PVOID*)ndis_packet->MiniportReserved = data;
		NdisChainBufferAtBack(ndis_packet, ndis_buffer);
		NDIS_SET_PACKET_HEADER_SIZE(ndis_packet, sizeof(ETHERNET_HEADER));
		NDIS_SET_PACKET_STATUS(ndis_packet, NDIS_STATUS_SUCCESS);
		NdisMIndicateReceivePacket(Adapter->AdapterHandle, &ndis_packet, 1);

		Output->hResult = S_OK;

		ndis_status = NDIS_STATUS_SUCCESS;
	} while(FALSE);
	
	if (ndis_status != NDIS_STATUS_SUCCESS &&
		ndis_status != STATUS_PENDING) {
		if (ndis_packet)
			NdisFreePacket(ndis_packet);
		if (ndis_buffer) 
			NdisFreeBuffer(ndis_buffer);		
		if (data)
			ExFreePoolWithTag(data, 'tkpi');
	}
	return ndis_status;
}

NTSTATUS
MPGetSendPacket(PHWT_ADAPTER Adapter, PVOID Input, ULONG InputSize, PVOID Output, ULONG OutputSize, PIRP Irp) {

	PGET_SEND_PACKET_OUT get_send_packet_out;
	PNDIS_PACKET Packet;
	get_send_packet_out = (PGET_SEND_PACKET_OUT)Output;

	if (remove_thread_safe_enlist_head(Adapter->UExtKeObj.SendPacketList, 
		&Packet,
		1)) {
		struct Send_Packet_Data* packet_data;
		packet_data = Packet2PacketData(Packet);
		if (packet_data) {
			get_send_packet_out->hResult = ERROR_SUCCESS;
			get_send_packet_out->Packet = (PACKET_HANDLE)packet_data;
			get_send_packet_out->Addr= packet_data->m_addr;
			get_send_packet_out->Size = packet_data->m_len;	
			return STATUS_SUCCESS;
		}
		else {
			NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_FAILURE);
			NdisMSendComplete(Adapter->AdapterHandle,
				Packet,
				NDIS_STATUS_FAILURE);
		}
	}
	get_send_packet_out->hResult = ERROR_RESOURCE_FAILED;
	get_send_packet_out->Packet = 0;
	get_send_packet_out->Addr= NULL;
	get_send_packet_out->Size = 0;
	return STATUS_SUCCESS;
}

NTSTATUS
MPCompleteSendPacket(PHWT_ADAPTER Adapter, PVOID Input, ULONG InputSize, PDEV_RET Output, ULONG OutputSize, PIRP Irp) {

	PCOMPLETE_SEND_PACKET_IN complete_send_packet_in;
	struct Send_Packet_Data* packet_data;
	complete_send_packet_in = (PCOMPLETE_SEND_PACKET_IN)Input;
	packet_data = (struct Send_Packet_Data*)complete_send_packet_in->Packet;
	NDIS_SET_PACKET_STATUS(packet_data->m_ndis_packet, 
		complete_send_packet_in->hResult);
	NdisMSendComplete(Adapter->AdapterHandle,
		packet_data->m_ndis_packet,
		complete_send_packet_in->hResult);	
	free_send_packet_data(packet_data);
	Output->hResult = ERROR_SUCCESS;
	return STATUS_SUCCESS;
}
