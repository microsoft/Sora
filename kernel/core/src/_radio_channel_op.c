/*++
Copyright (c) Microsoft Corporation

Module Name: _WARP_control.c

Abstract: WARP radio front end control functions.

History: 
          25/May/2009: Created by senxiang
--*/

#include "sora.h"

#include "__reg_file.h"

/*++
Write Radio Frontend (RF) register
--*/
VOID _SoraRadioWriteRFReg(PSORA_RADIO pRadio, s_uint32 addr, s_uint32 value)
{
    __REG32_RF_CMD_ADDR CmdAddr;
    __PSORA_RADIO_REGS pRadioRegs = pRadio->__ctrl_reg.pRadioRegs;

    ASSERT((addr < 0x80));

    CmdAddr.Value  = 0;
    CmdAddr.Bits.Addr   = addr & 0x7F;
    CmdAddr.Bits.Cmd    = 0; //write op
    
    WRITE_REGISTER_ULONG((PULONG)&pRadioRegs->RadioChannelRegs.RFRegValueIn, value);
    WRITE_REGISTER_ULONG((PULONG)&pRadioRegs->RadioChannelRegs.RFRegOpInst, CmdAddr.Value);
}

/*++
Read Radio Frontend (RF) register.
--*/
HRESULT _SoraRadioReadRFReg(PSORA_RADIO pRadio, s_uint32 addr, OUT s_uint32 *pValue)
{
    int timeout = 100;
    HRESULT hr = S_OK;
    __REG32_RF_CMD_ADDR CmdAddr;
    __PSORA_RADIO_REGS pRadioRegs = pRadio->__ctrl_reg.pRadioRegs;

    ASSERT((addr < 0x80));
    CmdAddr.Value  = 0;
    CmdAddr.Bits.Addr   = addr & 0x7F;
    CmdAddr.Bits.Cmd    = 1; //read op
    WRITE_REGISTER_ULONG((PULONG)&pRadioRegs->RadioChannelRegs.RFRegOpInst, CmdAddr.Value);

    do {
        timeout--;
        if (timeout < 0)
        {
            hr = E_FAIL;
            break;
        }
        CmdAddr.Value = READ_REGISTER_ULONG((PULONG)&pRadioRegs->RadioChannelRegs.RFRegOpInst);
    } while(!CmdAddr.Bits.RdDone);

    if (SUCCEEDED(hr))
    {
        *pValue = READ_REGISTER_ULONG((PULONG)&pRadioRegs->RadioChannelRegs.RFRegValueOut);
    }

    return hr;
}

VOID SoraAbsRFStart(PSORA_RADIO pRadio)
{
    _SoraRadioWriteRFReg(pRadio, 0x0, 0x1); //assert RadioEnable bit in RadioControl Register
    _SoraRadioWriteRFReg(pRadio, 0x4, 0x2); // light the 2nd LED as radio start/reset flag
    _SoraRadioWriteRFReg(pRadio, 0x5, pRadio->__radio_no + 1); // Select Antenna i
}

VOID SORAAPI SoraAbsRFReset(PSORA_RADIO pRadio)
{
    _SoraRadioWriteRFReg(pRadio, 0x0, 0x2);	// assert RadioReset bit in RadioControl Register
    _SoraRadioWriteRFReg(pRadio, 0x4, 0x2); // light the 2nd LED as radio start/reset flag
    _SoraRadioWriteRFReg(pRadio, 0x5, pRadio->__radio_no + 1); // Select Antenna i
}


VOID SORAAPI SoraHwSetTXVGA1(PSORA_RADIO pRadio, ULONG db_1_256)
{
    _SoraRadioWriteRFReg(pRadio, 0xC, db_1_256);

	pRadio->__uTxGain = db_1_256;
}

VOID SORAAPI SoraHwSetRXVGA1(PSORA_RADIO pRadio, ULONG db_1_256)
{
    _SoraRadioWriteRFReg(pRadio, 0x12, db_1_256);
	
	pRadio->__uRxGain = db_1_256;
}

VOID SORAAPI SoraHwSetRXPA(PSORA_RADIO pRadio, ULONG db_1_256)
{
    _SoraRadioWriteRFReg(pRadio, 0x11, db_1_256);

	pRadio->__uRxPa = db_1_256;
}

