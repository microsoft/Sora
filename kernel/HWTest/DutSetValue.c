#include "hwtest_miniport.h"

HRESULT DutSetValue(PSORA_RADIO Radio, ULONG Value, ULONG code)
{
    switch(code)
    {
    case DUT_IO_CODE(CMD_SAMPLE_RATE):
        DEBUGP(MP_INFO, ("[HWT] Set Sample Rate %d M\n", Value));
        SoraHwSetSampleClock(Radio, Value * 1000 * 1000);
        break;
    case DUT_IO_CODE(CMD_RX_GAIN):
        DEBUGP(MP_INFO, ("[HWT] Set RX Gain 0x%08x\n", Value));
        SoraHwSetRXVGA1(Radio, Value); 
        break;
    case DUT_IO_CODE(CMD_TX_GAIN):
        DEBUGP(MP_INFO, ("[HWT] Set TX Gain 0x%08x\n", Value));
        SoraHwSetTXVGA1(Radio, Value);
        break;
    case DUT_IO_CODE(CMD_CENTRAL_FREQ):
        DEBUGP(MP_INFO, ("[HWT] Set Central Frequency %u MHz\n", Value));
        SoraHwSetCentralFreq(Radio, Value * 1000, 0);
        break;
    case DUT_IO_CODE(CMD_FREQ_OFFSET):
        DEBUGP(MP_INFO, ("[HWT] Set Frequency compenstation %l Hz\n", (LONG)Value));
        SoraHwSetFreqCompensation(Radio, (LONG)Value);
        break;
    case DUT_IO_CODE(CMD_RX_PA):
        DEBUGP(MP_INFO, ("[HWT] Set RX PA 0x%04x 1/256 db\n", Value));
        SoraHwSetRXPA(Radio, Value);
        break;
    }
    return S_OK;
}