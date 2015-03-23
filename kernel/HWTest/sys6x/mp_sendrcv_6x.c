#include "mp_6x.h"
#include "send_packet_data_6x.h"

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
Packet2PacketData(PNET_BUFFER net_buffer) {

	UINT buffer_count;
	UINT buffer_size;
	PMDL mdl;
	UINT offset;	
	UINT packet_data_offset;
	PVOID addr;
	UINT len;
	struct Send_Packet_Data* packet_data;
	packet_data = NULL;

	buffer_count = 0;
	buffer_size = NET_BUFFER_DATA_LENGTH(net_buffer);
	
	mdl = NET_BUFFER_CURRENT_MDL(net_buffer);
	offset = NET_BUFFER_CURRENT_MDL_OFFSET(net_buffer);

	while(buffer_size) {
		ULONG count;
		count = MmGetMdlByteCount(mdl) - offset;
		offset = 0;
		buffer_count++;
		if (buffer_size <= count)
			break;		
		buffer_size -= count;
		mdl = mdl->Next;
	}

	if (!buffer_count ||
		!buffer_size) 
		goto error_exit;

	if ( buffer_count > 1) {
		packet_data = PreparePacketData(buffer_count,
			NET_BUFFER_DATA_LENGTH(net_buffer));
		if (!packet_data)
			goto error_exit;

		buffer_size = NET_BUFFER_DATA_LENGTH(net_buffer);

		mdl = NET_BUFFER_CURRENT_MDL(net_buffer);
		offset = NET_BUFFER_CURRENT_MDL_OFFSET(net_buffer);
		
		packet_data_offset = 0;
		while(buffer_size) {
			ULONG count;
			count = MmGetMdlByteCount(mdl) - offset;
			addr = (unsigned char*)MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority) + offset;
			offset = 0;
			if (buffer_size <= count) 
				count = buffer_size;
			RtlCopyMemory(packet_data->m_data + packet_data_offset, 
				addr, 
				count);
			packet_data_offset += count;
			buffer_size -= count;			
			mdl = mdl->Next;			
		}
		packet_data->m_net_buffer = net_buffer;
		packet_data->m_len = NET_BUFFER_DATA_LENGTH(net_buffer);
	}
	else {
		packet_data = PreparePacketData(buffer_count,
			0);
		if (!packet_data)
			goto error_exit;

		mdl = NET_BUFFER_CURRENT_MDL(net_buffer);
		offset = NET_BUFFER_CURRENT_MDL_OFFSET(net_buffer);
		addr = (unsigned char*)MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority) + offset;

		packet_data->m_net_buffer = net_buffer;
		packet_data->m_data = addr;
		packet_data->m_len = NET_BUFFER_DATA_LENGTH(net_buffer);
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

/*
	Each NET_BUFFER contains an ethernet frame. 
	Each NET_BUFFER_LIST contains 1 or more than 1 NET_BUFFERs, which means 1 or more than 1 ethernet frames.
	In current design, we reflect each ethernet frame to USER and USER is able to replace ethernet header with wlan header.
	Base on this, 1 NET_BUFFER_LIST may reflect as many times as number of NET_BUFFERs inside it.
	When should we complete the NET_BUFFER_LIST?
	We put each NET_BUFFER to the SendPacketList and set 
		NET_BUFFER_LIST_MINIPORT_RESERVED(NetBufferLists)[1] = number of NET_BUFFERs;
		NET_BUFFER_MINIPORT_RESERVED(NetBuffer)[1] = NetBufferLists;		
	Everytime USER completes 1 NET_BUFFER
		NET_BUFFER_LIST_MINIPORT_RESERVED(NetBufferLists)[1]--;
	if (NET_BUFFER_LIST_MINIPORT_RESERVED(NetBufferLists)[1] == 0) 
		NdisMSendNetBufferListsComplete(NetBufferLists);
	else
		do nothing;
*/

VOID
MPSendNetBufferLists(
    NDIS_HANDLE             MiniportAdapterContext,
    PNET_BUFFER_LIST        NetBufferLists,
    NDIS_PORT_NUMBER        PortNumber,
    ULONG                   SendFlags
    )
{
    PNET_BUFFER_LIST            currentNetBufferList;
	PNET_BUFFER_LIST            nextNetBufferList;
	PNET_BUFFER 				currentNetBuffer;
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_FAILURE;
    PHWT_ADAPTER                Adapter = (PHWT_ADAPTER)MiniportAdapterContext;
	ULONG buffer_counts;
	KIRQL irql;
	KeAcquireSpinLock(&Adapter->UExtKeObj.SendPacketLock,
		&irql);
	if (Adapter->UExtKeObj.SendPacketControl) {
		for(currentNetBufferList = NetBufferLists; 
			currentNetBufferList; 
			currentNetBufferList = nextNetBufferList) {
			nextNetBufferList = NET_BUFFER_LIST_NEXT_NBL(currentNetBufferList);
			NET_BUFFER_LIST_NEXT_NBL(currentNetBufferList) = NULL;
			
			buffer_counts = 0;

			currentNetBuffer = NET_BUFFER_LIST_FIRST_NB(currentNetBufferList);
			while(currentNetBuffer) {								
				buffer_counts++;
				currentNetBuffer = NET_BUFFER_NEXT_NB(currentNetBuffer);
			}

			if (get_thread_safe_enlist_count(Adapter->UExtKeObj.SendPacketList) + buffer_counts >= MAX_QUEUED_SEND_PACKET) {
				NET_BUFFER_LIST_STATUS(currentNetBufferList) = NDIS_STATUS_FAILURE;
				NdisMSendNetBufferListsComplete(Adapter->AdapterHandle,
					currentNetBufferList,
					0);
				continue;
			}
			NET_BUFFER_LIST_BUFFER_COUNT(currentNetBufferList) = buffer_counts;
			NET_BUFFER_LIST_STATUS(currentNetBufferList) = NDIS_STATUS_SUCCESS;
				
			currentNetBuffer = NET_BUFFER_LIST_FIRST_NB(currentNetBufferList);
			while(currentNetBuffer) {
				NET_BUFFER_BUFFER_LIST(currentNetBuffer) = currentNetBufferList;
				
				insert_thread_safe_enlist_tail(Adapter->UExtKeObj.SendPacketList, 
					currentNetBuffer,
					1);
				KeReleaseSemaphore(Adapter->UExtKeObj.SendPacketControl,
					IO_NO_INCREMENT,
					1, 
					FALSE);
				currentNetBuffer = NET_BUFFER_NEXT_NB(currentNetBuffer);
			}
		}
	}
	else {
		for(currentNetBufferList  = NetBufferLists; 
			currentNetBufferList; 
			currentNetBufferList  = NET_BUFFER_LIST_NEXT_NBL(currentNetBufferList))
			NET_BUFFER_LIST_STATUS(currentNetBufferList) = NDIS_STATUS_FAILURE;
			
		NdisMSendNetBufferListsComplete(Adapter->AdapterHandle,
			NetBufferLists,
			0);
	}
	KeReleaseSpinLock(&Adapter->UExtKeObj.SendPacketLock,
		irql);
}

VOID 
MPCancelSendNetBufferLists(
    NDIS_HANDLE             MiniportAdapterContext,
    PVOID                   CancelId
    )
{
    // TODO: Unsure how to identify which port the packets have been
    // sent on
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(CancelId);
    ASSERT(FALSE);
}

VOID 
MPReturnNetBufferLists(
    NDIS_HANDLE             MiniportAdapterContext,
    PNET_BUFFER_LIST        NetBufferLists,
    ULONG                   ReturnFlags
    )
{
    PHWT_ADAPTER adapter = (PHWT_ADAPTER)MiniportAdapterContext;

	while (NetBufferLists) {
		PVOID data = NULL;
		PNET_BUFFER net_buffer = NULL;
		PNET_BUFFER_LIST NextNetBufferLists;
		PMDL ndis_mdl = NULL;
		data = NET_BUFFER_LIST_MINIPORT_RESERVED(NetBufferLists)[0];
		net_buffer = NET_BUFFER_LIST_FIRST_NB(NetBufferLists);
		ndis_mdl = NET_BUFFER_FIRST_MDL(net_buffer);		
		NextNetBufferLists = NET_BUFFER_LIST_NEXT_NBL(NetBufferLists);
		NdisFreeNetBufferList(NetBufferLists);
		NetBufferLists = NextNetBufferLists;
		if (ndis_mdl) {
			NdisFreeMdl(ndis_mdl);
			ndis_mdl = NULL;
		}
		if (data) {
			ExFreePoolWithTag(data, 'tkpi');
			data = NULL;
		}
	}	
}

NTSTATUS
MPIndicatePacket(PHWT_ADAPTER Adapter, PVOID Input, ULONG InputSize, PDEV_RET Output, ULONG OutputSize, PIRP Irp) {

	PVOID data = NULL;
	NDIS_STATUS ndis_status = NDIS_STATUS_FAILURE;
	PMDL ndis_mdl = NULL;
	PNET_BUFFER_LIST net_buffer_list = NULL;
	do {
		data = ExAllocatePoolWithTag(NonPagedPool, InputSize, 'tkpi');
		if (!data)
			break;		
		RtlCopyMemory(data, Input, InputSize);
		ndis_mdl = NdisAllocateMdl(Adapter->AdapterHandle,
			data,
			InputSize);
		if (!ndis_mdl)
			break;		
		net_buffer_list = NdisAllocateNetBufferAndNetBufferList(Adapter->AllocateNetBufferListPool, 
                                                                0,
                                                                0,
                                                                ndis_mdl, 
                                                                0,
                                                                InputSize);
		if (!net_buffer_list)
			break;
		NET_BUFFER_LIST_MINIPORT_RESERVED(net_buffer_list)[0] = data;
		NdisMIndicateReceiveNetBufferLists(Adapter->AdapterHandle,
                                           net_buffer_list,
                                           Adapter->PortNumber,
                                           1,
                                           0);
		Output->hResult = S_OK;	
		ndis_status = NDIS_STATUS_SUCCESS;
	} while(FALSE);

	if (ndis_status != NDIS_STATUS_SUCCESS &&
		ndis_status != STATUS_PENDING) {
		if (net_buffer_list) {
			NdisFreeNetBufferList(net_buffer_list);
			net_buffer_list = NULL;
		}		
		if (ndis_mdl) {
			NdisFreeMdl(ndis_mdl);
			ndis_mdl = NULL;
		}
		if (data) {
			ExFreePoolWithTag(data, 'tkpi');
			data = NULL;
		}
	}
	return ndis_status;
}

NTSTATUS
MPGetSendPacket(PHWT_ADAPTER Adapter, PVOID Input, ULONG InputSize, PVOID Output, ULONG OutputSize, PIRP Irp) {

	PGET_SEND_PACKET_OUT get_send_packet_out;
	PNET_BUFFER net_buffer;
	get_send_packet_out = (PGET_SEND_PACKET_OUT)Output;
	if (remove_thread_safe_enlist_head(Adapter->UExtKeObj.SendPacketList, 
		&net_buffer,
		1)) {
		struct Send_Packet_Data* packet_data;
		packet_data = Packet2PacketData(net_buffer);
		if (packet_data) {
			get_send_packet_out->hResult = ERROR_SUCCESS;
			get_send_packet_out->Packet = (PACKET_HANDLE)packet_data;
			get_send_packet_out->Addr= packet_data->m_addr;
			get_send_packet_out->Size = packet_data->m_len;	
			return STATUS_SUCCESS;
		}
		else {
			PNET_BUFFER_LIST NetBufferList;
			NetBufferList = NET_BUFFER_BUFFER_LIST(net_buffer);			
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
	PNET_BUFFER_LIST NetBufferList;
	complete_send_packet_in = (PCOMPLETE_SEND_PACKET_IN)Input;
	packet_data = (struct Send_Packet_Data*)complete_send_packet_in->Packet;
	NetBufferList = NET_BUFFER_BUFFER_LIST(packet_data->m_net_buffer);
	if (complete_send_packet_in->hResult != NDIS_STATUS_SUCCESS)
		NET_BUFFER_LIST_STATUS(NetBufferList) = complete_send_packet_in->hResult;
	free_send_packet_data(packet_data);
	switch(InterlockedDecrement(&NET_BUFFER_LIST_BUFFER_COUNT(NetBufferList))) {
	case 0:
		NdisMSendNetBufferListsComplete(Adapter->AdapterHandle,
			NetBufferList,
			0);
		break;
	default:
		break;
	}
	Output->hResult = ERROR_SUCCESS;
	return STATUS_SUCCESS;
}

