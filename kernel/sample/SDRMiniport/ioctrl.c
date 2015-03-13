/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name: 
    ioctrl.c

Abstracts:
    Device IO control module, with which TX/RX parameter can be configured at runtime. 

Revision History:
    Created by senxiang, 20/Jul/2009

Notes: 

--*/
#include "miniport.h"
#include "sora.h"
#include "trace.h"
#ifdef EVENT_TRACING
#include "ioctrl.tmh"
#endif
#include <ntstrsafe.h>

void GetInfo(IN PMP_ADAPTER pAdapter, OUT PSTATISTIC pOut)
{
    PMAC            pMac        = &pAdapter->Mac;
    PSORA_RADIO     pRadio      = RadioInPHY(&pAdapter->Phy, RADIO_RECEIVE);

    pOut->nFreeTCB              = GET_SEND_QUEUE_MANAGER(pMac)->nFreeTCB;
    pOut->nSrcPacket            = GET_SEND_QUEUE_MANAGER(pMac)->nSrcPacket;
    pOut->nSymPacket            = GET_SEND_QUEUE_MANAGER(pMac)->nSymPacket;
    pOut->nCompletePacket       = GET_SEND_QUEUE_MANAGER(pMac)->nCompletePacket;
    
    pOut->nFreeRCB              = GET_RECEIVE_QUEUE_MANAGER(pMac)->nFreeRCB;
    pOut->nPendingRXPackets     = GET_RECEIVE_QUEUE_MANAGER(pMac)->nPendingRXPackets;

    pOut->ullGoodReceives       = pAdapter->ullGoodReceives;
    pOut->ullGoodTransmits      = pAdapter->ullGoodTransmits;
    pOut->ullReceiveErrors      = pAdapter->ullReceiveErrors;
    pOut->ullTransmitFail       = pAdapter->ullTransmitFail;
    pOut->ullDuplicatedReceives = pAdapter->ullDuplicatedReceives;
    pOut->ulSpdEnergy           = pAdapter->Phy.BBContextFor11B.SpdContext.b_evalEnergy;
    pOut->ulDataRate            = pAdapter->Phy.BBContextFor11B.TxVector.DateRate;
    pOut->ulPreambleType        = pAdapter->Phy.BBContextFor11B.TxVector.PreambleType;
    pOut->ulSpdPowerThreshold   = pAdapter->Phy.BBContextFor11B.SpdContext.b_threshold;
    pOut->ulSpdPowerThresholdLH = pAdapter->Phy.BBContextFor11B.SpdContext.b_thresholdLH;
    pOut->ulSpdPowerThresholdHL = pAdapter->Phy.BBContextFor11B.SpdContext.b_thresholdHL;
    pOut->ulGainLevel           = pAdapter->Phy.BBContextFor11B.SpdContext.b_gainLevel;
    pOut->ulRxGainPreset0       = pAdapter->Phy.RxGainPreset0;
    pOut->ulRxGainPreset1       = pAdapter->Phy.RxGainPreset1;
    pOut->ulRxGain              = SORA_RADIO_GET_RX_GAIN(pRadio);
    pOut->ulRxPa                = SORA_RADIO_GET_RX_PA(pRadio);
    pOut->ulTxGain              = SORA_RADIO_GET_TX_GAIN(pRadio);
    pOut->ulRxShift             = pAdapter->Phy.BBContextFor11B.RxContext.b_shiftRight;
    pOut->ulSampleRate          = pAdapter->Phy.BBContextFor11A.RxContext.SampleRate;

    RtlCopyMemory(pOut->MACAddr, pAdapter->CurrentAddress, ETH_LENGTH_OF_ADDRESS);
}

#define DEFAULT_DATA_RATE DOT11A_RATE_24M
unsigned int Dot11AKbpsToDataRate(ULONG uKbps)
{
    static unsigned int Dot11AKbpsToDataRateMap[17] = 
    {DOT11A_RATE_6M, DOT11A_RATE_9M, DOT11A_RATE_12M, DEFAULT_DATA_RATE, 
    DOT11A_RATE_18M, DEFAULT_DATA_RATE, DOT11A_RATE_24M, DEFAULT_DATA_RATE,
    DEFAULT_DATA_RATE, DEFAULT_DATA_RATE, DOT11A_RATE_36M, DEFAULT_DATA_RATE, 
    DEFAULT_DATA_RATE, DEFAULT_DATA_RATE, DOT11A_RATE_48M, DEFAULT_DATA_RATE,
    DOT11A_RATE_54M};
    int i = uKbps / (3 * 1000) - 2;
    unsigned int ret = DEFAULT_DATA_RATE;
    if (i >=0 && i < 17)
        ret = Dot11AKbpsToDataRateMap[i];
    return ret;
}


/*
MPDeviceIoControl handles IRP from applications or other drivers.
    
Parameters:
    pDevObj:  pointer to device object created in DriverEntry
    pIrp   :  IRP sent to this device.

    Return:
    STATUS_SUCCESS if IRP is handled successfully.

    IRQL: PASSIVE_LEVEL
*/
NTSTATUS  MPDeviceIoControl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
   
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG cbin = 0, code = 0, info = 0;
    PULONG pInputBuffer;
    ULONG               uInputValue = 0;
    PMP_ADAPTER         pAdapter    = g_pGlobalAdapter;
    PIO_STACK_LOCATION  pStack      = IoGetCurrentIrpStackLocation(pIrp);
    PSORA_RADIO         pRadio      = RadioInPHY(&pAdapter->Phy, RADIO_RECEIVE);

    cbin   = pStack->Parameters.DeviceIoControl.InputBufferLength;
    code   = pStack->Parameters.DeviceIoControl.IoControlCode;
    
    PAGED_CODE();
    
    do
    {
        if (!NIC_IS_DEVICE_CAN_WORK(pAdapter))
        {
            DEBUGP(MP_WARNING, ("[IOCTL] network device is not started normally \n"));
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
        if(pStack->MajorFunction != IRP_MJ_DEVICE_CONTROL)
        {
            DEBUGP(MP_WARNING, ("[INIT] Device Request Invalid\n"));
            break;
        }
        
        pInputBuffer = (PULONG) pIrp->AssociatedIrp.SystemBuffer;
        uInputValue = *pInputBuffer;

        switch(code)
        {
        case IOCTL_UPDATE_MAC_ADDR:
            pAdapter->CurrentAddress[0] = ((PUCHAR)pInputBuffer)[0];
            pAdapter->CurrentAddress[1] = ((PUCHAR)pInputBuffer)[1];
            pAdapter->CurrentAddress[2] = ((PUCHAR)pInputBuffer)[2];
            pAdapter->CurrentAddress[3] = ((PUCHAR)pInputBuffer)[3];
            pAdapter->CurrentAddress[4] = ((PUCHAR)pInputBuffer)[4];
            pAdapter->CurrentAddress[5] = ((PUCHAR)pInputBuffer)[5];
            DbgPrint("MacAddr=[%02X-%02X-%02X-%02X-%02X-%02X]\n",
                pAdapter->CurrentAddress[0], pAdapter->CurrentAddress[1], pAdapter->CurrentAddress[2],
                pAdapter->CurrentAddress[3], pAdapter->CurrentAddress[4], pAdapter->CurrentAddress[5]);    
            //TODO: store the address to registry
            break;
        case IOCTL_GET_INFO:
            cbin = pStack->Parameters.DeviceIoControl.OutputBufferLength;
            if (cbin >= sizeof(STATISTIC))
            {
                GetInfo(pAdapter, (PSTATISTIC)pInputBuffer);
                info = sizeof(STATISTIC);
                DEBUGP(MP_INFO, ("[IOCTL] IOCTL_GET_INFO, output len = %d\n", cbin));
            }
            break;
        case IOCTL_SET_DATA_RATE:
            if (pAdapter && (uInputValue == DATA_RATE_11000 || uInputValue == DATA_RATE_5500 
                || uInputValue == DATA_RATE_2000 || uInputValue == DATA_RATE_1000))
            {
                pAdapter->TxParameters[0] = (UCHAR)(uInputValue / 100);
                pAdapter->Phy.BBContextFor11B.TxVector.DateRate = (UCHAR)pAdapter->TxParameters[0];
                DEBUGP(MP_INFO, ("[IOCTL] Set 11B data rate:0X%02x", pAdapter->TxParameters[0]));
            }
            else if(pAdapter && (uInputValue == 6000 || uInputValue == 9000 
                || uInputValue == 12000 || uInputValue == 18000 ||
                uInputValue == 24000 || uInputValue == 36000 || 
                uInputValue == 48000 || uInputValue == 54000))
            {
                unsigned int rate = Dot11AKbpsToDataRate(uInputValue);
                pAdapter->Phy.BBContextFor11A.TxVector.ti_uiDataRate = rate;
                DEBUGP(MP_INFO, ("[IOCTL] Set 11A data rate:0X%02x", rate));
            }
            else
            {
                Status = STATUS_FAIL_CHECK;
            }
            break;
        case IOCTL_SET_PREAMBLE_TYPE:
            if (pAdapter && (uInputValue == DOT11B_PLCP_IS_LONG_PREAMBLE || uInputValue == DOT11B_PLCP_IS_SHORT_PREAMBLE))
            {
                pAdapter->TxParameters[2] = (UCHAR)uInputValue;
                pAdapter->Phy.BBContextFor11B.TxVector.PreambleType = (UCHAR)uInputValue;
                DEBUGP(MP_INFO, ("[IOCTL] Set preamble:0X%02x", pAdapter->TxParameters[2]));
            }
            else
            {
                Status = STATUS_FAIL_CHECK;
            }
            break;
        case IOCTL_SET_CHANNEL:
            //SoraHwSetCentralFreq(pRadio, uInputValue * 1000, 0);
            //channel 1: 2412MHz, 2: 2417Mhz.
            if (uInputValue >= 36 && uInputValue <= 161)
            {
                SoraHwSetCentralFreq(pRadio, (5180 + (uInputValue - 36) * 5) * 1000, 0);
            }
            else if (uInputValue >= 162)
            {
                SoraHwSetCentralFreq(pRadio, (5000 + (uInputValue - 162) * 350) * 1000, 0);
            }
            else
            {
                SoraHwSetCentralFreq(pRadio, (2407 + uInputValue * 5) * 1000, 0);
            }
            DEBUGP(MP_INFO, ("[IOCTL] Set Channel:%d\n", uInputValue));
            info = cbin;
            break;
        case IOCTL_SET_FREQ_OFFSET:
            SoraHwSetFreqCompensation(pRadio, (LONG)uInputValue);
            DEBUGP(MP_INFO, ("[IOCTL] Set Frequency offset:%d\n", (LONG)uInputValue));
            break;
        case IOCTL_SET_RX_GAIN:
            DEBUGP(MP_INFO, ("[IOCTL] rx gain:0X%02X\n",uInputValue));
            SoraHwSetRXVGA1(pRadio, uInputValue);
            info = cbin;
            break;
        case IOCTL_SET_RX_PA:
            DEBUGP(MP_INFO, ("[IOCTL] rx PA:0X%04X\n", uInputValue));
            SoraHwSetRXPA(pRadio, uInputValue);
            info = cbin;
            break;
        case IOCTL_SET_RX_GAIN_PRESET0:
            DEBUGP(MP_INFO, ("[IOCTL] rx gain preset0:0X%02X\n",uInputValue));
            pAdapter->Phy.RxGainPreset0 = uInputValue;
            info = cbin;
            break;
        case IOCTL_SET_RX_GAIN_PRESET1:
            DEBUGP(MP_INFO, ("[IOCTL] rx gain preset1:0X%02X\n",uInputValue));
            pAdapter->Phy.RxGainPreset1 = uInputValue;
            info = cbin;
            break;
        case IOCTL_SET_TX_GAIN:
            DEBUGP(MP_INFO, ("[IOCTL] tx gain:0X%02X\n",uInputValue));
            SoraHwSetTXVGA1(pRadio, uInputValue);
            break;
        case IOCTL_SET_RX_DUMP:
            pAdapter->Mac.fDumpMode = 1;
            info = cbin;
            DEBUGP(MP_INFO, ("[IOCTL] Set RX DUMP\n"));
            break;
        case IOCTL_SET_SPD_MAX_BLOCK:
            DEBUGP(MP_INFO, ("[IOCTL] Set SPD Block timeout 0X%08X\n", uInputValue));
            pAdapter->Phy.BBContextFor11B.SpdContext.b_maxDescCount = uInputValue;
            break;
        case IOCTL_SET_SPD_POWER_THRESHOLD:
            DEBUGP(MP_INFO, ("[IOCTL] Set SPD energy threshold 0X%08X\n", uInputValue));
            pAdapter->Phy.BBContextFor11B.SpdContext.b_threshold = uInputValue;
            break;
        case IOCTL_SET_SPD_POWER_THRESHOLD_LH:
            DEBUGP(MP_INFO, ("[IOCTL] Set SPD energy threshold LH 0X%08X\n", uInputValue));
            pAdapter->Phy.BBContextFor11B.SpdContext.b_thresholdLH = uInputValue;
            break;
        case IOCTL_SET_SPD_POWER_THRESHOLD_HL:
            DEBUGP(MP_INFO, ("[IOCTL] Set SPD energy threshold HL 0X%08X\n", uInputValue));
            pAdapter->Phy.BBContextFor11B.SpdContext.b_thresholdHL = uInputValue;
            break;
        case IOCTL_SET_SAMPLE_RATE:
            if (pAdapter &&
                (uInputValue == 40 || uInputValue == 44))
            {
                DEBUGP(MP_INFO, ("[IOCTL] Set sample rate of the radio PCB 0X%08X\n", uInputValue));
                pAdapter->Phy.BBContextFor11A.RxContext.SampleRate = uInputValue;
                pAdapter->Phy.BBContextFor11A.TxVector.SampleRate = uInputValue;
            }
            else
            {
                Status = STATUS_FAIL_CHECK;
            }
            break;
        case IOCTL_DISPLAY_DEBUG_REGS:
            DEBUGP(MP_INFO, ("[IOCTL] print debug registers\n"));
            SoraHwPrintDbgRegs(pAdapter->TransferObj);
            SoraHwPrintRadioRegs(pRadio);
            break;
        default:
            DEBUGP(MP_INFO, ("[IOCTL] User Extension code\n"));
            info = SoraKHandleUExtReq(
                &pAdapter->KeAppExtObj,
                pAdapter->TransferObj,
                &pRadio, 
                code, 
                pInputBuffer, 
                pStack->Parameters.DeviceIoControl.InputBufferLength, 
                pInputBuffer,
                pStack->Parameters.DeviceIoControl.OutputBufferLength);
            break;
        }
    }while(FALSE);

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = info;
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);
    return Status;
}


