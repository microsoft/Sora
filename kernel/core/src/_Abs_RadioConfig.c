/*++
Copyright (c) Microsoft Corporation

Module Name: _WARP_control.c

Abstract: WARP radio front end control functions.

History: 
          25/May/2009: Created by senxiang
--*/

#include "sora.h"

#include "__reg_file.h"

#ifdef __HW_V11

/*++
Write Radio Frontend (RF) register
--*/
VOID _SoraRadioWriteRFReg(PSORA_RADIO pRadio, s_uint32 addr, s_uint32 value)
{
    __REG32_RF_CMD_ADDR CmdAddr;
    __PSORA_RADIO_REGS pRadioRegs = pRadio->__ctrl_reg.pRadioRegs;

	if (addr > 0x3f) 
		return;
    ASSERT((addr < 0x80) && (addr % 4 == 0));

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

	if (addr > 0x3f)
		return;
    ASSERT((addr < 0x80) && (addr % 4 == 0));
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
#endif