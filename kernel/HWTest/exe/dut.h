#ifndef _DUT_H
#define _DUT_H

#ifdef __cplusplus
 extern "C" {
#endif 
#include <stdio.h>
#include <windows.h>
#include <winioctl.h> //CTL_CODE

#include <strsafe.h>

#define PRIVATE_FEATURE

#define IOCTL_TEST_HARDWARE   \
    CTL_CODE(FILE_DEVICE_NETWORK, 0x201, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define DEVNAME "\\\\.\\HwTest"

#define Err     printf
#define Wrn     printf

#ifdef __cplusplus
 } //end extern "C" {
#endif 

typedef enum __DUT_OPT
{
    DUT_OPT_TABLE_BEGIN = -1,
    /* int options */
    DUT_OPT_INT_START,
#ifdef PRIVATE_FEATURE
    DUT_OPT_REG_OFFSET = DUT_OPT_INT_START, 
    DUT_OPT_VALUE,
#else
    DUT_OPT_VALUE = DUT_OPT_INT_START, 
#endif
    DUT_OPT_SIG_ID, 
    DUT_OPT_RADIO_NO,
    DUT_OPT_INT_END,
    DUT_OPT_FILE_NAME = DUT_OPT_INT_END,    
    DUT_OPT_TABLE_SIZE,   	
} DUT_OPT;

#define MAX_COMMAND_LENGTH              256
#define MAX_OUTPUT_LENGTH               1024 * 1024 * 16 //10M

void DutReadRegister(HANDLE hDevice, ULONG Offset);
void DutWriteRegister(HANDLE hDevice, ULONG Offset, ULONG Value);
void DutStartRadio(HANDLE hDevice, ULONG RadioNo);
void DutStopRadio(HANDLE hDevice, ULONG RadioNo);
void DutDump(HANDLE hDevice, ULONG* RadioNo, ULONG Count, PCSTR szDumpFileName);
void DutGetInfo(HANDLE hDevice);
void DutTransferSignals(HANDLE hDevice, PCSTR szSigFileName);
void DutTxSignals(HANDLE hDevice, ULONG RadioNo, ULONG sigID, ULONG Times);
void DutTxDone(HANDLE hDevice, ULONG sigID);
void DutSetTxGain(HANDLE hDevice, ULONG RadioNo, ULONG Gain);
void DutSetRxGain(HANDLE hDevice, ULONG RadioNo, ULONG Gain);
void DutSetCentralFreq(HANDLE hDevice, ULONG RadioNo, ULONG Freq);
void DutSetFreqOffset(HANDLE hDevice, ULONG RadioNo, LONG Freq);
void DutStopTX(HANDLE hDevice);
void DutGetRxBuffer(HANDLE hDevice, ULONG RadioNo);
HANDLE GetDeviceHandle(LPCTSTR szDeviceName);
void DutGetFwVersion(HANDLE hDevice);
void DutMimoTx(HANDLE hDevice,ULONG * RadioNo,ULONG * sigID,ULONG Count,ULONG Times);

PVOID __MALLOC(size_t size);
void __FREE(PVOID p);
#endif //_DUT_H