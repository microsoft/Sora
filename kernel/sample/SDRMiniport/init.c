/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: 
   Init.C

Abstracts:
    This file defines functions to initialize NIC Adapter structure.

Revision History:
    Created by senxiang, 7/Apr/2009

Notes: 

--*/


#include "miniport.h"
#include "init.h"
#include "trace.h"
#ifdef EVENT_TRACING
#include "init.tmh"

#endif
#include <ntstrsafe.h>
#include "bb/DataRate.h"

#pragma NDIS_PAGEABLE_FUNCTION(NICAllocAdapter)
#pragma NDIS_PAGEABLE_FUNCTION(NICInitializeAdapter)
#pragma NDIS_PAGEABLE_FUNCTION(NICAdapterCtor)
#pragma NDIS_PAGEABLE_FUNCTION(ReadTxParameters)
#pragma NDIS_PAGEABLE_FUNCTION(NICInitializeSdrContext)
#pragma NDIS_PAGEABLE_FUNCTION(NICRequestRadiosThreadFunc)
#pragma NDIS_PAGEABLE_FUNCTION(NICReadRegistryConfiguration)

#define MODULATE_BUFFER_SIZE        (2 * 1024 * 1024)

/* 
NICAdapterCtor is constructor of MP_ADAPTER.

Parameters:
    pAdapter: Pointer to Adapter structure

Return:    

History:

IRQL: PASSIVE_LEVEL
*/
VOID 
NICAdapterCtor(IN PMP_ADAPTER pAdapter)
{
    PAGED_CODE();

    pAdapter->AdapterHandle                         = NULL;

    pAdapter->RecvPacketOrNetBufferListPoolHandle   = NULL;
    pAdapter->Fdo                                   = NULL;
    pAdapter->Pdo                                   = NULL;
    pAdapter->NextDeviceObject                      = NULL;
    pAdapter->RefCount                              = 0;
    pAdapter->ullGoodReceives                       = 0;
    pAdapter->ullDuplicatedReceives                 = 0;
    pAdapter->ullGoodTransmits                      = 0;
    pAdapter->Mac.bMacInitOK                        = FALSE;
    pAdapter->Phy.bRadiosInitOK                     = FALSE;

    pAdapter->ullGoodTransmits                      = 0;
    pAdapter->ullGoodReceives                       = 0;
    pAdapter->ullReceiveErrors                      = 0;
    pAdapter->ullReceiveNoBuffers                   = 0;
    pAdapter->ullTransmitFail                       = 0;

    RtlZeroMemory(pAdapter->TxParameters, sizeof(pAdapter->TxParameters));

    RtlZeroMemory(pAdapter->CurrentAddress, ETH_LENGTH_OF_ADDRESS);

    RtlZeroMemory(&pAdapter->List, sizeof(LIST_ENTRY));
    RtlZeroMemory(&pAdapter->PHYInitThread, sizeof(SORA_THREAD));

    SoraKUExtKeCtor(&pAdapter->KeAppExtObj);
}

/*++

NICRequestRadiosThreadFunc is the thread entry for radio request.

Arguments:
    pContext - thread context pointer.

Return Value: 

History:
    Modified by senxiang, 8/Dec/2009

Note:
    radio request continues only when radio is unvailable.

IRQL: PASSIVE_LEVEL
--*/
VOID NICRequestRadiosThreadFunc(IN PVOID pContext)
{
    PMP_ADAPTER Adapter      = SORA_THREAD_CONTEXT_PTR(MP_ADAPTER, pContext);
    HRESULT     hr           = E_FAIL;
    LARGE_INTEGER  Delay;
    Delay.QuadPart           = MP_RADIOS_REQ_RETRY_INTERVAL;

    PAGED_CODE();
    
    MP_INC_REF(Adapter); 

    DEBUGP(MP_TRACE, ("[STARTUP]--> NICRequestRadiosThreadFunc \n"));
    do
    {
    
        if(!NIC_IS_PHY_READY(Adapter))
        {
            hr = SdrPhyInitialize(
                    &(Adapter->Phy),
                    &Adapter->SdrContext,
                    Adapter->TransferObj,
                    REQUIRED_RADIO_NUM);
            if (SUCCEEDED(hr))
            { 
                DEBUGP(MP_TRACE, 
                    ("[STARTUP] PHY Startup asynchronously, get %d free radio(s) \n", REQUIRED_RADIO_NUM));
                NIC_SET_PHY_READY(Adapter);
                break;
            }
            else
            {
                if (hr != E_NOT_ENOUGH_FREE_RADIOS
                    && hr!= E_DEVICE_NOT_FOUND)
                    break; // for other reason, we don't need request radio again
            }    
        }
        KeDelayExecutionThread(KernelMode, FALSE, &Delay);
    } while(!IS_SORA_THREAD_NEED_TERMINATE(pContext));

    MP_DEC_REF(Adapter);
    
    SORA_THREAD_STOPPED(pContext);
    DEBUGP(MP_TRACE, ("[EXIT] NICRequestRadioThread Terminated\n"));
    PsTerminateSystemThread(STATUS_SUCCESS);
    return;
}

/*

NICInitializeSdrContext initializes all SDR objects (LL, MAC, PHY).

Parameters: 
    Adapter - pointer to adapter object

Return: 
    NDIS_STATUS_xxx

History:   
    Created by yichen: 1/Apr/2009 
    Modified by senxiang: 5/Dec/2009

IRQL: PASSIVE_LEVEL
*/
NDIS_STATUS NICInitializeSdrContext(IN PMP_ADAPTER Adapter)
{    
    HRESULT             hrPhy = E_FAIL;
    HRESULT             hrMac = E_FAIL;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;

    PAGED_CODE();

    DEBUGP(MP_INFO, ("[STARTUP] --> NICInitializeSdrContext \n"));
    do
    {
	    Status = SoraKAllocateTransferObj(&Adapter->TransferObj);
		if (Status != STATUS_SUCCESS)
			break;

        InitializeListHead(&(Adapter->Phy.RadiosHead));
        NIC_SET_PHY_NOT_READY(Adapter);
        NIC_SET_MAC_NOT_READY(Adapter);

        INIT_SORA_THREAD(
            Adapter->PHYInitThread,
            NICRequestRadiosThreadFunc,
            Adapter,
            0, 
            CORE_ANY);

        hrPhy = SdrPhyInitialize(&(Adapter->Phy),
            &Adapter->SdrContext,
            Adapter->TransferObj,
            REQUIRED_RADIO_NUM
            );
        if (SUCCEEDED(hrPhy))
        {
            DEBUGP(MP_INFO, 
                ("[STARTUP] PHY Startup, get %d free radio(s) \n", REQUIRED_RADIO_NUM));
            NIC_SET_PHY_READY(Adapter);
        }
        else
        {
            if (hrPhy == E_NOT_ENOUGH_FREE_RADIOS
                || hrPhy == E_DEVICE_NOT_FOUND)
            {
                DEBUGP(MP_WARNING, 
                    ("[STARTUP] PHY Startup fail, no %d free radio(s) \n", REQUIRED_RADIO_NUM));
				break;
            }
            else
            {
                DEBUGP(MP_ERROR, 
                    ("[STARTUP] PHY Startup fail due to out of memory or hardware error: 0x%08x\n", 
                    hrPhy));
                break;
            }
        }

        SdrLLInitialize(&Adapter->Lnk);

        hrMac = SdrMacInitialize(&(Adapter->Mac), &Adapter->SdrContext);
        if(SUCCEEDED(hrMac))
        {
            DEBUGP(MP_INFO, ("[STARTUP] 802.11 MAC startup succ\n"));
            NIC_SET_MAC_READY(Adapter);
        }
        else
        {
            DEBUGP(MP_ERROR, ("[STARTUP] 802.11 MAC startup fails: 0x%08x\n", hrMac));
            break;
        }

        //If we can't initialize Phy because no enough radios,we start a thread to initialize it repeatly
        if (hrPhy == E_NOT_ENOUGH_FREE_RADIOS
            || hrPhy == E_DEVICE_NOT_FOUND)
        {
            DEBUGP(MP_TRACE, ("[STARTUP] Try start PHY init thread \n"));
            START_SORA_THREAD(Adapter->PHYInitThread);
            return Status;
        }

    }while(FALSE);

    if(FAILED(hrPhy) || FAILED(hrMac))
    {
        if(SUCCEEDED(hrPhy))
            SdrPhyCleanUp(&(Adapter->Phy));

        if(SUCCEEDED(hrMac))
            SdrMacCleanUp(&(Adapter->Mac));

        Status = NDIS_STATUS_FAILURE;
    }

    if (SUCCEEDED(Status)) 
		SoraKUExtKeInit(&Adapter->KeAppExtObj);
	else  {

		if (Adapter->TransferObj) {
			SoraKFreeTransferObj(Adapter->TransferObj);
			Adapter->TransferObj = NULL;
		}
	}
    return Status;
}

UINT
Get11ADataRate(PUNICODE_STRING pValue)
{
	ULONG n11ADataRate = 0;
	RtlUnicodeStringToInteger(pValue, 0, &n11ADataRate);
    DbgPrint("[INFO] registry 11ADataRate=%d\n", n11ADataRate);
	
    if (Dot11ARate_KbpsValid(n11ADataRate * 1000))
    {
        return Dot11ADataRate_Kbps2Code(n11ADataRate * 1000);
    }
    else
    {
        DbgPrint("[ERROR] Not valid dot11a rate in Mbps, use 6M default\n");
        return DOT11A_RATE_6M;
    }
}

/*

ReadTxParameters reads Tx Parameters from registry, it reads 4 values for Tx, 
which are DataRate,DataRateInMbps,ModSelect,PreambleType.
 
Parameters:
    Adapter                     : pointer to Adapter object
    ConfigurationHandle         : registry handle
 
Return Value:

IRQL: PASSIVE_LEVEL
*/
VOID ReadTxParameters(IN PMP_ADAPTER pAdapter,
                      IN NDIS_HANDLE ConfigurationHandle)
{
    PNDIS_CONFIGURATION_PARAMETER pParameterValue;
    ULONG               TxValue;
    UNICODE_STRING      TxParameters;
    ULONG               Count = 0;
    NDIS_STATUS         Status;

    PWSTR pTxParameterNames[TX_PARAMETERS_CT] = {L"DataRate",
                                  L"ModSelect", L"PreambleType", L"SampleRate", L"11ADataRate"};

    PAGED_CODE();

    //Read TX related parameters
    for(Count = 0; Count < TX_PARAMETERS_CT; Count++)
    {
        RtlInitUnicodeString(&TxParameters, pTxParameterNames[Count]);
        NdisReadConfiguration(
            &Status,
            &pParameterValue,
            ConfigurationHandle,
            &TxParameters,
            NdisParameterString );
        if(Status == NDIS_STATUS_SUCCESS)
        {
			if(Count == TX_PARAMETERS_CT - 1)
			{
				pAdapter->Phy.BBContextFor11A.TxVector.ti_uiDataRate = Get11ADataRate(
					&(pParameterValue->ParameterData.StringData));
			}
			else
			{
                RtlUnicodeStringToInteger(&pParameterValue->ParameterData.StringData,
                    0, &TxValue);
                pAdapter->TxParameters[Count] = TxValue;
                KdPrint(("[TxParameter] %S:%02X",pTxParameterNames[Count], pAdapter->TxParameters[Count]));
			}
        }
    }
}

/*
ReadPHYModeReg
*/
PHY_MODE
ReadPHYModeReg(IN NDIS_HANDLE ConfigurationHandle)
{
	int i = DOT_11_A;
    NDIS_STATUS Status;
    PNDIS_CONFIGURATION_PARAMETER pParameterValue;
    UNICODE_STRING ustrModMode;
    PWSTR pModModes[MOD_MODE_CT] = {L"802.11a", L"802.11b"};

    PAGED_CODE();

    RtlInitUnicodeString(&ustrModMode, L"ModMode");
    NdisReadConfiguration(
        &Status,
        &pParameterValue,
        ConfigurationHandle,
        &ustrModMode,
        NdisParameterString);

	if(Status == NDIS_STATUS_SUCCESS)
	{
		for(i = DOT_11_A; i < MOD_MODE_CT; i++)
		{
			RtlInitUnicodeString(&ustrModMode, pModModes[i]);
			if(RtlEqualUnicodeString(&pParameterValue->ParameterData.StringData, &ustrModMode, TRUE))
			{
				return (PHY_MODE)i;
			}
		}
	}
	
	return MOD_MODE_ERROR;
}

/*
ReadBSSID
*/
VOID
ReadBSSID(IN NDIS_HANDLE ConfigurationHandle)
{
    int i = 0;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PNDIS_CONFIGURATION_PARAMETER pParameterValue;
    UNICODE_STRING      strBssid;
    PWSTR               pstrBssID;
    ULONG               ulValue = 0;

    //Read BSSID string
    RtlInitUnicodeString(&strBssid, L"BSSID");
    NdisReadConfiguration(
        &Status,
        &pParameterValue,
        ConfigurationHandle,
        &strBssid,
        NdisParameterString );
    if(Status != NDIS_STATUS_SUCCESS)
    {
        return;
    }
    pstrBssID = pParameterValue->ParameterData.StringData.Buffer;

    //Parse BSSID string
    for(i = 0; i < MAC_ADDRESS_LENGTH; i++)
    {
        RtlInitUnicodeString(&strBssid, pstrBssID + i * REGISTRY_ADDRESS_NUM_OFFSET);
        RtlUnicodeStringToInteger(&strBssid, 16, &ulValue);
        g_BSSID.Address[i] = (UCHAR)ulValue;
    }
}

/*
NICReadRegistryConfigrutation reads configurations for the driver, 
including MAC address, PHY parameters, etc.

Parameters:
    Adapter                     : pointer to Adapter object
    NdisHandle                  : handle from ndis
 
Return Value: 
    STATUS_SUCCESS for success otherwise fail

History:   1/Apr/2009 Created by yichen

IRQL: PASSIVE_LEVEL
*/
NDIS_STATUS NICReadRegistryConfiguration(
    IN PMP_ADAPTER Adapter,
    IN NDIS_HANDLE NdisHandle)
{
    NDIS_HANDLE         ConfigurationHandle;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    
    UINT                Length = 0;
    PVOID               pNetAddrBuf = NULL;

#ifdef NDIS620_MINIPORT
    NDIS_CONFIGURATION_OBJECT  ConfigObject;
#endif

    PAGED_CODE();

    do{
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

        //Read BSSID ,BSSID must has the format "XX XX XX XX XX XX"
        ReadBSSID(ConfigurationHandle);

		//Read modulation mode
		Adapter->Phy.PHYMode = ReadPHYModeReg(ConfigurationHandle);

        //Read Tx Parameters
        ReadTxParameters(Adapter,ConfigurationHandle);
                
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

        NdisCloseConfiguration(ConfigurationHandle);
    }
    while(FALSE);

    return Status;
}


/* 
NICAllocAdapter allocates memory for MP_ADAPTER object, 
and calls its constructor.

Parameters:
    pAdapter:   Pointer to pointer to Adapter structure
    NdisHandle: Handle from ndis

Return:    

History:   
    Modified by senxiang, 8/Dec/2009

IRQL: PASSIVE_LEVEL
*/

NDIS_STATUS
NICAllocAdapter(
    IN PMP_ADAPTER *ppAdapter)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER Adapter = NULL;

    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, DBG_INIT, "--> NICAllocAdapter \n");
    do 
    {
        MP_ALLOCATE_MEMORY((PVOID&)Adapter, sizeof(MP_ADAPTER), Status);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR,
                ("[STARTUP] insufficient non-paged memory for adapter context with size 0x%08x\n",
                sizeof(MP_ADAPTER)));
            break;
        }
        else
        {
            DEBUGP(MP_INFO,
                ("[STARTUP] 0x%08x-byte non-paged memory for adapter context allocated\n",
                sizeof(MP_ADAPTER)));
        }
        NdisZeroMemory(Adapter, sizeof(MP_ADAPTER));
        NICAdapterCtor(Adapter);
        g_pGlobalAdapter = Adapter;
    }
    while(FALSE);

    *ppAdapter = Adapter;

#ifdef NDIS620_MINIPORT
    if(*ppAdapter)
    {
        (*ppAdapter)->PortNumber = 0;
    }
#endif

    return Status;
}


/*

NICInitializeAdapter initializes members of Adapter object,
reads configuration from registery and initializes objects in 
SDRContext (LL, MAC, PHY).
 
Arguments:
    Adapter           : pointer to Adapter object
    NdisHandle        : handle from ndis
 
Return Value:
    NTSTATUS    - if the status value is not STATUS_SUCCESS,
    the driver will get unloaded

History:
    Modified by senxiang, 5/Dec/2009

IRQL: PASSIVE_LEVEL
*/

NDIS_STATUS NICInitializeAdapter(IN PMP_ADAPTER Adapter, IN NDIS_HANDLE NdisHandle)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    PAGED_CODE();

    do
    {
        //bind SdrContext 
        SdrContextBind(&Adapter->SdrContext, Adapter, &Adapter->Lnk, &Adapter->Mac, &Adapter->Phy);

        NdisInitializeListHead(&Adapter->List);
        NdisInitializeEvent(&Adapter->RemoveEvent);
        //KeInitializeSpinLock(&Adapter->AdapterSpinLock);

        //Allocate packet pool
        Status = MPAllocatePacketPool(Adapter);
        if(NDIS_STATUS_SUCCESS != Status)
        {
            break;
        }

        //Get device property
        NdisMGetDeviceProperty(Adapter->AdapterHandle,
                           &Adapter->Pdo,
                           &Adapter->Fdo,
                           &Adapter->NextDeviceObject,
                           NULL,
                           NULL);

        //Read registry
        Status = NICReadRegistryConfiguration(Adapter, NdisHandle);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("[STARTUP] NICReadRegistryConfiguration failed \n"));
            break;
        }

        Status = NICInitializeSdrContext(Adapter);
    }
    while(FALSE);

    return Status;
}


/*
FreeAdapter free ndis packet pool that used to indicate receiving data to up layer
and free memory occupied by adapter

Parameters:
    pAdapter : pointer to adapter object.
Return:  

Note: 

IRQL: PASSIVE_LEVEL
*/
VOID FreeAdapter(IN PMP_ADAPTER pAdapter)
{
    if(pAdapter->RecvPacketOrNetBufferListPoolHandle)
    {
        MPFreePacketOrNBLPool(pAdapter->RecvPacketOrNetBufferListPoolHandle);
        pAdapter->RecvPacketOrNetBufferListPoolHandle = NULL;
    }
	PSORA_RADIO pRadio = RadioInPHY(&pAdapter->Phy, RADIO_RECEIVE);
	SoraKUExtKeDtor(&pAdapter->KeAppExtObj, pAdapter->TransferObj);

	if (pAdapter->TransferObj) {
		SoraKFreeTransferObj(pAdapter->TransferObj);
		pAdapter->TransferObj = NULL;
	}

	NdisFreeMemory(pAdapter, sizeof(MP_ADAPTER), 0);
}


