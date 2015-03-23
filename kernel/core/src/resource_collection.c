#include "resource_collection.h"
#include "thread_if.h"
#include "_scheduler.h"

#define Tag	'corp'

VOID ProcessObjectMonitorFunc(__in PVOID  StartContext) {

	struct PROCESS_OBJECT_MONITOR* monitor;
	PVOID objects[MAXIMUM_WAIT_OBJECTS];
	struct PROCESS_OBJECT_DATA* pdatas[MAXIMUM_WAIT_OBJECTS];
	PKWAIT_BLOCK waitblk;
	ULONG count;
	BOOLEAN loop;
	waitblk = (PKWAIT_BLOCK)ExAllocatePoolWithTag(NonPagedPool,
		MAXIMUM_WAIT_OBJECTS*sizeof(KWAIT_BLOCK),
		Tag);
	monitor = (struct PROCESS_OBJECT_MONITOR*)StartContext;
	loop = TRUE;
	while(loop) {
		count = 0;
		objects[count] = &monitor->m_control;
		pdatas[count] = NULL;
		count++;
		{
			KIRQL irql;
			LIST_ENTRY* entry;
			NTSTATUS status;
			LIST_ENTRY release;
			InitializeListHead(&release);			
			KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
			entry = monitor->m_process_list.m_process_head.Flink;
			while(1) {
				if (entry != &monitor->m_process_list.m_process_head) {
					struct PROCESS_OBJECT_DATA* pdata;
					pdata = (struct PROCESS_OBJECT_DATA*)entry;
					switch(pdata->m_reference) {
					case 0: {
							entry = entry->Flink;
							RemoveEntryList(&pdata->m_process_entry);
							InsertTailList(&release, &pdata->m_process_entry);
						}
						break;
					default: {
							objects[count] = pdata->m_process_object;
							pdatas[count] = pdata;
							count++;
							entry = entry->Flink;
						}
						break;
					}
					if (count < MAXIMUM_WAIT_OBJECTS)
						continue;
				}
				break;
			}
			KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
			
			entry = release.Flink;
			while(1) {
				if (entry != &release) {
					struct PROCESS_OBJECT_DATA* pdata;
					pdata = (struct PROCESS_OBJECT_DATA*)entry;
					entry = entry->Flink;
					RemoveEntryList(&pdata->m_process_entry);
					ReleaseProcessObjectData(pdata);
					continue;
				}
				break;
			}
			
			status = KeWaitForMultipleObjects(count,
				objects,
				WaitAny,
				Executive,
				KernelMode,
				FALSE,
				NULL,
				waitblk);
			switch(status) {
			case STATUS_WAIT_0: {
					KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
					if (monitor->m_control_mask & control_mask_exit)
						loop = FALSE;
					KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
				}
				break;
			default: {
					if (status >= STATUS_WAIT_0+0x01 &&
						status <= STATUS_WAIT_0+0x40) {						
						KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
						RemoveEntryList(&pdatas[status-STATUS_WAIT_0]->m_process_entry);						
						KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
						ReleaseProcessObjectData(pdatas[status-STATUS_WAIT_0]);
					}
				}
				break;
			}
		}
	}
	ExFreePoolWithTag(waitblk, Tag);
}

struct RESOURCE_OBJECT_DATA* AllocateResourceObjectData() {

	struct RESOURCE_OBJECT_DATA* rdata;
	rdata = (struct RESOURCE_OBJECT_DATA*)ExAllocatePoolWithTag(NonPagedPool, sizeof(struct RESOURCE_OBJECT_DATA), Tag);
	RtlZeroMemory(rdata, sizeof(struct RESOURCE_OBJECT_DATA));
	InitializeListHead(&rdata->m_resource_entry);
	return rdata;
}

VOID ReleaseResourceObjectData(struct RESOURCE_OBJECT_DATA* rdata) {

	RemoveEntryList(&rdata->m_resource_entry);
	ExFreePoolWithTag(rdata, Tag);
}


VOID InitializeResourceObjectList(struct RESOURCE_OBJECT_LIST* list) {

	InitializeListHead(&list->m_resource_head);
}

struct PROCESS_OBJECT_DATA* AllocateProcessObjectData() {

	struct PROCESS_OBJECT_DATA* pdata;
	pdata = (struct PROCESS_OBJECT_DATA*)ExAllocatePoolWithTag(NonPagedPool, sizeof(struct PROCESS_OBJECT_DATA), Tag);
	RtlZeroMemory(pdata, sizeof(struct PROCESS_OBJECT_DATA));
	InitializeListHead(&pdata->m_process_entry);
	InitializeResourceObjectList(&pdata->m_resource_list);
	pdata->m_reference = 1;
	return pdata;
}

VOID ReleaseProcessObjectData(struct PROCESS_OBJECT_DATA* pdata) {

	LIST_ENTRY* entry;
	entry = pdata->m_resource_list.m_resource_head.Flink;
	while(1) {
		if (entry != &pdata->m_resource_list.m_resource_head) {
			struct RESOURCE_OBJECT_DATA* rdata;			
			rdata = (struct RESOURCE_OBJECT_DATA*)entry;
			switch(rdata->m_code) {
			case UEXT_CMD(MAP_MOD_BUFFER): {
					SoraKUExtKForceUnmapModBuffer(rdata->m_uext_ke, 
						rdata->m_radio, 
						rdata->m_associated.m_map_mod_buf_out.ModBuf);						
					entry = entry->Flink;
					ReleaseResourceObjectData(rdata);
				}
				break;
			case UEXT_CMD(TX_RES_ALLOC): {
					SoraKTxDoneByID(&rdata->m_uext_ke->TxQueue, 
						rdata->m_transfer_obj, 
						rdata->m_associated.m_tx_res_alloc_out.TxID);
					entry = entry->Flink;
					ReleaseResourceObjectData(rdata);
				}
				break;
			case UEXT_CMD(MAP_RX_BUFFER): {
					SoraKUExtKForceUnmapRxBuf(rdata->m_uext_ke,
						rdata->m_radio,
						rdata->m_associated.m_map_rx_buf_out.RxBuf);	
					entry = entry->Flink;
					ReleaseResourceObjectData(rdata);
				}
				break;
/*				
			case UEXT_CMD(MOD_BUF_LOCK_ACQUIRE): {
					SoraKReleaseModBufLock(rdata->m_radio);
					entry = entry->Flink;
					ReleaseResourceObjectData(rdata);
				}
				break;
*/				
			case UEXT_CMD(VSTREAM_MASK_ALLOC): {
					SoraKUExtKReleaseVStreamMask(rdata->m_uext_ke,
						rdata->m_radio,
						rdata->m_associated.m_vstream_mask_alloc_out.VStreamMask);
					entry = entry->Flink;
					ReleaseResourceObjectData(rdata);
				}
				break;

			case UEXT_CMD(ALLOC_KERNEL_BUFFER): {
					if (rdata->m_free_routine)
						rdata->m_free_routine(rdata->m_free_context, rdata);
					entry = entry->Flink;
					ReleaseResourceObjectData(rdata);
				}
				break;	
			case UEXT_CMD(GET_SEND_PACKET): {
					if (rdata->m_free_routine)
						rdata->m_free_routine(rdata->m_free_context, rdata);
					entry = entry->Flink;
					ReleaseResourceObjectData(rdata);
				}
				break;				
			default: {
					if (rdata->m_free_routine)
						rdata->m_free_routine(rdata->m_free_context, rdata);				
					entry = entry->Flink;
					ReleaseResourceObjectData(rdata);
				}
				break;
			}
			continue;
		}
		break;
	}	
	ObDereferenceObject(pdata->m_process_object);
	ExFreePoolWithTag(pdata, Tag);
}

VOID InitializeProcessObjectList(struct PROCESS_OBJECT_LIST* list) {

	InitializeListHead(&list->m_process_head);
}

struct PROCESS_OBJECT_DATA* GetProcessObjectDataByProcessId(struct PROCESS_OBJECT_LIST* list, HANDLE id) {

	LIST_ENTRY* entry;
	entry = list->m_process_head.Flink;
	while(1) {
		if (entry != &list->m_process_head) {
			struct PROCESS_OBJECT_DATA* pdata;
			pdata = (struct PROCESS_OBJECT_DATA*)entry;
			if (pdata->m_process_id != id) {
				entry = entry->Flink;
				continue;
			}
			return pdata;
		}
		break;
	}
	return NULL;
}

struct PROCESS_OBJECT_MONITOR* AllocateProcessObjectMonitor() {

	struct PROCESS_OBJECT_MONITOR* monitor;
	monitor = (struct PROCESS_OBJECT_MONITOR*)ExAllocatePoolWithTag(NonPagedPool, sizeof(struct PROCESS_OBJECT_MONITOR), Tag);
    if (monitor == NULL) return NULL;

	RtlZeroMemory(monitor, sizeof(struct PROCESS_OBJECT_MONITOR));
	KeInitializeSpinLock(&monitor->m_monitor_lock);
	InitializeProcessObjectList(&monitor->m_process_list);
	monitor->m_control_mask = 0;
	KeInitializeEvent(&monitor->m_control, SynchronizationEvent, FALSE);

    // Create thread for ProcessObjectMonitor
    do
	{
		OBJECT_ATTRIBUTES attr;
		HANDLE thread_handle;
        NTSTATUS rc;

		InitializeObjectAttributes(&attr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
		rc = PsCreateSystemThread(&thread_handle,
			THREAD_ALL_ACCESS,
			&attr, 
			NULL,
			NULL,
			ProcessObjectMonitorFunc,
			monitor);
        if (rc != STATUS_SUCCESS)
        {
            ExFreePoolWithTag(monitor, Tag);
            monitor = NULL;
            break;
        }

		ObReferenceObjectByHandle(thread_handle,
			THREAD_ALL_ACCESS,
			NULL,
			KernelMode,
			&monitor->m_thread_object,
			NULL);
		ZwClose(thread_handle);

	} while (FALSE);

	return monitor;
};

VOID ReleaseProcessObjectMonitor(struct PROCESS_OBJECT_MONITOR* monitor) {

	KIRQL irql;
	LIST_ENTRY release;
	LIST_ENTRY* entry;
	InitializeListHead(&release);	
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	monitor->m_control_mask |= control_mask_exit;
	KeSetEvent(&monitor->m_control,
		IO_NO_INCREMENT,
		FALSE);
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
	KeWaitForSingleObject(monitor->m_thread_object, 
		Executive,
		KernelMode,
		FALSE,
		NULL);
	ObDereferenceObject(monitor->m_thread_object);
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	entry = monitor->m_process_list.m_process_head.Flink;
	while(1) {
		if (entry != &monitor->m_process_list.m_process_head) {
			struct PROCESS_OBJECT_DATA* pdata;
			pdata = (struct PROCESS_OBJECT_DATA*)entry;
			entry = entry->Flink;
			RemoveEntryList(&pdata->m_process_entry);
			InsertTailList(&release, &pdata->m_process_entry);
			continue;
		}
		break;
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);

	entry = release.Flink;
	while(1) {
		if (entry != &release) {
			struct PROCESS_OBJECT_DATA* pdata;
			pdata = (struct PROCESS_OBJECT_DATA*)entry;
			entry = entry->Flink;
			RemoveEntryList(&pdata->m_process_entry);
			ReleaseProcessObjectData(pdata);
			continue;
		}
		break;
	}	
	ExFreePoolWithTag(monitor, Tag);
}

VOID DoCollect(
	struct PROCESS_OBJECT_MONITOR* monitor,
	PUEXT_KE uext_ke,
	PTRANSFER_OBJ transfer_obj,
	PSORA_RADIO radio,
	ULONG code,
	PVOID output, 
	ULONG outsize) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct PROCESS_OBJECT_DATA* pdata;
		pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
		if (pdata) {
			struct RESOURCE_OBJECT_DATA* rdata;
			rdata = AllocateResourceObjectData();
			rdata->m_uext_ke = uext_ke;
			rdata->m_transfer_obj = transfer_obj;
			rdata->m_radio = radio;
			rdata->m_code = code;
			RtlCopyMemory(rdata->m_associated.m_data, output, outsize);
			InsertTailList(&pdata->m_resource_list.m_resource_head, &rdata->m_resource_entry);
		}
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
}

struct RESOURCE_OBJECT_DATA* 
DoCollectEx(
	struct PROCESS_OBJECT_MONITOR* monitor,
	PUEXT_KE uext_ke,
	PSORA_RADIO radio,
	ULONG code,
	PVOID output, 
	ULONG outsize,
	PFREE_ROUTINE free_routine,
	PVOID free_context, 
	BOOL force) {

	struct RESOURCE_OBJECT_DATA* rdata;
	rdata = NULL;
	
	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct PROCESS_OBJECT_DATA* pdata;
		pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
		if (!pdata) {
			if (force) {
				pdata = AllocateProcessObjectData();
				pdata->m_process_id = PsGetCurrentProcessId();
				pdata->m_process_object = PsGetCurrentProcess();						
				ObReferenceObject(pdata->m_process_object);
				InsertTailList(&monitor->m_process_list.m_process_head, &pdata->m_process_entry);
				KeSetEvent(&monitor->m_control, IO_NO_INCREMENT, FALSE);
			}
		}
		if (pdata) {			
			rdata = AllocateResourceObjectData();
			rdata->m_uext_ke = uext_ke;
			rdata->m_radio = radio;
			rdata->m_code = code;
			rdata->m_free_routine = free_routine;
			rdata->m_free_context = free_context;
			RtlCopyMemory(rdata->m_associated.m_data, output, outsize);
			InsertTailList(&pdata->m_resource_list.m_resource_head, &rdata->m_resource_entry);
		}
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);

	return rdata;

}

VOID DoRelease_UNMAP_MOD_BUFFER(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct PROCESS_OBJECT_DATA* pdata;
		pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
		if (pdata) {
			LIST_ENTRY* entry;
			entry = pdata->m_resource_list.m_resource_head.Flink;
			while(1) {
				if (entry != &pdata->m_resource_list.m_resource_head) {
					struct RESOURCE_OBJECT_DATA* rdata;
					rdata = (struct RESOURCE_OBJECT_DATA*)entry;
					if (rdata->m_code != UEXT_CMD(MAP_MOD_BUFFER)) {
						entry = entry->Flink;
						continue;
					}
					if (rdata->m_associated.m_map_mod_buf_out.ModBuf != ((PUNMAP_MOD_BUF_IN)input)->ModBuf) { 
						entry = entry->Flink;
						continue;
					}
					ReleaseResourceObjectData(rdata);
				}
				break;
			}
		}
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
}

VOID DoRelease_TX_RES_RELEASE(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct PROCESS_OBJECT_DATA* pdata;
		pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
		if (pdata) {
			LIST_ENTRY* entry;
			entry = pdata->m_resource_list.m_resource_head.Flink;
			while(1) {
				if (entry != &pdata->m_resource_list.m_resource_head) {
					struct RESOURCE_OBJECT_DATA* rdata;
					rdata = (struct RESOURCE_OBJECT_DATA*)entry;
					if (rdata->m_code != UEXT_CMD(TX_RES_ALLOC)) {
						entry = entry->Flink;
						continue;
					}
					if (rdata->m_associated.m_tx_res_alloc_out.TxID != ((PTX_RES_REL_IN)input)->TxID) {
						entry = entry->Flink;
						continue;
					}
					ReleaseResourceObjectData(rdata);
				}
				break;
			}			
		}
	}		
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
}

VOID DoRelease_UNMAP_RX_BUFFER(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct PROCESS_OBJECT_DATA* pdata;
		pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
		if (pdata) {
			LIST_ENTRY* entry;
			entry = pdata->m_resource_list.m_resource_head.Flink;
			while(1) {
				if (entry != &pdata->m_resource_list.m_resource_head) {
					struct RESOURCE_OBJECT_DATA* rdata;
					rdata = (struct RESOURCE_OBJECT_DATA*)entry;
					if (rdata->m_code != UEXT_CMD(MAP_RX_BUFFER)) {
						entry = entry->Flink;
						continue;
					}
					if (rdata->m_associated.m_map_rx_buf_out.RxBuf != ((PUNMAP_RX_BUF_IN)input)->UserBuffer) {
						entry = entry->Flink;
						continue;
					}
					ReleaseResourceObjectData(rdata);
				}
				break;
			}
		}
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
}

VOID DoRelease_MOD_BUF_LOCK_RELEASE(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct PROCESS_OBJECT_DATA* pdata;
		pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
		if (pdata) {
			LIST_ENTRY* entry;
			entry = pdata->m_resource_list.m_resource_head.Flink;
			while(1) {
				if (entry != &pdata->m_resource_list.m_resource_head) {
					struct RESOURCE_OBJECT_DATA* rdata;
					rdata = (struct RESOURCE_OBJECT_DATA*)entry;
					if (rdata->m_code != UEXT_CMD(MOD_BUF_LOCK_ACQUIRE)) {
						entry = entry->Flink;
						continue;
					}
					ReleaseResourceObjectData(rdata);
				}
				break;
			}
		}
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
}

VOID DoRelease_VSTREAM_MASK_RELEASE(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input,
	ULONG insize) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct PROCESS_OBJECT_DATA* pdata;
		pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
		if (pdata) {
			LIST_ENTRY* entry;
			entry = pdata->m_resource_list.m_resource_head.Flink;
			while(1) {
				if (entry != &pdata->m_resource_list.m_resource_head) {
					struct RESOURCE_OBJECT_DATA* rdata;
					rdata = (struct RESOURCE_OBJECT_DATA*)entry;
					if (rdata->m_code != UEXT_CMD(VSTREAM_MASK_ALLOC)) {
						entry = entry->Flink;
						continue;
					}
					// TODO: also check radio number after multiple radio supported
					if (rdata->m_associated.m_vstream_mask_alloc_out.VStreamMask != ((PVSTREAM_MASK_RELEASE_IN)input)->VStreamMask) {
						entry = entry->Flink;
						continue;
					}
					ReleaseResourceObjectData(rdata);
				}
				break;
			}
		}
	}	
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
}

struct RESOURCE_OBJECT_DATA* Get_COMPLETE_SEND_PACKET(struct PROCESS_OBJECT_MONITOR* monitor,
	PVOID input, 
	ULONG insize) {

	struct PROCESS_OBJECT_DATA* pdata;
	pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
	if (pdata) {
		LIST_ENTRY* entry;
		entry = pdata->m_resource_list.m_resource_head.Flink;
		while(1) {
			if (entry != &pdata->m_resource_list.m_resource_head) {
				struct RESOURCE_OBJECT_DATA* rdata;
				rdata = (struct RESOURCE_OBJECT_DATA*)entry;
				if (rdata->m_code != UEXT_CMD(GET_SEND_PACKET)) {
					entry = entry->Flink;
					continue;
				}
				if (rdata->m_associated.m_get_send_packet_out.Packet != ((COMPLETE_SEND_PACKET_IN*)input)->Packet) {
					entry = entry->Flink;
					continue;
				}
				return rdata;
			}
			break;
		}
	}
	return NULL;
}

VOID DoRelease_COMPLETE_SEND_PACKET(struct PROCESS_OBJECT_MONITOR* monitor,
	PVOID input,
	ULONG insize) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct RESOURCE_OBJECT_DATA* rdata;
		rdata = Get_COMPLETE_SEND_PACKET(monitor,
			input, 
			insize);
		if (rdata)
			ReleaseResourceObjectData(rdata);
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);	
}

struct RESOURCE_OBJECT_DATA* Get_RELEASE_KERNEL_BUFFER(struct PROCESS_OBJECT_MONITOR* monitor,
	PVOID input, 
	ULONG insize) {

	struct PROCESS_OBJECT_DATA* pdata;
	pdata = GetProcessObjectDataByProcessId(&monitor->m_process_list, PsGetCurrentProcessId());
	if (pdata) {
		LIST_ENTRY* entry;
		entry = pdata->m_resource_list.m_resource_head.Flink;
		while(1) {
			if (entry != &pdata->m_resource_list.m_resource_head) {
				struct RESOURCE_OBJECT_DATA* rdata;
				rdata = (struct RESOURCE_OBJECT_DATA*)entry;
				if (rdata->m_code != UEXT_CMD(ALLOC_KERNEL_BUFFER)) {
					entry = entry->Flink;
					continue;
				}
				if (rdata->m_associated.m_alloc_kernel_buffer_data.m_ubuf != ((RELEASE_KERNEL_BUFFER_IN*)input)->Buff) {
					entry = entry->Flink;
					continue;
				}
				return rdata;
			}
			break;
		}
	}
	return NULL;
}

VOID DoRelease_RELEASE_KERNEL_BUFFER(struct PROCESS_OBJECT_MONITOR* monitor,
	PVOID input,
	ULONG insize) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);
	{
		struct RESOURCE_OBJECT_DATA* rdata;
		rdata = Get_RELEASE_KERNEL_BUFFER(monitor,
			input, 
			insize);
		if (rdata)
			ReleaseResourceObjectData(rdata);
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);		
}

BOOL 
FreeResourceObjectData(
	struct PROCESS_OBJECT_MONITOR* monitor,
	struct RESOURCE_OBJECT_DATA* rdata) {

	KIRQL irql;
	KeAcquireSpinLock(&monitor->m_monitor_lock, &irql);

	struct PROCESS_OBJECT_DATA* pdata;
	pdata = GetProcessObjectDataByProcessId(
		&monitor->m_process_list, 
		PsGetCurrentProcessId());
	if (pdata) {
		LIST_ENTRY* entry;
		entry = pdata->m_resource_list.m_resource_head.Flink;
		while(1) {
			if (entry != &pdata->m_resource_list.m_resource_head) {
				if ((struct RESOURCE_OBJECT_DATA*)entry != rdata) {
					entry = entry->Flink;
					continue;
				}
				RemoveEntryList(&rdata->m_resource_entry);
				break;
			}
			rdata = NULL;
			break;
		}
	}
	KeReleaseSpinLock(&monitor->m_monitor_lock, irql);
	
	if (rdata) {
		if (rdata->m_free_routine)
			rdata->m_free_routine(rdata->m_free_context, rdata);
		ExFreePoolWithTag(rdata, Tag);
		return TRUE;
	}	
	return FALSE;	
}

