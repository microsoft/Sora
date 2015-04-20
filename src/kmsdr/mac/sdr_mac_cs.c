/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_mac_cs.c

Abstract: This file implements 802.11 carrier sense (CS) state handler, which calls a 
simple carrier sense algorithm in base band (BB) to detect energy. And set the next 
state for the state machine.

History: 
    3/25/2009: Created by senxiang
--*/

#include "miniport.h"

#define HW_ERROR_THRESHHOLD     10

void _Dump(PMAC pMac, PSORA_RADIO pRadio)
{
    ULONG   byteDump          = 0;
    HANDLE  hFile;
	WCHAR* file = L"\\??\\C:\\SdrDump";
    if(pMac->pDumpBuffer) //move it to mac 
    {
        SoraDumpNewRxBuffer(pRadio,pMac->pDumpBuffer,RX_BUFFER_SIZE,
                            &byteDump);
        DbgPrint("[dump] Dump %08X bytes to file \n",byteDump);
    }
    else
        DbgPrint("[dump] dump buffer empty\n");

	hFile = __CreateDumpFile(file);
	if (!hFile)
		hFile = __CreateDumpFile(L"\\Device\\HarddiskVolume1\\SdrDump");
	
    if(hFile != NULL)
    {
        __DumpBuffer2File(hFile,pMac->pDumpBuffer,byteDump);
        __CloseDumpFile(hFile);
    }
    pMac->fDumpMode = 0;
}

//expect an ACK is in the RX stream.
__inline 
HRESULT __ExpectAck(PPHY pPhy)
{
    int i = 6;
    HRESULT hRes;
    KIRQL OldIrql =  KeRaiseIrqlToDpcLevel();    
    do
    {
        hRes = PhyDot11BCs(pPhy, RADIO_SEND);
        i--;
    }while(hRes != BB11B_OK_POWER_DETECTED && i > 0 );
    KeLowerIrql(OldIrql);
    return hRes;
}

//If Hardware is in uncoverable state, driver should reset it.
//for heavy hardware reset, packet symbols already transferred now become invalid.
void SdrMacResetSend(PMAC pMac, PPHY pPhy)
{
    HRESULT hr;
    PSEND_QUEUE_MANAGER pSendQueueManager    = GET_SEND_QUEUE_MANAGER(pMac);
    KIRQL oldIrql;
    BOOLEAN fRxRestore; 
    PSORA_RADIO pRadio = RadioInPHY(pPhy, RADIO_RECEIVE);

    if(SoraRadioCheckRxState(pRadio))
    {
        SORA_HW_STOP_RX(pRadio);
        fRxRestore = TRUE;
    }
    
    STOP_SORA_THREAD(pMac->SendThread);
    STOP_SORA_THREAD(pMac->ReceiveThread);
    
    hr = SoraHwHeavyRestart(pRadio);
    if (FAILED(hr))
        DbgPrint("[Error] Heavy Reset failed with %08x\n", hr);
    
    CleanQueueWithTxResources(pSendQueueManager);

    SoraCleanSignalCache(&pPhy->AckCacheMgr.AckCache);
    SoraInitSignalCache(		
        &pPhy->AckCacheMgr.AckCache, 
        pPhy->TransferObj,
        RadioInPHY(pPhy, RADIO_SEND), 
        MAX_PHY_ACK_SIZE, 
        MAX_PHY_ACK_NUM);
    
    if (fRxRestore)
    {
        SORA_HW_ENABLE_RX(pRadio);
    }

    START_SORA_THREAD(pMac->SendThread);
    START_SORA_THREAD(pMac->ReceiveThread);
}

/*
MAC State hander: SdrMacCs
	In carrier sensing state, MAC calls PHY functions to compute the average
	energy of incoming samples.

	PHY will sense the channel for a window of time and turn the status.
	If the channel is busy, go to RX state; 
	Otherwise, go to TX state if there are any pending packets in queue.

Parameters:
    StateMachine    : pointer to THIS state machine
        
Return:  
    None

Note: 

IRQL: PASSIVE_LEVEL
*/
VOID
SdrMacCs(IN PFSM_BASE StateMachine)
{
    HRESULT hRes                = S_OK;
    
    //Get SDR context initialized by SORA_FSM_CONFIG;
    PSDR_CONTEXT pSDRContext    = (PSDR_CONTEXT)SoraFSMGetContext(StateMachine); 
    // Get all references from SDR context
    PMP_ADAPTER pAdapter        = (PMP_ADAPTER)pSDRContext->Nic; //initialized by SdrContextBind;
    PMAC pMac                   = (PMAC)pSDRContext->Mac; //initialized by SdrContextBind;
    PPHY pPhy                   = (PPHY)pSDRContext->Phy; //initialized by SdrContextBind;
    PSORA_RADIO pRadio          = NULL;

    if(!NIC_IS_DEVICE_CAN_WORK(pAdapter))
    {
        LARGE_INTEGER Delay;
        Delay.QuadPart = -10 * 1000 * 10; //10ms
        KeDelayExecutionThread(KernelMode, FALSE, &Delay); //device can't work, so sleep. 
        return; //state not changed.
    }
    pRadio    =  RadioInPHY(pPhy, RADIO_RECEIVE);
    if(!SoraRadioCheckRxState(pRadio))
    {
        DbgPrint("[MAC_CS] enable Rx for the first time\n");
        SORA_HW_ENABLE_RX(pRadio);
    }
    if (pPhy->HwErrorNum > HW_ERROR_THRESHHOLD)
    {
        DbgPrint("[Error] Reset MAC send \n");
        InterlockedExchange(&pPhy->HwErrorNum, 0);
        //SdrMacResetSend(pMac);
    }
    if(pMac->fDumpMode) 
    {
        _Dump(pMac, pRadio);
    }
    hRes = (*pPhy->FnPHY_Cs)(pPhy, RADIO_SEND);
    if (hRes == E_FETCH_SIGNAL_HW_TIMEOUT)
    {
        DbgPrint("[MAC_CS][Error] E_FETCH_SIGNAL_HW_TIMEOUT \n");
    }

    switch (hRes)
    {
    case BB11A_CHANNEL_CLEAN:
	case BB11B_CHANNEL_CLEAN:
        if (IS_MAC_EXPECT_ACK(pMac))
        {
            hRes = __ExpectAck(pPhy);
            if (hRes != BB11B_OK_POWER_DETECTED)
            {
                DbgPrint("[MAC_CS][Error]  Ack detect fail, we don't need ACK anymore \n");
                MAC_DISLIKE_ACK(pMac);
            }
            else
            {
                SoraFSMGotoState(StateMachine, Dot11_MAC_RX);
                return;
            }
        }

        DbgPrint("[MAC_CS] channel clean, goto tx \n");
        SoraFSMGotoState(StateMachine, Dot11_MAC_TX);
        return;
    case BB11A_OK_POWER_DETECTED:
	case BB11B_OK_POWER_DETECTED:
		DbgPrint("[MAC_CS] channel busy, goto rx \n");
        SoraFSMGotoState(StateMachine, Dot11_MAC_RX);
        return;
    case E_FETCH_SIGNAL_HW_TIMEOUT: //Hardware error
        SoraFSMGotoState(StateMachine, Dot11_MAC_TX);
        DbgPrint("[Error] E_FETCH_SIGNAL_HW_TIMEOUT \n");
        //InterlockedIncrement(&pMac->pPhy->HwErrorNum);
        break;
    default: 
        DbgPrint("[MAC_CS] CS return %x\n", hRes);
        break;
    }

}


