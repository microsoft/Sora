#include "miniport.h"

ULONG __FillWithDefaultValue(PVOID Buffer, ULONG s);
extern PACKET_BASE g_Packet;

HRESULT AutoTest(PMP_ADAPTER Adapter, int argc, char *argv[])
{
    ARG_STRING  timesArg   = {{"t", "times", "Specify Loop times", STR}, NULL, 0};
    void *ArgTable[] = {&timesArg};
    
    ULONG nLoopCount = 0;
    ULONG TestType = 0;
    HRESULT hr = E_CMD_SYNTAX;

    if (argc >= 1)
    {
        ParseHex(argv[0], &TestType);
        if (argc >= 3)
        {
            int ret = ParseArg(argc - 1, &argv[1], ArgTable, sizeof(ArgTable)/sizeof(PVOID));
            if (timesArg.Count == 1)
            {
                ParseHex(timesArg.szStrValue, &nLoopCount);
            }
        }
    }

    switch (TestType)
    {
    case 0:
        break;
    }

    return hr;
}

HRESULT MixedRxTf(PSORA_RADIO pRadio)
{
    __REG32_TRANS_CTRL TransCtrl;
    __PSORA_RADIO_REGS pRegs = pRadio->__ctrl_reg.pRadioRegs;
    ULONG              uTemp  = 100000;

    pPhyFrameDesc->__FrameCtrlOwn = 1; //software own the phy frame

    TransCtrl.Value = 0;
    TransCtrl.Bits.TransferInit  = 1;

    WRITE_REGISTER_ULONG(
        (PULONG)&pRegs->TransferSrcAddrL, pPhyFrameDesc->ThisPa.u.LowPart);
    
    WRITE_REGISTER_ULONG(
        (PULONG)&pRegs->TransferSrcAddrH, pPhyFrameDesc->ThisPa.u.HighPart);
    WRITE_REGISTER_ULONG(
        (PULONG)&pRegs->TransferControl, TransCtrl.Value);

    while(pPhyFrameDesc->__FrameCtrlOwn == 1) //wait until hw owns the phy frame
    //hw will set it if TXDone. PHY frame desc is in MmWriteCombined mem, so not cached.
    {
        {
            __REG32_RX_CTRL RXControl;
            RXControl.Value         = 0;
            RXControl.Bits.RXEnable = 1;
            
            WRITE_REGISTER_ULONG(
                (PULONG)&pRadio->__phy_reg_manager.pRadioRegs->RXControl,
                RXControl.Value);
            pRadio->__fRxEnabled = TRUE;
        }
        
        uTemp--;
        if(uTemp < 1) {
            KdPrint(("Wait Own Timeout!"));
            hr = E_TX_TRANSFER_FAIL;
            break;
        }
        _mm_pause();
    }

    TransCtrl.Value = 0;
    WRITE_REGISTER_ULONG((PULONG)&pRegs->TransferControl, TransCtrl.Value); //clear init bit
    return hr;
}

HRESULT RXInterruptTransfer(PMP_ADAPTER Adapter)
{
    HRESULT hr;
    PSORA_RADIO     pRadio = &Adapter->RadioManager.__radio_pool[0];
    ULONG SampleSize;
    PRCB_MD        pTxRCBMem;

    DbgPrint("[MIX] Get symbol from file, size = %08x \n", SampleSize);
    do {
        hr = SoraPacketGetTxResource(pRadio, &g_Packet);
        FAILED_BREAK(hr);

        SampleSize = __FillWithDefaultValue(g_Packet.pTxDesc->pSampleBuffer, MODULATE_BUFFER_SIZE);    

        SoraPacketSetPhyFrameSize(&g_Packet, SampleSize);

        if (g_Packet.pTxDesc->FrameSize & RCB_BUFFER_ALIGN_MASK)
        {
            hr = E_INVALID_SIGNAL_SIZE;
            break;
        }

        pTxRCBMem = SoraAllocateRCBMem(pRadio->pTxResMgr, pPacket->pTxDesc->FrameSize);
        if (pTxRCBMem)
        {
            pPacket->pTxDesc->__RCBDestAddr   = pTxRCBMem->Start;
            pPacket->pTxDesc->pRMD    = pTxRCBMem;
            
                       
            
            if (SUCCEEDED(hr))
            {
                pPacket->pTxDesc->pSampleBuffer      = NULL;
                pPacket->pTxDesc->SampleBufferSize   = 0;
                InterlockedExchange(&pPacket->fStatus, PACKET_CAN_TX);
            }
            else
            { //release ULCB TX memory
                SoraFreeRCBMem(pRadio->pTxResMgr, pPacket->pTxDesc->pRMD);
                pPacket->pTxDesc->__RCBDestAddr   = 0xcdcdcdcd;
                pPacket->pTxDesc->pRMD    = NULL;
                InterlockedExchange(&pPacket->fStatus, PACKET_TF_FAIL);
            }
        }
    }
    }while(FALSE);

    return hr;
}