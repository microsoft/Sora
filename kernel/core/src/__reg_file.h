/*++
Copyright (c) Microsoft Corporation

Module Name: __reg_file.h

Abstract: This header file defines registers file of Sora radio control board.

History: 
          25/May/2009: Created by senxiang
--*/

#ifndef _SORA_REGMAN2_
#define _SORA_REGMAN2_
#pragma once
//#include "_sora_regman.h"

#define RF_REGISTER_SECTION_OFFSET          0x90

#define __RADIO_REG_SEG_SIZE                0x100
#define __RADIO_SEG_REG_NUM                 (__RADIO_REG_SEG_SIZE / sizeof(s_uint32))

#define ConfigRegisters2                  __ConfigRegistersUnsafe

/* register types */
typedef union ___REG32_COMMON{
    struct{
        s_uint32      Reserved              : 32;
    }Bits;
    s_uint32 Value;
}__REG32_COMMON, *__PREG32_COMMON;


typedef union __REG32_HW_CONTROL
{
    struct{
        s_uint32    HWReset:    1;
        s_uint32    Reserved:   31;
    }Bits;
    s_uint32        Value;
} __REG32_HW_CONTROL, *__REG32_PHW_CONTROL;

typedef union __REG32_TRANS_CTRL
{
    struct {
        s_uint32    TransferInit: 1;
        s_uint32    TransferDone: 1;
        s_uint32    Reserved:   30;
    }Bits;
    s_uint32        Value;
}__REG32_TRANS_CTRL, *__PREG32_TRANS_CTRL;

typedef union __REG32_RX_CTRL
{
    struct{
        s_uint32    RXEnable: 1;
        s_uint32    Reserved: 31;
    }Bits;
    s_uint32        Value;
}__REG32_RX_CTRL, *__PREG32_RX_CTRL;

typedef union __REG32_TX_CTRL
{
    struct{
        s_uint32    TXInit: 1;
        s_uint32    TXDone: 1;
		s_uint32	Reserved: 14;
        s_uint32    TXMask: 16;
    }Bits;
    s_uint32        Value;
}__REG32_TX_CTRL, *__PREG32_TX_CTRL;

typedef union __REG32_RADIO_STATUS
{
    struct {
        s_uint32    RadioAlive : 1;
        s_uint32    Reserved   : 31;
    } Bits;
    s_uint32        Value;
}__REG32_RADIO_STATUS, *__PREG32_RADIO_STATUS;


typedef union __REG32_RF_CMD_ADDR
{
    struct {
        s_uint32    Addr    : 7;
        s_uint32    Cmd     : 1; //0:WR, 1:RD
        s_uint32    RdDone  : 1;
        s_uint32    Reserved: 23;
    } Bits;
    s_uint32    Value;
}__REG32_RF_CMD_ADDR, *__PREG32_RF_CMD_ADDR;

typedef struct __SORA_RADIO_CHANNEL_REGS
{
    __REG32_COMMON      RFRegValueIn;   //0xNC0
    __REG32_RF_CMD_ADDR RFRegOpInst;    //0xNC4
    __REG32_COMMON      RFRegValueOut;  //0xNC8
    __REG32_COMMON      Reserved[13];   //0xNCC~0xNFF
} __SORA_RADIO_CHANNEL_REGS, *__PSORA_RADIO_CHANNEL_REGS;

CCASSERT(sizeof(__SORA_RADIO_CHANNEL_REGS) == 0x40)

typedef union __REG32_VERSION
{
    BYTE        SubVersion8[4];
    s_uint32    Version32;
} __REG32_VERSION, *__PREG32_VERSION;


typedef struct __SORA_RADIO_REGS
{                                               //offset (N=1-8)
	__REG32_COMMON			Pad1[12];			//0xN00~0xN2F

	__REG32_COMMON			TransferErrorCount;			//0xN30 transfer error count
	__REG32_COMMON			TransferCountChecksum;		//0xN34 transfer count 16bit (128bit), checksum 16bit
	__REG32_COMMON			TXErrorCount;				//0xN38 TX error count
	__REG32_COMMON			TXCountChecksum; 			//0xN3C TX count 16bit (16bit), checksum 16bit

	__REG32_COMMON			Pad2[5];			//0xN40~0xN53
	
    __REG32_RADIO_STATUS    RadioStatus;        //0xN54
    __REG32_COMMON          RadioID;            //0xN58
    __REG32_COMMON          RoundTripLatency;   //0xN5C
    __REG32_COMMON          TransferDuration;   //0xN60
    __REG32_RX_CTRL         RXControl;          //0xN64
    __REG32_TRANS_CTRL      TransferControl;    //0xN68
    __REG32_COMMON          TransferMask;       //0xN6C
    __REG32_COMMON          RXBufAddrL;         //0xN70
    __REG32_COMMON          RXBufAddrH;         //0xN74

    __REG32_COMMON          TXAddr;             //0xN78
    __REG32_COMMON          TransferReset;      //0xN7C
    __REG32_COMMON          TXReserved;         //0xN80
    __REG32_TX_CTRL         TXControl;          //0xN84
    __REG32_COMMON          TransferSrcAddrL;   //0xN88
    __REG32_COMMON          TransferSrcAddrH;   //0xN8C
    
    __REG32_COMMON          TXSize;             //0xN90
    __REG32_COMMON          RXBufSize;          //0xN94

	__REG32_COMMON			TransferChecksum;	//0xN98	LSB 16-bit
	__REG32_COMMON			TXChecksum;			//0xN9C	LSB 16-bit
	__REG32_COMMON			TXTiming;			//0xNA0

    s_uint32                Reserved[7];       //0xNB0~0xNBF

#pragma region "    Special Radio Registers"
    __SORA_RADIO_CHANNEL_REGS   RadioChannelRegs;   //0xNC0~0xNFF
#pragma warning (default: 4201)
#pragma endregion
}__SORA_RADIO_REGS, *__PSORA_RADIO_REGS;

CCASSERT(sizeof(__SORA_RADIO_REGS) == __RADIO_REG_SEG_SIZE)

typedef union __REG32_HWSTATUS
{
    struct{
        s_uint32  DDR2InitDone              : 1;
        s_uint32  PCIELinkup                : 1;
        s_uint32  Reserved                  : 30;
    }Bits;
    s_uint32      Value;
} __REG32_HWSTATUS, *__PREG32_HWSTATUS;

typedef struct __SORA_REGISTERS
{
    //Reg Type (bits meaning)-- Name            --PHY Address Offset    --default value
#pragma region "    System Registers"
    __REG32_COMMON              Pad1[14];
    __REG32_VERSION             FirmwareVersion;//0x038
    __REG32_HWSTATUS            HWStatus;       //0x03C
    __REG32_COMMON              LinkStatus;     //0x040
    __REG32_COMMON              LinkControl;    //0x044
    __REG32_HW_CONTROL          HWControl;      //0x048
    __REG32_COMMON              PCIeTXBurstSize;//0x04C
    __REG32_COMMON              PCIeRxBlockSize;//0x050
    __REG32_COMMON              Reserved1[11];      //0x054~0x07F
    __REG32_COMMON              DebugRegSect1[32];  //0x080-0x0FF;

#pragma endregion 

    __SORA_RADIO_REGS           RadioRegs[MAX_RADIO_NUMBER];
    __REG32_COMMON              DebugRegSect2[__RADIO_SEG_REG_NUM * 7];

}__SORA_REGISTERS, *__PSORA_REGISTERS;


CCASSERT( sizeof(__SORA_REGISTERS) == 0x1000)

_inline HRESULT
__ConfigRegistersUnsafe(
                IN __PHW_REGISTER_FILE pRegisterManager, 
                IN __PREG_CONFIGURATION_ENTRY pRegConfigurationEntry, 
                IN ULONG Count)
{
    HRESULT         hRes            = S_OK;
    ULONG           WriteIndex      = 0;
    ULONG           ConfirmIndex    = 0;
    ULONG           fConfirmed      = FALSE;
    __REG32_COMMON    CommonRegister  = {0};

    //KdPrint(("==>ConfigReg"));

    for (WriteIndex = 0; WriteIndex < Count; WriteIndex++)
    {
        WRITE_REGISTER_ULONG(
            (PULONG)((PUCHAR)(pRegisterManager->pRadioRegs) + pRegConfigurationEntry[WriteIndex].RegAddress), 
            pRegConfigurationEntry[WriteIndex].RegValue);

        //KdPrint(("WriteReg[%04x]=%08x", pRegConfigurationEntry[WriteIndex].RegAddress, pRegConfigurationEntry[WriteIndex].RegValue));

        if (pRegConfigurationEntry[WriteIndex].fNeedConfirm)
        {
            fConfirmed = FALSE;

            for (ConfirmIndex = 0; 
                ConfirmIndex < pRegConfigurationEntry[WriteIndex].MaxConfirmTimes; 
                ConfirmIndex++)
            {
                CommonRegister.Value = 
                    READ_REGISTER_ULONG(
                        (PULONG)((PUCHAR)(pRegisterManager->pRadioRegs) + 
                        pRegConfigurationEntry[WriteIndex].RegAddress));

                //KdPrint((" CheckReadReg[%04x]=%08x", pRegConfigurationEntry[WriteIndex].RegAddress, CommonRegister.Value));
                if (CommonRegister.Value == pRegConfigurationEntry[WriteIndex].RegValue)
                {
                    fConfirmed = TRUE;
                    break;
                }
            }

            if (!fConfirmed)
            {
                hRes = E_REG_WRITE_FAIL;
                break;
            }
        }

        if (pRegConfigurationEntry[WriteIndex].fDependent)
        {
            do 
            {
                CommonRegister.Value = 
                    READ_REGISTER_ULONG(
                        (PULONG)((PUCHAR)(pRegisterManager->pRadioRegs) + 
                        pRegConfigurationEntry[WriteIndex].DependentRegAddress));
                
                pRegConfigurationEntry[WriteIndex].DependentTimeOuts--;
            } while (
                ((CommonRegister.Value & pRegConfigurationEntry[WriteIndex].DependentRegBitMask) 
                    != pRegConfigurationEntry[WriteIndex].DependentRegValue)
                &&
                (pRegConfigurationEntry[WriteIndex].DependentTimeOuts > 0)
                );

            if (pRegConfigurationEntry[WriteIndex].DependentTimeOuts <= 0)
            {
                hRes = E_REG_WRITE_DEP_FAIL;
                break;
            }
        }
    }

    return hRes;
}

#endif
