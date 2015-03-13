/*++
Copyright (c) Microsoft Corporation

Module Name: Sora Hardware Abstract Layer

Abstract: This header file declares functions to operate Sora Hardware.

--*/

#ifndef _HW_OP_H
#define _HW_OP_H
#pragma once

#define SORA_HW_VERSION 0x01000200
#define SORA_HW_LEGACY  0x01000200

#pragma region "new primitives"

VOID SORAAPI SoraAbsRFStart(PSORA_RADIO pRadio);
VOID SORAAPI SoraAbsRFReset(PSORA_RADIO pRadio);

// Read radio status, 0: power off, 1: power on.
ULONG SORAAPI SoraHwGetRadioStatus(PSORA_RADIO pRadio);

// Set LED on/off, 1: ON, 0: OFF. bit 0~31 delegates 32 LED indicators.
VOID SoraHwSetLEDMask(PSORA_RADIO pRadio, ULONG OnOffMask);

// Get LED on/off mask, 1: ON, 0: OFF.
ULONG SoraHwGetLEDMask(PSORA_RADIO pRadio);

// Select Antennas using mask, 1: selected, 0: discarded
VOID SoraHwSelectAntenna(PSORA_RADIO pRadio, ULONG Mask);

// Get antennas mask
ULONG SoraHwGetAntennaMask(PSORA_RADIO pRadio);

// Set sample clock with Hz as unit.
VOID SoraHwSetSampleClock(PSORA_RADIO pRadio, ULONG Hz);

// Get sample clock with Hz as unit.
ULONG SoraHwGetSampleClock(PSORA_RADIO pRadio);

// Set central freqency with kHz coare frequency + finer grain Hz
VOID SoraHwSetCentralFreq(PSORA_RADIO pRadio, ULONG kHzCoarse, LONG HzFine);

// Get central frequency
VOID SoraHwGetCentralFreq(PSORA_RADIO pRadio, PULONG kHzCoare, PLONG HzFinerGrain);

// Set compensation around central frequency
VOID SoraHwSetFreqCompensation(PSORA_RADIO pRadio, LONG lFreq);

// Get current compensation
LONG SoraHwGetFreqCompensation(PSORA_RADIO pRadio);

// Get frequency set status. 1: set success, 0: set failure
ULONG SoraHwReadFreqSetStatus(PSORA_RADIO pRadio);

// Set filter bandwidth in Hz
VOID SoraHwSetFilterBandwidth(PSORA_RADIO pRadio, ULONG HzBandwidth);

// Get filter bandwidth in Hz
ULONG SoraHwGetFilterBandwidth(PSORA_RADIO pRadio);

// Set TX VGA1 in 1/256 db
VOID SoraHwSetTXVGA1(PSORA_RADIO pRadio, ULONG uGain);

// Set TX VGA2 in 1/256 db
VOID SoraHwSetTXVGA2(PSORA_RADIO pRadio, ULONG db_1_256);

// Set TX PA1 in 1/256 db
VOID SoraHwSetTXPA1(PSORA_RADIO pRadio, ULONG db_1_256);

// Set TX PA2 in 1/256 db
VOID SoraHwSetTXPA2(PSORA_RADIO pRadio, ULONG db_1_256);

// set RX LNA in 1/256 db
VOID SoraHwSetRXLNA(PSORA_RADIO pRadio, ULONG db_1_256);

// set RX PA in 1/256 db
VOID SoraHwSetRXPA(PSORA_RADIO pRadio, ULONG uGain);

// set RX VGA1 in 1/256 db
VOID SORAAPI SoraHwSetRXVGA1(PSORA_RADIO pRadio, ULONG uGain);

// set RX VGA2 in 1/256 db
VOID SoraHwSetRXVGA2(PSORA_RADIO pRadio, ULONG db_1_256);

#pragma region

HRESULT SORAAPI SoraHwHeavyRestart(PSORA_RADIO pRadio);

VOID SORAAPI SORA_HW_READ_RADIO_POWER_STATUS(OUT PSORA_RADIO pRadio);

VOID SORAAPI SoraHwPrintDbgRegs(HANDLE TransferObj);
VOID SORAAPI SoraHwPrintRadioRegs(PSORA_RADIO pRadio);

void SoraHwSetTxBufferRegs(PSORA_RADIO pRadio, PPACKET_BASE pPacket);

HRESULT SoraHwSyncTx(PSORA_RADIO pRadio, ULONG mask);

//
// APIs for RCB control
//

VOID 
SORAAPI 
SORA_HW_ENABLE_RX ( PSORA_RADIO pRadio );


VOID 
SORAAPI 
SORA_HW_STOP_RX ( PSORA_RADIO pRadio );


HRESULT 
SORAAPI 
SORA_HW_TX_TRANSFER ( IN HANDLE TransferObj, 
                      IN PPACKET_BASE pPacket );

HRESULT
SORAAPI
SORA_HW_FAST_TX_TRANSFER(HANDLE TransferObj, PTX_DESC pTxDesc);

HRESULT 
SORAAPI 
SORA_HW_BEGIN_TX( PSORA_RADIO  pRadio, 
                  PPACKET_BASE pPacket );

HRESULT 
SORAAPI 
SORA_HW_FAST_TX ( PSORA_RADIO pRadio, 
                  PTX_DESC    pTxDesc );

//void SORAAPI SoraHwWriteRadioRF(PSORA_RADIO pRadio, ULONG RadioRFRegOffset, ULONG Value);
//ULONG SORAAPI SoraHwReadRadioRF(PSORA_RADIO pRadio, ULONG RadioRFRegOffset);

//#define SORA_HW_WRITE_RF_REGISTER32(pRadio, RFRegistersStruct, RFRegistersField, Value) \
//    do \
//    { \
//        UPOINTER Offset = (UPOINTER)(&((RFRegistersStruct*)NULL)->RFRegistersField); \
//        Offset >>= 2; \
//        SoraHwWriteRadioRF(pRadio, Offset, Value); \
//    } while(FALSE);
//
//#define SORA_HW_READ_RF_REGISTER32(pRadio, RFRegistersStruct, RFRegistersField, Value) \
//    do { \
//        UPOINTER Offset = (UPOINTER)(&((RFRegistersStruct*)NULL)->RFRegistersField) >> 2; \
//        Value = SoraHwReadRadioRF(pRadio, Offset); \
//    }while(FALSE);

#endif
