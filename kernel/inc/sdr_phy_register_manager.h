#ifndef _PCIE_REG_H_
#define _PCIE_REG_H_

//*****************************************************************************
//  
//  File Name: Register.h
// 
//  Description:  This file defines all the PCI Express Design Sample Card's Registers.
// 
//  NOTE: These definitions are for memory-mapped register access only.
//
//*****************************************************************************


typedef union _REG32_COMMON{
    struct{
        ULONG      Reserved              : 32;
    }Bits;
    ULONG Value;
}REG32_COMMON, *PREG32_COMMON;

typedef union _REG32_TX_CONTROL{
    struct{
        ULONG      TransferInit          : 1;
        ULONG      TxInit                : 1;
        ULONG      Reserved              : 30;
    }Bits;
    ULONG      Value;
}REG32_TX_CONTROL, *PREG32_TX_CONTROL;

typedef union _REG32_RX_CONTROL{
    struct{
        ULONG      CMFParaDone           : 1;
        ULONG      RXReset               : 1;
        ULONG      ResetPreaDetected     : 1;
        ULONG      Reserved              : 29;
    }Bits;
    ULONG      Value;
}REG32_RX_CONTROL, *PREG32_RX_CONTROL;

typedef union _REG32_TXRX_STATUS{
    struct{
        ULONG  TXDone                    : 1;
        ULONG  TransferDone              : 1;
        ULONG  PreaDetected              : 1;
        ULONG  CSDetected                : 1;
        ULONG  Reserved                  : 28;
    }Bits;
    ULONG      Value;    
}REG32_TXRX_STATUS, *PREG32_TXRX_STATUS;

typedef union _REG32_INTERRUPT_FLAG{
    struct{
        ULONG  IsTxDoneInterrupt         : 1;
        ULONG  IsPreDetectedInterrupt    : 1;
        ULONG  Reserved                  : 30;
    }Bits;
    ULONG      Value;
}REG32_INTERRUPT_FLAG, *PREG32_INTERRUPT_FLAG;

typedef union _REG32_INTERRUPT_MASK{
    struct{
        ULONG  TXDoneInterruptMask       : 1;
        ULONG  PreDetectedInterruptMask  : 1;
        ULONG  Reserved                  : 30;
    }Bits;
    ULONG      Value;
}REG32_INTERRUPT_MASK, *PREG32_INTERRUPT_MASK;

typedef union _REG32_RF_CONTROL{
    struct{
        ULONG  M2P4PA_EN                 : 1;
        ULONG  M5PA_EN                   : 1;
        ULONG  ANTSW1                    : 1; // ANTSW1 : ANTSW2       STATE
        ULONG  ANTSW2                    : 1; //    1   :   0        ANT1 TO RX, ANT2 TO TX
        //    0   :   1        ANT2 TO RX, ANT1 TO TX
        ULONG  Reserved                  : 28;
    }Bits;
    ULONG      Value;
}REG32_RF_CONTROL, *PREG32_RF_CONTROL;

typedef union _REG32_LED_CONTROL{
    struct{
        ULONG  LED1                      : 1;
        ULONG  LED2                      : 1;
        ULONG  LED3                      : 1;
        ULONG  Reserved                  : 29;
    }Bits;
    ULONG      Value;
}REG32_LED_CONTROL, *PREG32_LED_CONTROL;

typedef union _REG32_DIPSW{
    struct{
        ULONG  DIPSW1                    : 1;
        ULONG  DIPSW2                    : 1;
        ULONG  DIPSW3                    : 1;
        ULONG  DIPSW4                    : 1;
        ULONG  Reserved                  : 28;
    }Bits;
    ULONG      Value;
}REG32_DIPSW, *PREG32_DIPSW;

typedef union _REG32_MAXIM_CONTROL{
    struct{
        ULONG  RADIO_SHDN                : 1;
        ULONG  RADIO_TXEN                : 1;
        ULONG  RADIO_RXEN                : 1;
        ULONG  RADIO_RXHP                : 1;
        ULONG  Reserved                  : 28;
    }Bits;
    ULONG      Value;
}REG32_MAXIM_CONTROL, *PREG32_MAXIM_CONTROL;

typedef union _REG32_MAXIM_GAIN_CONTROL{
    struct{
        ULONG  B1TOB7                    : 7;
        ULONG  Reserved                  : 25;
    }Bits;
    ULONG      Value;
}REG32_MAXIM_GAIN_CONTROL, *PREG32_MAXIM_GAIN_CONTROL;

typedef union _REG32_MAXIM_STATUS{
    struct{
        ULONG  RADIO_LD                  : 1;
        ULONG  Reserved                  : 31;
    }Bits;
    ULONG      Value;
}REG32_MAXIM_STATUS, *PREG32_MAXIM_STATUS;

typedef union _REG32_ADC_CONTROL{
    struct{
        ULONG  ADC_PDWN_A                : 1;
        ULONG  ADC_PDWN_B                : 1;
        ULONG  ADC_DCS                   : 1;
        ULONG  ADC_DFS                   : 1;
        ULONG  Reserved                  : 28;
    }Bits;
    ULONG      Value;
}REG32_ADC_CONTROL, *PREG32_ADC_CONTROL;

typedef union _REG32_ADC_STATUS{
    struct{
        ULONG  ADC_OTR_A                 : 1;
        ULONG  ADC_OTR_B                 : 1;
        ULONG  Reserved                  : 30;
    }Bits;
    ULONG      Value;
}REG32_ADC_STATUS, *PREG32_ADC_STATUS;

typedef union _REG32_DAC_CONTROL{
    struct{
        ULONG  DAC_RESET                 : 1;
        ULONG  Reserved                  : 31;
    }Bits;
    ULONG      Value;
}REG32_DAC_CONTROL, *PREG32_DAC_CONTROL;

typedef union _REG32_DAC_STATUS{
    struct{
        ULONG  DAC_PLL_LOCK              : 1;
        ULONG  Reserved                  : 31;
    }Bits;
    ULONG      Value;
}REG32_DAC_STATUS, *PREG32_DAC_STATUS;

typedef union _REG32_SPI_INIT{
    struct{
        ULONG  MAXIM_SPI_INIT            : 1;
        ULONG  DAC_SPI_INIT              : 1;
        ULONG  Reserved                  : 30;
    }Bits;
    ULONG      Value;
}REG32_SPI_INIT, *PREG32_SPI_INIT;

typedef union _REG32_SPI_STATUS{
    struct{
        ULONG  MAXIM_SPI_DONE            : 1;
        ULONG  DAC_SPI_DONE              : 1;
        ULONG  Reserved                  : 30;
    }Bits;
    ULONG      Value;
}REG32_SPI_STATUS, *PREG32_SPI_STATUS;

typedef union _REG32_MAXIM_SPI_DATA_IN{
    struct{
        ULONG  A0TOA3                    : 4;
        ULONG  D0TOD13                   : 14;
        ULONG  Reserved                  : 14;
    }Bits;
    ULONG      Value;
}REG32_MAXIM_SPI_DATA_IN, *PREG32_MAXIM_SPI_DATA_IN;

typedef union _REG32_DAC_SPI_DATA_IN{
    struct{
        ULONG  RegisterUCHAR              : 8;
        ULONG  InstructionUCHAR           : 8;
        ULONG  Reserved                  : 16;
    }Bits;
    ULONG      Value;
}REG32_DAC_SPI_DATA_IN, *PREG32_DAC_SPI_DATA_IN;

typedef union _REG32_DAC_SPI_DATA_OUT{
    struct{
        ULONG  RegisterUCHAR              : 8;
        ULONG  Reserved                  : 24;
    }Bits;
    ULONG      Value;
}REG32_DAC_SPI_DATA_OUT, *PREG32_DAC_SPI_DATA_OUT;

typedef union _REG32_RSSI_ADC_CONTROL{
    struct{
        ULONG  RSSI_ADC_Clamp            : 1;
        ULONG  RSSI_ADC_Sleep            : 1;
        ULONG  RSSI_ADC_HIZ              : 1;
        ULONG  Reserved                  : 29;
    }Bits;
    ULONG      Value;
}REG32_RSSI_ADC_CONTROL, *PREG32_RSSI_ADC_CONTROL;

typedef union _REG32_RSSI_ADC_DATA{
    struct{
        ULONG  RSSI_ADC_D0TOD9           : 10;
        ULONG  Reserved1                 : 6;
        ULONG  RSSI_OTR                  : 1;
        ULONG  Reserved2                 : 15;
    }Bits;
    ULONG      Value;
}REG32_RSSI_ADC_DATA, *PREG32_RSSI_ADC_DATA;

typedef union _REG32_SDRAM_SLOT_INDICATOR{
    struct{
        ULONG  SlotIndex                 : 16;
        ULONG  fReserveSlot              : 16;
    }Bits;
    ULONG      Value;
}REG32_SDRAM_SLOT_INDICATOR, *PREG32_SDRAM_SLOT_INDICATOR;



///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _SDR_REGISTER
{
    //=========================== Registers ===================================
    //  Type                        Name                              Address(Hex)
    //=========================================================================


    //================== Reserved Registers ===============================
    REG32_COMMON                       Reg1;                             // 00
    REG32_COMMON                       Reg2;                             // 04
    REG32_COMMON                       Reg3;                             // 08
    REG32_COMMON                       Reg4;                             // 0C
    REG32_COMMON                       Reg5;                             // 10
    REG32_COMMON                       Reg6;                             // 14
    REG32_COMMON                       Reg7;                             // 18 
    REG32_COMMON                       Reg8;                             // 1C
    REG32_COMMON                       Reg9;                             // 20
    REG32_COMMON                       Reg10;                            // 24
    REG32_COMMON                       Reg11;                            // 28
    REG32_COMMON                       Reg12;                            // 2C
    REG32_COMMON                       Reg13;                            // 30
    REG32_COMMON                       Reg14;                            // 34
    REG32_COMMON                       Reg15;                            // 38
    REG32_COMMON                       Reg16;                            // 3C

    //======================= SDR Registers ===============================

    //====================  PC Write, FPGA Read  ==========================
    REG32_TX_CONTROL                   TXControl;                        // 40
    REG32_COMMON                       TXFrameAddress;                   // 44
    REG32_COMMON                       TXFrameID;                        // 48

    REG32_COMMON                       MaxRXSize;                        // 4C
    REG32_RX_CONTROL                   RXControl;                        // 50
    REG32_COMMON                       CurrentRXAddress;                 // 54

    REG32_COMMON                       CMFPara1;                         // 58
    REG32_COMMON                       CMFPara2;                         // 5C
    REG32_COMMON                       CMFPara3;                         // 60

    //====================  FPGA Write, PC Read  ==========================

    REG32_TXRX_STATUS                  TXRXStatus;                       // 64
    REG32_COMMON                       RXAddress;                        // 68

    REG32_INTERRUPT_FLAG               InterruptFlag;                    // 6c
    REG32_INTERRUPT_MASK               InterruptMask;                    // 70

    REG32_COMMON                       TXInitDelay;                      // 74

    REG32_COMMON                       TXBurstSize;                      // 78
    REG32_COMMON                       RXBurstSize;                      // 7c

    //====================  RF Registers  ==================================

    REG32_RF_CONTROL                   RFControl;                        // 80
    REG32_LED_CONTROL                  LEDControl;                       // 84
    REG32_DIPSW                        DIPSW;                            // 88

    REG32_MAXIM_CONTROL                MaximControl;                     // 8c
    REG32_MAXIM_GAIN_CONTROL           MaximGainControl;                 // 90
    REG32_MAXIM_STATUS                 MaximStatus;                      // 94

    REG32_ADC_CONTROL                  ADCControl;                       // 98
    REG32_ADC_STATUS                   ADCStatus;                        // 9c

    REG32_DAC_CONTROL                  DACControl;                       // a0
    REG32_DAC_STATUS                   DACStatus;                        // a4

    REG32_SPI_INIT                     SPIInit;                          // a8
    REG32_SPI_STATUS                   SPIStatus;                        // ac

    REG32_MAXIM_SPI_DATA_IN            MaximSPIDataIn;                   // b0
    REG32_DAC_SPI_DATA_IN              DACSPIDataIn;                     // b4
    REG32_DAC_SPI_DATA_OUT             DACSPIDataOut;                    // b8

    REG32_RSSI_ADC_CONTROL             RSSIADCControl;                   // bc
    REG32_RSSI_ADC_DATA                RSSIADCData;                      // c0

    REG32_COMMON                       PDThreashold;                     // c4
    REG32_COMMON                       CSThreashold;                     // c8

    REG32_SDRAM_SLOT_INDICATOR         SlotIndicator;                    // cc
} SDR_REGISTER, *PSDR_REGISTER;

//////////////////////////////////////////////////////////////////////////

#define REG_TX_CONTROL                 0x40
#define REG_TX_FRAME_ADDRESS           0x44
#define REG_TX_FRAME_ID                0x48
#define REG_CURRENT_RX_ADDRESS         0x54

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _REG_CONFIGURATION_ENTRY{
    ULONG        RegAddress;
    ULONG        RegValue;

    ULONG        fNeedConfirm;
    ULONG        MaxConfirmTimes;

    ULONG        fDependent;
    ULONG        DependentRegAddress;
    ULONG        DependentRegBitMask;
    ULONG        DependentRegValue;
    ULONG        DependentTimeOuts;
}REG_CONFIGURATION_ENTRY, *PREG_CONFIGURATION_ENTRY;

extern REG_CONFIGURATION_ENTRY gc_PhyRegisterDefaultValueTable[];

extern REG_CONFIGURATION_ENTRY gc_RXEnableTable[];

extern REG_CONFIGURATION_ENTRY gc_TXEnableTable[];

extern ULONG gc_DefaultTableCnt;
extern ULONG g_RXEnableTableCnt;
extern ULONG g_TXEnableTableCnt;
//////////////////////////////////////////////////////////////////////////

typedef struct _PHY_REGISTER_MANAGER{
    PSDR_REGISTER           pRegs;             // Registers address

    PUCHAR                  pRegsPhyBase;      // Registers phy address
    PUCHAR                  pRegsSysBase;      // Registers base address
    ULONG                   RegsLength;        // Registers base length

    ULONG                   TxTransferLock;    // TX and Transfer can not go simultaneously, we need a lock to sync them
}PHY_REGISTER_MANAGER, *PPHY_REGISTER_MANAGER;

/************************************************************************/
/* SPIN_LOCK:
test, test-and-set  
*/
/************************************************************************/
#define SDR_PHY_ACQUIRE_TX_TRANSFER_LOCK(_pPhyManager)          \
{                                                               \
    ULONG Lock = 0;                                             \
L_SPIN_LOCK:                                                    \
    while ((_pPhyManager)->PhyRegManager.TxTransferLock && ((_pPhyManager)->fCanWork))\
    {                                                           \
        _asm { _asm pause }                                     \
    }                                                           \
    Lock = InterlockedExchange(&((_pPhyManager)->PhyRegManager.TxTransferLock), 1);\
    if (Lock == 1)/*Lock Failed*/                               \
    {                                                           \
        goto L_SPIN_LOCK;                                       \
    }                                                           \
}

#define SDR_PHY_FREE_TX_TRANSFER_LOCK(_pPhyManager)             \
    (_pPhyManager)->PhyRegManager.TxTransferLock = 0


_inline HRESULT
ConfigRegisters(IN PPHY_REGISTER_MANAGER pRegisterManager, IN PREG_CONFIGURATION_ENTRY pRegConfigurationEntry, IN ULONG Count)
{
    HRESULT         hRes            = S_OK;
    ULONG           WriteIndex      = 0;
    ULONG           ConfirmIndex    = 0;
    ULONG           fConfirmed      = FALSE;
    REG32_COMMON    CommonRegister  = {0};

    KdPrint(("==>ConfigReg"));

    for (WriteIndex = 0; WriteIndex < Count; WriteIndex++)
    {
        WRITE_REGISTER_ULONG((PULONG)(pRegisterManager->pRegsSysBase + pRegConfigurationEntry[WriteIndex].RegAddress), pRegConfigurationEntry[WriteIndex].RegValue);

        KdPrint(("WriteReg[%04x]=%08x", pRegConfigurationEntry[WriteIndex].RegAddress, pRegConfigurationEntry[WriteIndex].RegValue));

        if (pRegConfigurationEntry[WriteIndex].fNeedConfirm)
        {
            fConfirmed = FALSE;

            for (ConfirmIndex = 0; ConfirmIndex < pRegConfigurationEntry[WriteIndex].MaxConfirmTimes; ConfirmIndex++)
            {
                CommonRegister.Value = READ_REGISTER_ULONG((PULONG)(pRegisterManager->pRegsSysBase + pRegConfigurationEntry[WriteIndex].RegAddress));

                KdPrint((" CheckReadReg[%04x]=%08x", pRegConfigurationEntry[WriteIndex].RegAddress, CommonRegister.Value));
                if (CommonRegister.Value == pRegConfigurationEntry[WriteIndex].RegValue)
                {
                    fConfirmed = TRUE;
                    break;
                }
            }

            if (!fConfirmed)
            {
                hRes = S_FAIL;
                break;
            }
        }

        if (pRegConfigurationEntry[WriteIndex].fDependent)
        {
            do 
            {
                CommonRegister.Value = READ_REGISTER_ULONG((PULONG)(pRegisterManager->pRegsSysBase + pRegConfigurationEntry[WriteIndex].DependentRegAddress));
            } while (
                ((CommonRegister.Value & pRegConfigurationEntry[WriteIndex].DependentRegBitMask) != pRegConfigurationEntry[WriteIndex].DependentRegValue)
                &&
                ((pRegConfigurationEntry[WriteIndex].DependentTimeOuts--) > 0)
                );

            if (pRegConfigurationEntry[WriteIndex].DependentTimeOuts == 0)
            {
                hRes = S_FAIL;
                break;
            }

            KdPrint(("Dependent OK"));
        }
    }

    KdPrint(("<==ConfigReg"));

    return hRes;
}

/************************************************************************/


typedef struct _PHY_CHANNEL_SELECTOR
{
    ULONG       ChannelNumber;
    ULONG       Reg1, Reg2, Reg3; // To change the channel, we have to write 3 registers, see MAXIM manual
}PHY_CHANNEL_SELECTOR, *PPHY_CHANNEL_SELECTOR;

extern PHY_CHANNEL_SELECTOR     g_Phy_5GHz_ChannelSelectors[];
extern PHY_CHANNEL_SELECTOR     g_Phy_2dot4GHz_ChannelSelectors[];

extern ULONG                    g_Phy_5GHz_ChannelSelector_Count;
extern ULONG                    g_Phy_2dot4GHz_ChannelSelector_Count;


_inline HRESULT
SelectChannel(IN PPHY_REGISTER_MANAGER pRegisterManager, IN ULONG ChannelNumber)
{
    HRESULT hRes = S_OK;
    static REG_CONFIGURATION_ENTRY ChannelConfigEntry[] = 
    {
        {0xB0,         0X00000000,         FALSE,            0X00000000,           FALSE,          0X00000000,           0XFFFFFFFF,                 0X00000000,         0X0001FFFF},
        {0xA8,         0X00000001,         FALSE,            0X00000000,           TRUE,           0X000000AC,           0X00000001,                 0X00000000,         0X0001FFFF},
        {0xA8,         0X00000000,         FALSE,            0X00000000,           FALSE,          0X000000AC,           0X00000001,                 0X00000000,         0X01FFFFFF},
    };

    do 
    {
        if (ChannelNumber >= 1 && ChannelNumber <=14)
        {
            ChannelConfigEntry[0].RegValue = g_Phy_2dot4GHz_ChannelSelectors[ChannelNumber - 1].Reg1;
            ConfigRegisters(pRegisterManager, ChannelConfigEntry, 3);

            ChannelConfigEntry[0].RegValue = g_Phy_2dot4GHz_ChannelSelectors[ChannelNumber - 1].Reg2;
            ConfigRegisters(pRegisterManager, ChannelConfigEntry, 3);

            ChannelConfigEntry[0].RegValue = g_Phy_2dot4GHz_ChannelSelectors[ChannelNumber - 1].Reg3;
            ConfigRegisters(pRegisterManager, ChannelConfigEntry, 3);

            break;
        }
        else if (ChannelNumber >= 36 && ChannelNumber <= 161)
        {
            ChannelConfigEntry[0].RegValue = g_Phy_5GHz_ChannelSelectors[ChannelNumber - 36].Reg1;
            ConfigRegisters(pRegisterManager, ChannelConfigEntry, 3);

            ChannelConfigEntry[0].RegValue = g_Phy_5GHz_ChannelSelectors[ChannelNumber - 36].Reg2;
            ConfigRegisters(pRegisterManager, ChannelConfigEntry, 3);

            ChannelConfigEntry[0].RegValue = g_Phy_5GHz_ChannelSelectors[ChannelNumber - 36].Reg3;
            ConfigRegisters(pRegisterManager, ChannelConfigEntry, 3);

            break;
        }
        else
        {
            // Invalid Channel Number
            hRes = S_FAIL;
            break;
        }
    } while (FALSE);

    return hRes;
}

/************************************************************************/

HRESULT
InitializeRegisterManager(
    IN PPHY_REGISTER_MANAGER pRegisterManager, 
    IN PHYSICAL_ADDRESS      RegPhyAddressBase,
    IN ULONG                 RegPhyAddressLength
    );


HRESULT
CleanupRegisterManager(IN PPHY_REGISTER_MANAGER pRegisterManager);


HRESULT
ReadRegister(
    IN  PPHY_REGISTER_MANAGER pRegisterManager,
    IN  ULONG                 RegIndex,
    OUT PULONG                pRegValue
    );

HRESULT
WriteMaximSPI(
    IN PPHY_REGISTER_MANAGER pPhyRegManager, 
    IN ULONG        RegAddress, 
    IN ULONG        RegValue
    );

HRESULT
WriteDACSPI(
    IN PPHY_REGISTER_MANAGER pPhyRegManager, 
    IN ULONG        RegAddress, 
    IN ULONG        RegValue
    );

#endif  // _PCIE_REG_H_

