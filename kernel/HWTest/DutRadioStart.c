#include "hwtest_miniport.h"

HRESULT DutStartRadio(PSORA_RADIO Radio)
{
    HRESULT hr;
    do
    {
        hr = SoraRadioStart(
                Radio, 
                SORA_RADIO_DEFAULT_RX_GAIN,
                SORA_RADIO_DEFAULT_TX_GAIN, 
                NULL);
        FAILED_BREAK(hr);
        
        SoraHwSetTXVGA1(Radio, SORA_RADIO_DEFAULT_TX_GAIN);
        SoraHwSetRXVGA1(Radio, SORA_RADIO_DEFAULT_RX_GAIN);

        SoraHwSetCentralFreq(Radio, HWT_DEFAULT_CENTRAL_FREQ, 0);

        SORA_HW_ENABLE_RX(Radio);
    } while(FALSE);

    return hr;
}

void SoraStopRadio2(PSORA_RADIO);

HRESULT DutStopRadio(PSORA_RADIO Radio)
{
    SoraStopRadio2(Radio);
    return S_OK;
}