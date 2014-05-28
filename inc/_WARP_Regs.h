/*++
Copyright (c) Microsoft Corporation

Module Name: WARP radio registers layout and utility

Abstract: 
    This header file defines WARP register layout. If other types of 
    radio front end is used, users should define their own register 
    layout according to radio front end specification. 

--*/

#ifndef _WARP_REGS_H
#define _WARP_REGS_H
#pragma once

typedef union __REG32_RF_CTRL
{
    struct{
        s_uint32    ANTSW1      : 1;
        s_uint32    ANTSW2      : 1;
        s_uint32    Reserved    : 30;
    } Bits;
    s_uint32        Value;
}__REG32_RF_CTRL, *__PREG32_RF_CTRL;

typedef union __REG32_LED_CTRL
{
    struct {
        s_uint32    LED1 : 1;
        s_uint32    LED2 : 1;
        s_uint32    LED3 : 1;
        s_uint32    Reserved : 29;
    } Bits;
    s_uint32        Value;
}__REG32_LED_CTRL, *__PREG32_LED_CTRL;

typedef union ___REG32_DIPSW{
    struct{
        s_uint32  DIPSW1                    : 1;
        s_uint32  DIPSW2                    : 1;
        s_uint32  DIPSW3                    : 1;
        s_uint32  DIPSW4                    : 1;
        s_uint32  Reserved                  : 28;
    }Bits;
    s_uint32      Value;
}__REG32_DIPSW, *__PREG32_DIPSW;

typedef union ___REG32_MAXIM_CONTROL{
    struct{
        s_uint32  RADIO_SHDN                : 1; //can't be 1
        s_uint32  RADIO_RST                 : 1; //can't be 1
        s_uint32  RADIO_RXHP                : 1;
        s_uint32  Reserved                  : 29;
    }Bits;
    s_uint32      Value;
}__REG32_MAXIM_CONTROL, *__PREG32_MAXIM_CONTROL;

typedef union ___REG32_MAXIM_GAIN_SETTING{
    struct{
        s_uint32  TX_Gain                : 6;
        s_uint32  B7TOB16                : 10;
        s_uint32  RX_Gain                : 7;
        s_uint32  B24TOB32               : 9;
    }Bits;
    s_uint32      Value;
}__REG32_MAXIM_GAIN_SETTING, *__PREG32_MAXIM_GAIN_SETTING;

typedef union ___REG32_MAXIM_STATUS{
    struct{
        s_uint32  RADIO_LD                  : 1;
        s_uint32  Reserved                  : 31;
    }Bits;
    s_uint32      Value;
}__REG32_MAXIM_STATUS, *__PREG32_MAXIM_STATUS;

typedef union ___REG32_ADC_CONTROL{
    struct{
        s_uint32  ADC_PDWN_A                : 1;
        s_uint32  ADC_PDWN_B                : 1;
        s_uint32  ADC_DCS                   : 1;
        s_uint32  ADC_DFS                   : 1;
        s_uint32  Reserved                  : 28;
    }Bits;
    s_uint32      Value;
}__REG32_ADC_CONTROL, *__PREG32_ADC_CONTROL;

typedef union ___REG32_ADC_STATUS{
    struct{
        s_uint32  ADC_OTR_A                 : 1;
        s_uint32  ADC_OTR_B                 : 1;
        s_uint32  Reserved                  : 30;
    }Bits;
    s_uint32      Value;
}__REG32_ADC_STATUS, *__PREG32_ADC_STATUS;

typedef union ___REG32_DAC_CONTROL{
    struct{
        s_uint32  DAC_RESET                 : 1;
        s_uint32  Reserved                  : 31;
    }Bits;
    s_uint32      Value;
}__REG32_DAC_CONTROL, *__PREG32_DAC_CONTROL;

typedef union ___REG32_DAC_STATUS{
    struct{
        s_uint32  DAC_PLL_LOCK              : 1;
        s_uint32  Reserved                  : 31;
    }Bits;
    s_uint32      Value;
}__REG32_DAC_STATUS, *__PREG32_DAC_STATUS;

typedef union ___REG32_SPI_INIT{
    struct{
        s_uint32  MAXIM_SPI_INIT            : 1;
        s_uint32  MAXIM_SPI_DONE            : 1;
        s_uint32  Reserved                  : 30;
    }Bits;
    s_uint32      Value;
}__REG32_SPI_INIT, *__PREG32_SPI_INIT;

typedef union ___REG32_SPI_STATUS{
    struct{
        s_uint32  DAC_SPI_INIT              : 1;
        s_uint32  DAC_SPI_DONE              : 1;
        s_uint32  Reserved                  : 30;
    }Bits;
    s_uint32      Value;
}__REG32_SPI_STATUS, *__PREG32_SPI_STATUS;

typedef union ___REG32_MAXIM_SPI_DATA_IN{
    struct{
        s_uint32  A0TOA3                    : 4;
        s_uint32  D0TOD13                   : 14;
        s_uint32  Reserved                  : 14;
    }Bits;
    s_uint32      Value;
}__REG32_MAXIM_SPI_DATA_IN, *__PREG32_MAXIM_SPI_DATA_IN;

typedef union ___REG32_DAC_SPI_DATA_IN{
    struct{
        s_uint32  RegisterUCHAR              : 8;
        s_uint32  InstructionUCHAR           : 8;
        s_uint32  Reserved                  : 16;
    }Bits;
    s_uint32      Value;
}__REG32_DAC_SPI_DATA_IN, *__PREG32_DAC_SPI_DATA_IN;

typedef union ___REG32_DAC_SPI_DATA_OUT{
    struct{
        s_uint32  RegisterUCHAR              : 8;
        s_uint32  Reserved                   : 24;
    }Bits;
    s_uint32      Value;
}__REG32_DAC_SPI_DATA_OUT, *__PREG32_DAC_SPI_DATA_OUT;

typedef union ___REG32_RSSI_ADC_CONTROL{
    struct{
        s_uint32  RSSI_ADC_Clamp            : 1;
        s_uint32  RSSI_ADC_Sleep            : 1;
        s_uint32  RSSI_ADC_HIZ              : 1;
        s_uint32  Reserved                  : 29;
    }Bits;
    s_uint32      Value;
}__REG32_RSSI_ADC_CONTROL, *__PREG32_RSSI_ADC_CONTROL;

typedef union ___REG32_RSSI_ADC_DATA{
    struct{
        s_uint32  RSSI_ADC_D0TOD9           : 10;
        s_uint32  Reserved1                 : 6;
        s_uint32  RSSI_OTR                  : 1;
        s_uint32  Reserved2                 : 15;
    }Bits;
    s_uint32      Value;
}__REG32_RSSI_ADC_DATA, *__PREG32_RSSI_ADC_DATA;

typedef struct __WARP_REGS
{
    //Offset = RF_REGISTER_SECTION_OFFSET(0xN90) + field_Offset
    __REG32_RF_CTRL             RFControl;          //0xN90
    __REG32_LED_CTRL            LEDControl;         //0xN94
    __REG32_DIPSW               DIPSW;              //0xN98
    __REG32_MAXIM_CONTROL       MaximControl;       //0xN9c
    __REG32_MAXIM_GAIN_SETTING  MaximGainControl;   //0xNA0
    __REG32_MAXIM_STATUS        MaximStatus;        //0xNA4
    __REG32_ADC_CONTROL         ADCControl;         //0xNA8
    __REG32_ADC_STATUS          ADCStatus;          //0xNAC
    __REG32_DAC_CONTROL         DACControl;         //0xNB0
    __REG32_DAC_STATUS          DACStatus;          //0xNB4
    __REG32_SPI_INIT            SPIInit;            //0xNB8
    __REG32_SPI_STATUS          SPIStatus;          //0xNBC
    __REG32_MAXIM_SPI_DATA_IN   MaximSPIDataIn;     //0xNC0
    __REG32_DAC_SPI_DATA_IN     DACSPIDataIn;       //0xNC4
    __REG32_DAC_SPI_DATA_OUT    DACSPIDataOut;      //0xNC8
    __REG32_RSSI_ADC_CONTROL    RSSIADCControl;     //0xNCC
    __REG32_RSSI_ADC_DATA       RSSIADCData;        //0xND0
    s_uint32                    ReservedEndSp[11];  //0xND4~0xNFF
} WARP_REGS, *PWARP_REGS;


#endif
