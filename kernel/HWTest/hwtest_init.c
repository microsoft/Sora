#include "hwtest_miniport.h"
#include <ntstrsafe.h>
#include "__transfer_obj.h"
#include "thread_if.h"

#define NIC_RECV_POOL_SIZE   256

void NICCleanAdapter(IN  PHWT_ADAPTER  Adapter)
{
	if (Adapter->WrapperPageVAddr) {
		MmFreeContiguousMemory(Adapter->WrapperPageVAddr);
		Adapter->WrapperPageVAddr = NULL;
	}

	if (Adapter->Thread) {
		SoraThreadFree(Adapter->Thread);
		Adapter->Thread = NULL;
	}

    SoraKUExtKeDtor(&Adapter->UExtKeObj, &Adapter->TransferObj);

	{
		LIST_ENTRY radio_list;
		int i;
		InitializeListHead(&radio_list);
		for(i=0; i < MAX_RADIO_NUMBER; i++)
			if (Adapter->Radios2[i]) {
				InitializeListHead(&Adapter->Radios2[i]->RadiosList);
				InsertTailList(&radio_list, &Adapter->Radios2[i]->RadiosList);
				Adapter->Radios2[i] = NULL;
			}
		if (!IsListEmpty(&radio_list))
    		SoraReleaseRadios(&radio_list);
	}
	
#if NDIS_SUPPORT_NDIS6
	if (Adapter->AllocateNetBufferListPool) {
		NdisFreeNetBufferListPool(Adapter->AllocateNetBufferListPool);
		Adapter->AllocateNetBufferListPool = NULL;
	}
#else
	if (Adapter->AllocatePacketPool) {
		NdisFreePacketPool(Adapter->AllocatePacketPool);
		Adapter->AllocatePacketPool = NULL;
	}
	if (Adapter->AllocateBufferPool) {
		NdisFreeBufferPool(Adapter->AllocateBufferPool);
		Adapter->AllocateBufferPool = NULL;
	}	
#endif	

	if (Adapter->TransferObj.__TxSampleBufVa) {
		MmFreeContiguousMemorySpecifyCache(
			Adapter->TransferObj.__TxSampleBufVa,				
			Adapter->TransferObj.__TxSampleBufSize,
			MmWriteCombined);
		Adapter->TransferObj.__TxSampleBufVa	= NULL;
		Adapter->TransferObj.__TxSampleBufSize	= 0;
	}

	if (Adapter->SoraRadioManager) {
		SoraReleaseRadioManager();
		Adapter->SoraRadioManager = NULL;
	}
}

void HwtInfoCtor(PHWT_INFO Info)
{
    Info->ullBlocksLag = 0;
    Info->ullBlocksInTime = 0;
}

NDIS_STATUS NICInitializeAdapter(
    IN  PHWT_ADAPTER  Adapter,
    IN  NDIS_HANDLE   NdisHandle
    )
{
    //#pragma prefast(suppress: 8199, "")
    NDIS_HANDLE         ConfigurationHandle;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    UINT                Length = 0;
    PVOID               pNetAddrBuf = NULL;

    ULONG               index;
    HRESULT             hr;
    PSORA_RADIO         pRadio;

#ifdef NDIS620_MINIPORT
    NDIS_CONFIGURATION_OBJECT  ConfigObject;
#endif

    DEBUGP(MP_TRACE, ("[HT_STARTUP]---> InitializeAdapter\n"));
    PAGED_CODE();

	Adapter->Removed = 0;
	KeInitializeEvent(&Adapter->Exit,
		NotificationEvent,
		TRUE);

    do {
#ifdef NDIS620_MINIPORT
        //Init ConfigObject
        ConfigObject.Header.Type        = NDIS_OBJECT_TYPE_CONFIGURATION_OBJECT;
        ConfigObject.Header.Revision    = NDIS_CONFIGURATION_OBJECT_REVISION_1;
        ConfigObject.Header.Size        = NDIS_SIZEOF_CONFIGURATION_OBJECT_REVISION_1;
        ConfigObject.NdisHandle         = NdisHandle;
        ConfigObject.Flags              = NDIS_CONFIG_FLAG_FILTER_INSTANCE_CONFIGURATION;

        //Open configuration
        Status = NdisOpenConfigurationEx(&ConfigObject, &ConfigurationHandle);
#else
        NdisOpenConfiguration(&Status,
            &ConfigurationHandle,
            NdisHandle);
#endif
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_INFO,("NdisOpenConfiguration failed\n"));
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        NdisReadNetworkAddress(
            &Status, 
            &pNetAddrBuf, 
            &Length, 
            ConfigurationHandle);
        if(Status == NDIS_STATUS_SUCCESS)
        {
            PUCHAR pMacAddr = (PUCHAR)pNetAddrBuf;
            //02-50-F2-00-00-01
            if (pMacAddr[0] == 0x02 && pMacAddr[1] == 0x50
                && pMacAddr[2] == 0xF2 && pMacAddr[3] == 0x00
                && pMacAddr[4] == 0x00 && pMacAddr[5] == 0x01)
            {
                NDIS_CONFIGURATION_PARAMETER NewPhysicalAddress;
                LARGE_INTEGER TickCountValue;
                UCHAR CurrentMacIndex = 3;
                WCHAR szTemp[20];
                NDIS_STRING strValue;
                NDIS_STRING strKey = RTL_CONSTANT_STRING(L"NetworkAddress");

                Adapter->CurrentAddress[0] = 0x02;
                Adapter->CurrentAddress[1] = 0x50;
                Adapter->CurrentAddress[2] = 0xF2;

                //
                // Generated value based on the current tick count value. 
                //
                KeQueryTickCount(&TickCountValue);
                do
                {
                    //
                    // Pick up the value in groups of 8 bits to populate the rest of the MAC address.
                    //
                    Adapter->CurrentAddress[CurrentMacIndex] = (UCHAR)(TickCountValue.LowPart>>((CurrentMacIndex-3)*8));
                } while(++CurrentMacIndex < ETH_LENGTH_OF_ADDRESS);

                //
                // Finally, we should make a best-effort attempt to save this address
                // to our configuration, so the NIC will always come up with this
                // permanent address.
                //

                RtlStringCchPrintfW(szTemp, 20, L"%02X-%02X-%02X-%02X-%02X-%02X",
                    Adapter->CurrentAddress[0],
                    Adapter->CurrentAddress[1],
                    Adapter->CurrentAddress[2],
                    Adapter->CurrentAddress[3],
                    Adapter->CurrentAddress[4],
                    Adapter->CurrentAddress[5]);
                RtlInitUnicodeString(&strValue, szTemp);

                NewPhysicalAddress.ParameterType = NdisParameterString;
                NewPhysicalAddress.ParameterData.StringData = strValue;

                NdisWriteConfiguration(
                        &Status,
                        ConfigurationHandle,
                        &strKey,
                        &NewPhysicalAddress);
                if (NDIS_STATUS_SUCCESS != Status)
                {
                    DEBUGP(MP_WARNING, ("NdisWriteConfiguration failed to save the permanent MAC address"));
                    // No other handling -- this isn't a fatal error
                }
            }
            else
            {
                int i = 0;
                for(; i < ETH_LENGTH_OF_ADDRESS; i++)
                {
                    Adapter->CurrentAddress[i] = ((PUCHAR)pNetAddrBuf)[i];
                }
            }
        }
        else
        {
            Status = NDIS_STATUS_FAILURE;
        }
    } while (FALSE);
	
    Adapter->fDump = 0;

    HwtInfoCtor(&Adapter->Infos);

    do
    {   
		Status = SoraAcquireRadioManager(&Adapter->SoraRadioManager);
		if (Status != STATUS_SUCCESS)
			break;

		Adapter->SoraRegs = Adapter->SoraRadioManager->__RegsBase;
		Adapter->TransferObj.TransferResMgr = &Adapter->SoraRadioManager->__TX_RESMGR;
		KeInitializeSpinLock(&Adapter->TransferObj.TransferLock);
		Adapter->TransferObj.TransferReg = Adapter->SoraRadioManager->__RegsBase->RadioRegs;
		Adapter->TransferObj.SoraSysReg = Adapter->SoraRadioManager->__RegsBase;
		Adapter->TransferObj.SoraHWOpRadio = &Adapter->SoraRadioManager->__radio_pool[0];

		{
			
			PHYSICAL_ADDRESS	PhysicalAddress 	 = {0, 0}; 
			PHYSICAL_ADDRESS	PhysicalAddressLow	 = {0, 0};
			PHYSICAL_ADDRESS	PhysicalAddressHigh  = {0x80000000, 0};
			HRESULT hr = E_NOT_ENOUGH_CONTINUOUS_PHY_MEM;
			PTRANSFER_OBJ pTransferObj = &Adapter->TransferObj;
			ULONG SampleBufferSize = MODULATE_BUFFER_SIZE;
			
			ALIGN_WITH_RCB_BUFFER_PADDING_ZERO(NULL, SampleBufferSize);

			pTransferObj->__TxBufLock = 0;

			if (!pTransferObj->__TxSampleBufVa)
			{
				pTransferObj->__TxSampleBufVa = 
						(PCOMPLEX8) MmAllocateContiguousMemorySpecifyCache(
									SampleBufferSize, 
									PhysicalAddressLow,
									PhysicalAddressHigh,
									PhysicalAddress,
									MmWriteCombined);
				if (pTransferObj->__TxSampleBufVa)
				{
					pTransferObj->__TxSampleBufPa	 = MmGetPhysicalAddress(pTransferObj->__TxSampleBufVa);
					ASSERT(pTransferObj->__TxSampleBufPa.u.LowPart % 0x00001000 == 0); //should always 4K aligned
					pTransferObj->__TxSampleBufSize  = SampleBufferSize;
				}
				else {
					Status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}
			}
		}

#if NDIS_SUPPORT_NDIS6 
		{
			NET_BUFFER_LIST_POOL_PARAMETERS param;
			param.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
			param.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
			param.Header.Size = NDIS_SIZEOF_NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
			param.ProtocolId = NDIS_PROTOCOL_ID_DEFAULT;
			param.fAllocateNetBuffer = TRUE;
			param.ContextSize = 0;
			param.PoolTag = 'tkpi';
			param.DataSize = 0;
			Adapter->AllocateNetBufferListPool = NdisAllocateNetBufferListPool(Adapter->AdapterHandle, 
                                                                               &param);
			if (!Adapter->AllocateNetBufferListPool) {
				Status = NDIS_STATUS_FAILURE;
				break;
			}
		}
#else
		NdisAllocatePacketPool(&Status, 
                               &Adapter->AllocatePacketPool,
                               NIC_RECV_POOL_SIZE,
                               PROTOCOL_RESERVED_SIZE_IN_PACKET);
		FAILED_BREAK(Status);
		
		NdisAllocateBufferPool(&Status, 
                               &Adapter->AllocateBufferPool, 
                               NIC_RECV_POOL_SIZE);
		FAILED_BREAK(Status);
#endif	

		Adapter->Thread = SoraThreadAlloc();
		if (!Adapter->Thread) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		{	// allocate one physical page for transfer...
			PHYSICAL_ADDRESS low;
			PHYSICAL_ADDRESS high;
			PHYSICAL_ADDRESS mul;
			low.QuadPart = 0;
			high.u.LowPart = 0x80000000;
			high.u.HighPart = 0;
			mul.QuadPart = 0;

			Adapter->WrapperPageVAddr = MmAllocateContiguousMemorySpecifyCache(PAGE_SIZE,
				low, 
				high,
				mul, 
				MmWriteCombined);

			if (!Adapter->WrapperPageVAddr) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			Adapter->WrapperPagePAddr = MmGetPhysicalAddress(Adapter->WrapperPageVAddr);

			Adapter->WrapperPageLock = 0;
		}
    }while (FALSE);
    
    if (FAILED(Status))
    {
		if (Adapter->WrapperPageVAddr) {
			MmFreeContiguousMemory(Adapter->WrapperPageVAddr);
			Adapter->WrapperPageVAddr = NULL;
		}

		if (Adapter->Thread) {
			SoraThreadFree(Adapter->Thread);
			Adapter->Thread = NULL;
		}		

#if NDIS_SUPPORT_NDIS6
		if (Adapter->AllocateNetBufferListPool) {
			NdisFreeNetBufferListPool(Adapter->AllocateNetBufferListPool);
			Adapter->AllocateNetBufferListPool = NULL;
		}
#else
		if (Adapter->AllocatePacketPool) {
			NdisFreePacketPool(Adapter->AllocatePacketPool);
			Adapter->AllocatePacketPool = NULL;
		}
		if (Adapter->AllocateBufferPool) {
			NdisFreeBufferPool(Adapter->AllocateBufferPool);
			Adapter->AllocateBufferPool = NULL;
		}
#endif

		if (Adapter->TransferObj.__TxSampleBufVa) {
			MmFreeContiguousMemorySpecifyCache(
				Adapter->TransferObj.__TxSampleBufVa, 				
				Adapter->TransferObj.__TxSampleBufSize,				
				MmWriteCombined);			
			Adapter->TransferObj.__TxSampleBufVa    = NULL;			
			Adapter->TransferObj.__TxSampleBufSize  = 0;		
		}

		if (Adapter->SoraRadioManager) {
			SoraReleaseRadioManager();
			Adapter->SoraRadioManager = NULL;
		}
    }
	else {
			SoraKUExtKeCtor(&Adapter->UExtKeObj);
			SoraKUExtKeInit(&Adapter->UExtKeObj);			
	}
    
    DEBUGP(MP_TRACE, ("[HT_STARTUP]<--- InitializeAdapter:%08x\n", Status));
    return Status;
}

