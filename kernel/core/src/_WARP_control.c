/*++
Copyright (c) Microsoft Corporation

Module Name: _WARP_control.c

Abstract: WARP radio front end control functions.

History: 
          25/May/2009: Created by senxiang
--*/

#include "sora.h"

#include "__reg_file.h"

//static __REG_CONFIGURATION_ENTRY __gc_WARPDefaultTable[] = 
//{
//    //RFControl._2P4PA_EN_n=1, RFControl._5PA_EN_n=1, RFControl.ANTSW1=1
//    { 0x90, 0x00000001, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//    
//    //LEDControl to 2
//    { 0x94, 0x00000002, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//    
//    //MaximControl.RADIO_RXHP=0, MaximControl.RADIO_RST=1, MaximControl.RADIO_SHDN=1
//    { 0x9c, 0x00000003, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//    
//    //DACControl = 1
//    { 0xb0, 0x00000001, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//
//    //MaximGainControl.TX_Gain=0x1F RX_Gain=0x50
//    { 0xa0, 0x0050001F, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//
//    //ADCControl.ADC_DFS=1 ADC_DCS=1 ADC_PDWN_B=0 ADC_PDWN_A=0
//    { 0xa8, 0x0000000c, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//
//    //SPIInit=0
//    { 0xb8, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//    
//    //MaximSPIDataIn=0
//    { 0xc0, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//
//    //DACSPIDataIn=0
//    { 0xc4, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//
//    //RSSIADCControl=0
//    { 0xcc, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//    
//    //MaximControl.RADIO_RXHP=0, MaximControl.RADIO_RST=0, MaximControl.RADIO_SHDN=0
//    { 0x9c, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//
//    //DACControl = 0 
//    { 0xb0, 0x00000000, FALSE, 0x00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//};

/*++
WARPRFConfig set Radio Front end's registers to default value.
It is only for WARP radio.
--*/
//HRESULT WARPRFConfig(PSORA_RADIO pRadio)
//{
//    HRESULT hRes = S_OK;
//    __PHW_REGISTER_FILE pRegisterManager = &pRadio->__ctrl_reg;
//    KIRQL OldIrql;
//
//    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
//    __ConfigRegistersUnsafe(
//                pRegisterManager, 
//                __gc_WARPDefaultTable, //TBD: move it to const static data section.
//                sizeof(__gc_WARPDefaultTable) / sizeof(__REG_CONFIGURATION_ENTRY));
//    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);
//    
//    do 
//    {
//        /*hRes = WARPRFWriteMaximSPI((__PHW_REGISTER_FILE)pRegisterManager, 0x3, 0x00a0);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteMaximSPI((__PHW_REGISTER_FILE)pRegisterManager, 0x4, 0x3333);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteMaximSPI((__PHW_REGISTER_FILE)pRegisterManager, 0x5, 0x1822);*/
//        hRes = WARPRFSelectChannel(pRadio, 3); 
//        FAILED_BREAK(hRes);
//
//        hRes = WARPRFWriteMaximSPI(pRadio, 0x8, 0x0c21);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteMaximSPI(pRadio, 0x9, 0x0203);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteMaximSPI(pRadio, 0xc, 0x003f);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteDACSPI(pRadio, 0x0, 0x4);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteDACSPI(pRadio, 0x5, 0x0);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteDACSPI(pRadio, 0x6, 0xf);
//        FAILED_BREAK(hRes);
// //       hRes = WARPRFWriteDACSPI((__PHW_REGISTER_FILE)pRegisterManager, 0x7, 0x22);
//        hRes = WARPRFWriteDACSPI(pRadio, 0x7, 0x0);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteDACSPI(pRadio, 0x8, 0x0);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteDACSPI(pRadio, 0x9, 0x0);
//        FAILED_BREAK(hRes);
//        hRes = WARPRFWriteDACSPI(pRadio, 0xa, 0xf);
//        FAILED_BREAK(hRes);
////        hRes = WARPRFWriteDACSPI((__PHW_REGISTER_FILE)pRegisterManager, 0xb, 0x10);
//        hRes = WARPRFWriteDACSPI(pRadio, 0xb, 0x0);
//        FAILED_BREAK(hRes);
////        hRes = WARPRFWriteDACSPI((__PHW_REGISTER_FILE)pRegisterManager, 0xc, 0x2);
//        hRes = WARPRFWriteDACSPI(pRadio, 0xc, 0x0);
//    } while (FALSE);
//    return hRes;
//}

//HRESULT __WARPRFSelectChannelUnsafe(PSORA_RADIO pRadio, s_uint32 ChannelNumber)
//{
//    HRESULT hRes = S_OK;
//    __PHW_REGISTER_FILE pRegMan = &(pRadio->__ctrl_reg);
//
//    __REG_CONFIGURATION_ENTRY ChannelConfigEntry[] = 
//    {   //0xC0 = RF_REGISTER_SECTION_OFFSET + 0x30
//        {0xC0, 0X00000000, FALSE, 0X00000000, FALSE, 0X00000000, 0X00000000, 0X00000000, 0X00000000},
//        //write MaximSPIDataIn
//        {0xB8, 0X00000001, FALSE, 0X00000000, TRUE,  0X000000B8, 0X00000002, 0X00000002, 0X0001FFFF},
//        //write SPIInit, depend on SPIStatus
//        {0xB8, 0X00000000, FALSE, 0X00000000, FALSE, 0X00000000, 0X00000000, 0X00000000, 0X00000000},
//        //write SPIInit, depend on SPIStatus
//    };
//    do 
//    {
//        if (ChannelNumber >= 1 && ChannelNumber <=14)
//        {
//            ChannelConfigEntry[0].RegValue = __gc_Phy_2dot4GHz_ChannelSelectors[ChannelNumber - 1].Reg1;
//            hRes = __ConfigRegistersUnsafe(pRegMan, ChannelConfigEntry, 3);
//            FAILED_BREAK(hRes);
//
//            ChannelConfigEntry[0].RegValue = __gc_Phy_2dot4GHz_ChannelSelectors[ChannelNumber - 1].Reg2;
//            hRes = __ConfigRegistersUnsafe(pRegMan, ChannelConfigEntry, 3);
//            FAILED_BREAK(hRes);
//
//            ChannelConfigEntry[0].RegValue = __gc_Phy_2dot4GHz_ChannelSelectors[ChannelNumber - 1].Reg3;
//            hRes = __ConfigRegistersUnsafe(pRegMan, ChannelConfigEntry, 3);
//            FAILED_BREAK(hRes);
//
//            break;
//        }
//        else if (ChannelNumber >= 36 && ChannelNumber <= 161)
//        {
//            ChannelConfigEntry[0].RegValue = __gc_Phy_5GHz_ChannelSelectors[ChannelNumber - 36].Reg1;
//            hRes = __ConfigRegistersUnsafe(pRegMan, ChannelConfigEntry, 3);
//            FAILED_BREAK(hRes);
//
//            ChannelConfigEntry[0].RegValue = __gc_Phy_5GHz_ChannelSelectors[ChannelNumber - 36].Reg2;
//            hRes = __ConfigRegistersUnsafe(pRegMan, ChannelConfigEntry, 3);
//            FAILED_BREAK(hRes);
//
//            ChannelConfigEntry[0].RegValue = __gc_Phy_5GHz_ChannelSelectors[ChannelNumber - 36].Reg3;
//            hRes = __ConfigRegistersUnsafe(pRegMan, ChannelConfigEntry, 3);
//            FAILED_BREAK(hRes);
//
//            break;
//        }
//        else
//        {
//            // Invalid Channel Number
//            hRes = E_FAIL;
//            break;
//        }
//    } while (FALSE);
//
//    return hRes;
//}
//
//__REG_CONFIGURATION_ENTRY __gc_RadioRegMaximSPITemplate[] = 
//{
//    {0xC0, 0Xcdcdcdcd, FALSE, 0X00000000, FALSE, 0X00000000, 0XFFFFFFFF, 0X00000000, 0X0001FFFF},
//    //SPIInit
//    {0xB8, 0X00000001, FALSE, 0X00000000, TRUE,  0X000000B8, 0X00000002, 0X00000002, 0X0001FFFF},
//    //SPIInit 
//    {0xB8, 0X00000000, FALSE, 0X00000000, FALSE, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
//};
//
//__REG_CONFIGURATION_ENTRY __gc_RadioRegDACSPITemplate[] = 
//{
//    {0xC4, 0Xcdcdcdcd, FALSE, 0X00000000, FALSE, 0X00000000, 0x00000000, 0X00000000, 0x00000000},
//    {0xBC, 0X00000001, FALSE, 0X00000000, TRUE,  0X000000BC, 0X00000002, 0X00000002, 0X0001FFFF},
//    {0xBC, 0X00000000, FALSE, 0X00000000, FALSE, 0X00000000, 0X00000000, 0X00000000, 0X00000000},
//};
//
//
//__inline 
//HRESULT
//__WARPRFWriteDACSPIUnsafe(
//    SORA_REGS_HANDLE pRegisterManager, 
//    s_uint32 address, 
//    s_uint32 value)
//{
//    HRESULT hRes = S_OK;
//    s_uint32 reg = (value & 0x0FF) + ((address &0x01F) << 8);
//    __gc_RadioRegDACSPITemplate[0].RegValue = reg;
//    hRes = __ConfigRegistersUnsafe(
//                (__PHW_REGISTER_FILE)pRegisterManager, 
//                __gc_RadioRegDACSPITemplate,
//                sizeof(__gc_RadioRegDACSPITemplate) / sizeof(__REG_CONFIGURATION_ENTRY));
//    return hRes;
//}
//
//__inline
//HRESULT 
//__WARPRFWriteMaximSPIUnsafe(
//    SORA_REGS_HANDLE pRegisterManager, 
//    s_uint32 address, 
//    s_uint32 value)
//{
//    HRESULT hRes = S_OK;
//    s_uint32 reg = (value << 4) + (address & 0x0F);
//    __gc_RadioRegMaximSPITemplate[0].RegValue = reg;
//    hRes = __ConfigRegistersUnsafe(
//                (__PHW_REGISTER_FILE)pRegisterManager, 
//                __gc_RadioRegMaximSPITemplate,
//                sizeof(__gc_RadioRegMaximSPITemplate) / sizeof(__REG_CONFIGURATION_ENTRY));
//    return hRes;
//}
//
//
//HRESULT 
//WARPRFWriteDACSPI(
//    PSORA_RADIO pRadio,
//    s_uint32 address, 
//    s_uint32 value)
//{
//    
//    KIRQL OldIrql;
//    HRESULT hr;
//    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
//    hr = __WARPRFWriteDACSPIUnsafe(&pRadio->__ctrl_reg, address, value);
//    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);
//    return hr;
//}
//
//HRESULT 
//WARPRFWriteMaximSPI(
//    PSORA_RADIO pRadio, 
//    s_uint32 address, 
//    s_uint32 value)
//{
//    KIRQL OldIrql;
//    HRESULT hr;
//    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
//    hr = __WARPRFWriteMaximSPIUnsafe(&pRadio->__ctrl_reg, address, value);
//    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);
//    return hr;
//}

//HRESULT WARPRFSelectChannel(PSORA_RADIO pRadio, s_uint32 ChannelNumber)
//{
//    KIRQL OldIrql;
//    HRESULT hr;
//    KeAcquireSpinLock(&pRadio->__HWOpLock, &OldIrql);
//    hr = __WARPRFSelectChannelUnsafe(pRadio, ChannelNumber);
//    KeReleaseSpinLock(&pRadio->__HWOpLock, OldIrql);
//    return hr;
//}

//__inline s_uint32 
//__WARPRFReadDACSPI(
//    PSORA_RADIO pRadio,
//    s_uint32 address)
//{
//    s_uint32 value = 0xFFFFFFFF;
//    s_uint32 reg = (0x1 << 16) + ((address & 0x1f) << 8);
//    s_uint32 SPIStatusValue;
//    s_uint32 timeout = 0X0001FFFF;
//
//    SORA_HW_WRITE_RF_REGISTER32(pRadio, WARP_REGS, DACSPIDataIn, reg);
//    SORA_HW_WRITE_RF_REGISTER32(pRadio, WARP_REGS, SPIInit, 0x2);
//
//    do
//    {
//        SORA_HW_READ_RF_REGISTER32(pRadio, WARP_REGS, SPIStatus, SPIStatusValue);
//        timeout--;
//    } while ((SPIStatusValue & 0x2) == 0 && (timeout != 0)); //loop if DAC_SPI_DONE == 0 and not timeout
//
//    if (timeout > 0)
//    {
//        SORA_HW_READ_RF_REGISTER32(pRadio, WARP_REGS, DACSPIDataOut, value);
//    }
//    return value;
//}
//
//s_uint32 WARPRFReadDACSPI(PSORA_RADIO pRadio, s_uint32 address)
//{
//    return __WARPRFReadDACSPI(pRadio, address);
//}

//VOID WARPRFWriteMaximGain(PSORA_RADIO pRadio, ULONG uGain)
//{
//    SORA_HW_WRITE_RF_REGISTER32(pRadio, WARP_REGS, MaximGainControl, uGain);
//}

// Set TX VGA1
//VOID SoraHwSetTXVGA1(PSORA_RADIO pRadio, ULONG db_1_256)
//{
//    s_uint32 rxgain = (pRadio->__uRxGain >> 7) << 16;
//    s_uint32 txgain = ((db_1_256 >> 7) & 0x0000FFFF);
//    pRadio->__uTxGain = db_1_256;
//    //DbgPrint("[TEMP1] RX/TX gain %08x", rxgain + txgain);
//    SORA_HW_WRITE_RF_REGISTER32(pRadio, WARP_REGS, MaximGainControl, rxgain + txgain);
//}

// Set RX VGA1
//VOID SoraHwSetRXVGA1(PSORA_RADIO pRadio, ULONG db_1_256)
//{
//    s_uint32 txgain = (pRadio->__uTxGain >> 7) & 0x0000FFFF;
//    s_uint32 rxgain = ((db_1_256 >> 7) << 16);
//
//    pRadio->__uRxGain = db_1_256;
//    //DbgPrint("[TEMP1] RX/TX gain %08x", rxgain + txgain);
//    SORA_HW_WRITE_RF_REGISTER32(pRadio, WARP_REGS, MaximGainControl, rxgain + txgain);
//}