#pragma once

#define control_mask_exit 		1 << 0

#include "sora.h"
#include "__transfer_obj.h"

struct RESOURCE_OBJECT_DATA;

typedef VOID (*PFREE_ROUTINE)(PVOID context, struct RESOURCE_OBJECT_DATA* rdata);

struct RESOURCE_OBJECT_DATA {

	LIST_ENTRY m_resource_entry;
	PUEXT_KE m_uext_ke;
	PTRANSFER_OBJ m_transfer_obj;
	PSORA_RADIO m_radio;
	ULONG m_code;
	PFREE_ROUTINE m_free_routine;
	PVOID m_free_context;
	union {
		UCHAR m_data[1];
		MAP_MOD_BUF_OUT m_map_mod_buf_out;
		TX_RES_ALLOC_OUT m_tx_res_alloc_out;
		VSTREAM_MASK_ALLOC_OUT m_vstream_mask_alloc_out;
		MAP_RX_BUF_OUT m_map_rx_buf_out;
		GET_SEND_PACKET_OUT m_get_send_packet_out;
		struct ALLOC_KERNEL_BUFFER_DATA m_alloc_kernel_buffer_data;
		HRESULT m_hresult;
	} m_associated;
};

#ifdef __cplusplus
extern "C" {
#endif

struct RESOURCE_OBJECT_DATA* AllocateResourceObjectData();
VOID ReleaseResourceObjectData(struct RESOURCE_OBJECT_DATA* rdata);

struct RESOURCE_OBJECT_LIST {

	LIST_ENTRY m_resource_head;
};

VOID InitializeResourceObjectList(struct RESOURCE_OBJECT_LIST* list);

struct PROCESS_OBJECT_DATA {

	LIST_ENTRY m_process_entry; 	
	struct RESOURCE_OBJECT_LIST m_resource_list;
	LONG m_reference;	
	HANDLE m_process_id;
	PVOID m_process_object;	
};

struct PROCESS_OBJECT_DATA* AllocateProcessObjectData();
VOID ReleaseProcessObjectData(struct PROCESS_OBJECT_DATA* pdata);

struct PROCESS_OBJECT_LIST {
	
	LIST_ENTRY m_process_head;	
};

VOID InitializeProcessObjectList(struct PROCESS_OBJECT_LIST* list);
struct PROCESS_OBJECT_DATA* GetProcessObjectDataByProcessId(struct PROCESS_OBJECT_LIST* list, HANDLE id);

struct PROCESS_OBJECT_MONITOR {

	KSPIN_LOCK m_monitor_lock;
	struct PROCESS_OBJECT_LIST m_process_list;
	ULONG m_control_mask;
	KEVENT m_control;
	PVOID m_thread_object;
};

struct PROCESS_OBJECT_MONITOR* AllocateProcessObjectMonitor();
VOID ReleaseProcessObjectMonitor(struct PROCESS_OBJECT_MONITOR* monitor);

VOID DoCollect(
	struct PROCESS_OBJECT_MONITOR* monitor,
	PUEXT_KE uext_ke,
	PTRANSFER_OBJ transfer_obj,
	PSORA_RADIO radio,
	ULONG code,
	PVOID output, 
	ULONG outsize);

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
	BOOL force);

BOOL 
FreeResourceObjectData(
	struct PROCESS_OBJECT_MONITOR* monitor,
	struct RESOURCE_OBJECT_DATA* rdata);

VOID DoRelease_UNMAP_MOD_BUFFER(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize);

VOID DoRelease_TX_RES_RELEASE(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize);

VOID DoRelease_UNMAP_RX_BUFFER(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize);

VOID DoRelease_MOD_BUF_LOCK_RELEASE(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize);

VOID DoRelease_VSTREAM_MASK_RELEASE(struct PROCESS_OBJECT_MONITOR* monitor, 
	PVOID input, 
	ULONG insize);

struct RESOURCE_OBJECT_DATA* Get_COMPLETE_SEND_PACKET(struct PROCESS_OBJECT_MONITOR* monitor,
	PVOID input, 
	ULONG insize);

VOID DoRelease_COMPLETE_SEND_PACKET(struct PROCESS_OBJECT_MONITOR* monitor,
	PVOID input,
	ULONG insize);

struct RESOURCE_OBJECT_DATA* Get_RELEASE_KERNEL_BUFFER(struct PROCESS_OBJECT_MONITOR* monitor,
	PVOID input, 
	ULONG insize);

VOID DoRelease_RELEASE_KERNEL_BUFFER(struct PROCESS_OBJECT_MONITOR* monitor,
	PVOID input,
	ULONG insize);

#ifdef __cplusplus
	}
#endif

