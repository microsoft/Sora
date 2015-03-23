#include "hwtest_miniport.h"

INT             MPDebugLevel = MP_INFO;
NDIS_HANDLE     NdisWrapperHandle;
MP_GLOBAL_DATA  GlobalData;
NDIS_HANDLE     g_NdisDeviceHandle = NULL; 
PDEVICE_OBJECT  g_ControlDeviceObject = NULL;

void MpFreeAdapter(PHWT_ADAPTER pAdapter)
{
    if (pAdapter)
    {
        NdisFreeMemory(pAdapter, sizeof(HWT_ADAPTER), 0);
    }
}

NDIS_STATUS
MpAllocateAdapter(
    __in  NDIS_HANDLE             AdapterHandle,
    __deref_out_opt PHWT_ADAPTER* Adapter
    )

{
#ifdef NDIS620_MINIPORT
    *Adapter = NdisAllocateMemoryWithTagPriority(
                        AdapterHandle,
                        sizeof(HWT_ADAPTER),
                        NIC_TAG,
                        NormalPoolPriority);
#else
    *Adapter = NULL;
    NdisAllocateMemoryWithTag(
        Adapter, 
        sizeof(HWT_ADAPTER),
        NIC_TAG);
#endif
    
    if (*Adapter == NULL) 
    {
        return NDIS_STATUS_RESOURCES;
    }
    else
    {
        NdisZeroMemory(*Adapter, sizeof(HWT_ADAPTER));

        (*Adapter)->AdapterHandle = AdapterHandle;
    }

#ifdef NDIS620_MINIPORT
    (*Adapter)->PortNumber                  =  0;
#endif

    return NDIS_STATUS_SUCCESS;
}

void NICAttachAdapter(PHWT_ADAPTER Adapter)
{
    PAGED_CODE();
    NdisInterlockedInsertTailList(
        &GlobalData.AdapterList, 
        &Adapter->List, 
        &GlobalData.Lock);
	Adapter->Attached = 1;
}

void NICDettachAdapter(PHWT_ADAPTER Adapter)
{
    PAGED_CODE();
	if (Adapter->Attached) {
		NdisAcquireSpinLock(&GlobalData.Lock);
		RemoveEntryList(&Adapter->List);
		NdisReleaseSpinLock(&GlobalData.Lock);
		Adapter->Attached = 0;
	}
}

