/*++
Copyright (c) Microsoft Corporation

Module Name: RX process for Mac

Abstract: This file implements interface functions to process receiving packets

History: 
          3/25/2009: Created by yichen
--*/

#include "miniport.h"

__inline void __CopyMACAddr2Key ( PCACHE_KEY pKey, 
                                  PMAC_ADDRESS pMacAddr)
{
    int i;
    pKey->QuadKey.u.HighPart = 0;
    for (i = 0; i < MAC_ADDRESS_LENGTH; i++)
    {
        pKey->KeyBytes[i] = pMacAddr->Address[i];
    }
    
}

__inline 
void 
__Ack(
    PACK_CACHE_MAN pCacheMan, 
    PSORA_RADIO pRadio, 
    MAC_ADDRESS Dest
    )
{
    HRESULT hr;
    CACHE_KEY Key;
    PTX_DESC pAckDesc;
    __CopyMACAddr2Key(&Key, &Dest);

    pAckDesc = SoraGetSignal(&pCacheMan->AckCache, Key);
    if (pAckDesc != NULL)
    {
        hr = SORA_HW_FAST_TX(pRadio, pAckDesc);
        if (FAILED(hr))
        {
            DbgPrint("[Error] ACK fast TX FAIL \n"); //hardware error.
        }
        else
        {
            DbgPrint("[RX] Fast ACK success \n");
        }
    }
    else
    {
        SdrPhyAsyncMakeAck(pCacheMan, &Dest);
    }
}

//0.1Mbps
static const ULONG RateMap[16] =  
    { 0, 10, 20, 55, 110, 0, 0, 0,          //11b
      480, 240, 120, 60, 540, 360, 180, 90  //11a
    };

__forceinline
void 
LatencyCheck(
    LARGE_INTEGER SFDFindTimeStamp, 
    ULONG PacketLength, 
    char Rate)
{
    LARGE_INTEGER   Freq;    
    ULONG           DecodeTime;
    ULONG           SendTime; // micro-senconds
    
    LARGE_INTEGER End = KeQueryPerformanceCounter(&Freq);
    
    SendTime = (PacketLength * 80) / RateMap[Rate]; //RateMap with 0.1Mbps unit, so PacketLength * 80
	DecodeTime = (ULONG)((End.QuadPart - SFDFindTimeStamp.QuadPart) * 1000 * 1000 / Freq.QuadPart);

    DbgPrint("[RX] send time %d us\n", SendTime);
    DbgPrint("[RX] decode time %d us, End=%I64d, Freq=%I64d\n", DecodeTime, End.QuadPart, Freq.QuadPart);
    if (DecodeTime > ACK_MAX_TIMEOUT + SendTime)
        DbgPrint("[RX][Warning] Decode Time cost %d us, send time %d us\n", DecodeTime, SendTime);
    else
        if (DecodeTime < ACK_MIN_TIMEOUT + SendTime)
        {
            SoraSpinSleep(ACK_MIN_TIMEOUT + SendTime - DecodeTime, &End);
            DbgPrint("[RX][Warning] Decode fast, sleep for %d us \n", ACK_MIN_TIMEOUT + SendTime - DecodeTime);
        }
    return;
}

__forceinline
void 
LatencyMeasure(
    LARGE_INTEGER SFDFindTimeStamp, 
    ULONG PacketLength, 
    char Rate)
{
    LARGE_INTEGER Freq;    
    ULONG   DecodeTime;
    ULONG   SendTime; // micro-senconds
    
    LARGE_INTEGER End = KeQueryPerformanceCounter(&Freq);
    
    SendTime = (PacketLength * 80) / RateMap[Rate]; //RateMap with 0.1Mbps unit, so PacketLength * 80
	DecodeTime = (ULONG)((End.QuadPart - SFDFindTimeStamp.QuadPart) * 1000 * 1000 / Freq.QuadPart);

    DbgPrint("[RX] send time %d us\n", SendTime);
    DbgPrint("[RX] decode time %d us\n", DecodeTime);
    //if (DecodeTime > ACK_MAX_TIMEOUT + SendTime)
        DbgPrint("[RX][Warning] Decode Time cost %d us, send time %d us\n", DecodeTime, SendTime);
    return;
}


__inline 
void 
__DataFrameACK(
    PMP_ADAPTER pAdapter, 
    PSORA_RADIO pRadio, 
    PDOT11RFC1042ENCAP pWlanHeader, 
    ULONG PacketLength)
{
    if (memcmp(pAdapter->CurrentAddress, 
        pWlanHeader->MacHeader.Address1.Address, MAC_ADDRESS_LENGTH) == 0)
    {
        // Last 3 bits as index
        LatencyCheck(
            DemodStartTimeStampInPHY(&pAdapter->Phy), 
            PacketLength, 
            PrevDataRateInPHY(&pAdapter->Phy));
        __Ack(
            &pAdapter->Phy.AckCacheMgr, 
            pRadio, 
            pWlanHeader->MacHeader.Address2);
    }
    else
    {
        //LatencyMeasure(
        //    DemodStartTimeStampInPHY(&pAdapter->Phy), 
        //    PacketLength, 
        //    PrevDataRateInPHY(&pAdapter->Phy));
        DbgPrint("[RX] Destination is not ONLY me\n");
    }
}

__inline void 
__IndicateTXDoneAfterGetACK(
    PMP_ADAPTER pAdapter, 
    PSORA_RADIO pRadio, 
    PDOT11RFC1042ENCAP pWlanHeader)
{
    if (memcmp(pAdapter->CurrentAddress, 
               pWlanHeader->MacHeader.Address1.Address,
               MAC_ADDRESS_LENGTH) == 0
        )
    {
        PDLCB                pTCB                = NULL;
        PSEND_QUEUE_MANAGER pSendQueueManager   = GET_SEND_QUEUE_MANAGER(&pAdapter->Mac);
        
        //get oldest packet which is waiting for ack
        SafeDequeue(pSendQueueManager, SendSymWaitList, pTCB, DLCB); 
        ASSERT(pTCB);
        SoraPacketFreeTxResource(pAdapter->TransferObj, &pTCB->PacketBase); 
        //It is actually TX out, free occupied TX resources.
        SoraPacketSetTXDone(&pTCB->PacketBase);
        //Send OK
        pTCB->bSendOK = TRUE;
        SafeEnqueue(pSendQueueManager, SendCompleteList, pTCB);
        //MarkModulatedSlotAsTxDone(pSendQueueManager);//dequeue oldest modulated packet.
        InterlockedIncrement(&pSendQueueManager->nCompletePacket);
        InterlockedDecrement(&pSendQueueManager->nSymPacket);
        DbgPrint("[TX]Received an ACK for us, TX Done\n");

        pAdapter->Mac.fTxNeedACK = FALSE;  //we don't need ack, expect data frame.

        // Notify SDR MAC RECV thread to call LL to complete.
        SDR_MAC_INDICATE_PACKET_SENT_COMPLETE(&pAdapter->Mac);
    }
}

__inline void __SetupPacketMDL(PMDL pPacketMDL, PVOID pPacketBuffer, ULONG PacketLength)
{
    pPacketMDL->ByteCount      = PacketLength;
    pPacketMDL->ByteOffset     = 0;
    pPacketMDL->StartVa        = pPacketBuffer;
    pPacketMDL->MappedSystemVa = (PUCHAR)(pPacketMDL->StartVa);
}

__inline void __DataPacketIndicateUp(PMAC pMac, PULCB pRCB)
{
    __SetupPacketMDL(pRCB->pMdl,  pRCB->pVirtualAddress, pRCB->PacketLength);
    SafeEnqueue(&pMac->RecvQueueManager, RecvWaitList, pRCB);
    // Notify mini-port driver to indicate this packet up to NDIS
    DbgPrint("[RX]Indicate up \n");
    // Notify SDR MAC RECV thread to call LL to indicate packet up to protocal stack.
    SDR_MAC_INDICATE_RECV_PACKET(pMac);
}

void PrintMacBeacon(PDOT11_MAC_BEACON beacon)
{
    unsigned __int8 i = 0;
    DbgPrint("[RX] recv beacon BSSID=%02x-%02x-%02x-%02x-%02x-%02x\n", 
                        beacon->MacHeader.Address3.Address[0], 
                        beacon->MacHeader.Address3.Address[1], 
                        beacon->MacHeader.Address3.Address[2], 
                        beacon->MacHeader.Address3.Address[3],
                        beacon->MacHeader.Address3.Address[4], 
                        beacon->MacHeader.Address3.Address[5]);

    DbgPrint("[RX] beacon SSID=");
    for (; i < beacon->SSID.Length; i++)
    {
        DbgPrint("%c", beacon->SSID.SSID[i]);
    }
    DbgPrint("\n");
}

BSSID g_BSSID;

void ModifyBSSID(PDOT11_MAC_BEACON beacon)
{
    int i;
    if ((beacon->SSID.Length == 3) && 
        (beacon->SSID.SSID[0] == 's') &&
        (beacon->SSID.SSID[1] == 'd') &&
        (beacon->SSID.SSID[2] == 'r'))
    {
        for (i = 0; i < 6; i++)
            g_BSSID.Address[i] = beacon->MacHeader.Address3.Address[i];
    }
}
/*
SdrMacRx get receiving data from hardware,if the type of data received is FRAME_DATA, 
it send back an ack packet,and signal the receiving thread to handle the received data,
if type of data is FRAME_CTRL then it set the Corresponding TX packet to TxDone state 
and signal the receiving thread to release the conresponding Tx slot in send queue.

 Parameters:
            pContext    : pointer to DOT11CONTEXT object 
            pStateIndex : next state of the state machine 
        
 Return:  

 Note: 

 History:   4/1/2009 Created by yichen

 IRQL: DISPATCH_LEVEL
*/
VOID
SdrMacRx(IN PFSM_BASE StateMachine)
{
    KIRQL                   OldIrql;
    HRESULT                 hRes                = S_OK;
    PULCB                   pRCB                = NULL;
    PDOT11RFC1042ENCAP      pWlanHeader         = NULL;
    PSDR_CONTEXT            pSDRContext         = SoraFSMGetContext(StateMachine);
    PMP_ADAPTER             pAdapter            = (PMP_ADAPTER)pSDRContext->Nic ;
    PMAC                    pMac                = (PMAC)pSDRContext->Mac;
    PPHY                    pPhy                = (PPHY)pSDRContext->Phy;
    PRECV_QUEUE_MANAGER     pRecvQueueManager   = GET_RECEIVE_QUEUE_MANAGER(pMac);
    PSORA_RADIO             pRadio              = RadioInPHY(pPhy, RADIO_RECEIVE);

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    do 
    {
        SafeDequeue(pRecvQueueManager, RecvFreeList, pRCB, ULCB);
        if (!pRCB)
        {   
            DbgPrint("[RX][Warning] no free ULCB\n");
            InterlockedIncrement64(&pAdapter->ullReceiveNoBuffers);
            break;
        }

        hRes = (*pPhy->FnPHY_Rx)(pPhy, RADIO_RECEIVE, pRCB);
        if (FAILED(hRes))
        {
            InterlockedIncrement64(&pAdapter->ullReceiveErrors);
            break;
        }

        pWlanHeader = (PDOT11RFC1042ENCAP)(pRCB->pVirtualAddress);

        if (pWlanHeader->MacHeader.FrameControl.Type == FRAME_DATA)
        {
            __DataFrameACK(pAdapter, pRadio, pWlanHeader, pRCB->PacketLength);

            if (pWlanHeader->MacHeader.SequenceControl.usValue != pMac->CurRecvSeqNo)
            {
                pMac->CurRecvSeqNo = pWlanHeader->MacHeader.SequenceControl.usValue;
                __DataPacketIndicateUp(pMac, pRCB);
                InterlockedDecrement(&pRecvQueueManager->nFreeRCB);
                InterlockedIncrement(&pRecvQueueManager->nPendingRXPackets);
                pRCB = NULL; //already queued
            }
            else
            {
                InterlockedIncrement64(&pAdapter->ullDuplicatedReceives);
                DbgPrint("[RX][Warning] duplicated packet, maybe ack failed\n");
            }
        }
        else 
        {
            if (pWlanHeader->MacHeader.FrameControl.Type == FRAME_CTRL && 
                 pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_ACK)
            {
                DbgPrint("[RX]recv Ack packet \n");
                if (pMac->fTxNeedACK)
                {
                    __IndicateTXDoneAfterGetACK(pAdapter, pRadio, pWlanHeader);
                }
            }
            else
            {
                /*DbgPrint("possible beacon type=%d, subtype=%d\n",
                    pWlanHeader->MacHeader.FrameControl.Type, 
                    pWlanHeader->MacHeader.FrameControl.Subtype);*/

                if ((pWlanHeader->MacHeader.FrameControl.Type == FRAME_MAN) &&
                    pWlanHeader->MacHeader.FrameControl.Subtype == SUBT_BEACON)
                {
                    ModifyBSSID((PDOT11_MAC_BEACON)pWlanHeader);
                }
            }
        }
        
    }while (FALSE);

    if (pRCB) //ULCB is not used, queue it in free list
    {
        SafeEnqueue(pRecvQueueManager, RecvFreeList, pRCB);
    }
    KeLowerIrql(OldIrql);
    pMac->fTxNeedACK = FALSE;

    SoraFSMGotoState(StateMachine, Dot11_MAC_CS);
}
