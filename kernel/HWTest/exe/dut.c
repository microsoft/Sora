#include "dut.h"
#include "args.h"
#define USER_MODE
#include "..\\hwtest_ioctrl.h"
#include <assert.h>

#define DUMP_BUFFER_DELAY_TIMESTAMP 56*10
#define DUMP_BUFFER_SIZE			16*1024*1024

HANDLE GetDeviceHandle(LPCTSTR szDeviceName)
{
    HANDLE hDrv = CreateFile ( szDeviceName, 
        GENERIC_READ | GENERIC_WRITE, 
        0,  
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL );
    return hDrv;
}

void DutReadRegister(HANDLE hDevice, ULONG Offset)
{
    ULONG OutLen = sizeof(DEV_RET) + sizeof(ULONG);
    PDEV_RET pOut = (PDEV_RET)__MALLOC(OutLen);
    ULONG nWritten = 0;
    BOOL ret = DeviceIoControl ( hDevice, 
                    DUT_IO_CODE(CMD_RD_REG),
                    &Offset,
                    sizeof(Offset),
                    pOut,
                    OutLen,
                    &nWritten,
                    NULL);
    if (nWritten != OutLen)
    {
        printf("Read register failed\n");
    }
    else
    {
        printf("Ret: 0x%08x\n", pOut->hResult);
        printf("Content: 0x%08x\n", *(PULONG)(pOut->data));
    }
    __FREE(pOut);
}

void DutWriteRegister(HANDLE hDevice, ULONG Offset, ULONG Value)
{
    OFFSET_VALUE Input;
    PDEV_RET pOut = (PDEV_RET)__MALLOC(sizeof(DEV_RET));
    BOOL ret;
    ULONG nWritten;
    Input.Offset    = Offset;
    Input.Value     = Value;
    ret = DeviceIoControl ( 
            hDevice, 
            DUT_IO_CODE(CMD_WR_REG),
            &Input,
            sizeof(Input),
            pOut,
            sizeof(DEV_RET), 
            &nWritten,
            NULL);
    if (nWritten != sizeof(DEV_RET))
    {
        printf("Write register failed, %d, %d\n", nWritten, ret);
    }
    else
    {
        printf("Ret:0x%08x\n", pOut->hResult);
    }
    __FREE(pOut);
}

BOOL __DutSimpeCmd(HANDLE hDevice, DUT_CMD Cmd, PDEV_RET pOut)
{
    ULONG nWritten = 0;
    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(Cmd),
        NULL, 
        0, 
        pOut,
        sizeof(DEV_RET), 
        &nWritten,
        NULL);
    return nWritten == sizeof(DEV_RET);
        
}
void DutStartRadio(HANDLE hDevice, ULONG RadioNo)
{
    PDEV_RET pOut = (PDEV_RET)__MALLOC(sizeof(DEV_RET));
	ULONG nWritten = 0;
	START_RADIO start_radio;	
	start_radio.RadioNo = RadioNo;	
    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(CMD_START_RADIO),
        &start_radio,
        sizeof(START_RADIO), 
        pOut,
        sizeof(DEV_RET), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET))
    {
        printf("Start Radio fails\n");
    }
    else
    {
        printf("Ret: 0x%08x\n", pOut->hResult);
    }	
    __FREE(pOut);
}

void DutStopRadio(HANDLE hDevice, ULONG RadioNo)
{
    PDEV_RET pOut = (PDEV_RET)__MALLOC(sizeof(DEV_RET));
	ULONG nWritten = 0;
	STOP_RADIO stop_radio;
	stop_radio.RadioNo = RadioNo;
    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(CMD_STOP_RADIO),
        &stop_radio,
        sizeof(STOP_RADIO), 
        pOut,
        sizeof(DEV_RET), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET))
    {
        printf("Stop Radio fails\n");
    }
    else
    {
        printf("Ret: 0x%08x\n", pOut->hResult);
    }	
    __FREE(pOut);
}

void DumpFromTimeStamp(ULONG* RadioNo, PSORA_RADIO_RX_STREAM pRxStream, char** buf, ULONG Count, ULONG start_timestamp, char UseTimeStamp, PCSTR szDumpFileName) {

	HRESULT hr;
	FLAG fReachEnd;
	PRX_BLOCK rx_block;
 	ULONG i;
	ULONG buf_offset;
	ULONG max_retry = DUMP_BUFFER_SIZE/sizeof(RX_BLOCK);
	
	printf("start dump from timestamp: 0x%08x\n", start_timestamp);
	
	for(buf_offset=0; buf_offset<DUMP_BUFFER_SIZE; buf_offset+=sizeof(RX_BLOCK)) {
		for(i=0; i < Count; i++) {
			PRX_BLOCK dest_rx_block = (PRX_BLOCK)(buf[i] + buf_offset);
			ULONG retry = 0;
	 		while(1) {
				rx_block = SoraRadioGetRxStreamPos(&pRxStream[i]);
				hr = SoraCheckSignalBlock(
					rx_block,
					SoraGetStreamVStreamMask(&pRxStream[i]),
					8000,
					&fReachEnd);
				if (hr != S_OK)
					return;

				__SoraRadioAdvanceRxStreamPos(&pRxStream[i]);

				if (UseTimeStamp) {
					if (!buf_offset) {
						// printf("radio: %d, rx_block: 0x%08x, found timestamp: 0x%08x\n", RadioNo[i], rx_block, rx_block->u.Desc.TimeStamp);
						if (rx_block->u.Desc.TimeStamp != start_timestamp) {
							retry ++; 
							if (retry > max_retry) {
								printf("out of sync\n");
								return;
							}
							continue;
						}
						printf("radio: %d, found timestamp: 0x%08x\n", RadioNo[i], rx_block->u.Desc.TimeStamp);
					}
				}
				
				memcpy(dest_rx_block, 
					rx_block, 
					sizeof(RX_BLOCK));
				
				dest_rx_block->u.Desc.VStreamBits = 0xffffffff;
				break;
			}		
		}
	}
	{
		SYSTEMTIME systime;
		CreateDirectory(
			"C:\\SORADUMP",
			NULL);
		GetLocalTime(&systime);
		for(i=0; i < Count; i++) {
			char name[256] = { 0 };			
			HANDLE hfile;

            if (szDumpFileName != NULL) {
			    sprintf_s(name, 
				    sizeof(name)-1,
				    "%s_%d.dmp",
                    szDumpFileName,
				    RadioNo[i]);			
            }
            else {
			    sprintf_s(name, 
				    sizeof(name)-1,
				    "C:\\SORADUMP\\HWTRxDump_%04d%02d%02d_%02d%02d%02d%03d_%08x_rad_%d.dmp",
				    systime.wYear,
				    systime.wMonth,
				    systime.wDay,
				    systime.wHour,
				    systime.wMinute,
				    systime.wSecond,
				    systime.wMilliseconds,
				    start_timestamp,
				    RadioNo[i]);			
            }
			hfile = CreateFile(name,
				GENERIC_READ|GENERIC_WRITE,
				0,
				NULL, 
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			if (hfile != INVALID_HANDLE_VALUE) {
				DWORD wrote;
				WriteFile(hfile,
					buf[i],
					DUMP_BUFFER_SIZE,
					&wrote,
					NULL);
				CloseHandle(hfile);
			}
		}
	}
}

ULONG ClearRxBuf(PSORA_RADIO_RX_STREAM pRxStream) {

	PRX_BLOCK rx_block;
	FLAG fReachEnd = 0;
	HRESULT hr;	
	ULONG last_timestamp = 0;
	ULONG skip_number = 0;
	rx_block = SoraRadioGetRxStreamPos(pRxStream);	
	while (1) {
		hr = SoraCheckSignalBlock(
			rx_block,
			SoraGetStreamVStreamMask(pRxStream),
			1,
			&fReachEnd);
		if (hr != S_OK)
			break;
		skip_number ++;
		last_timestamp = rx_block->blocks->Desc.TimeStamp;
		rx_block = __SoraRadioIncRxStreamPointer(pRxStream, rx_block);
	}	
	SoraRadioSetRxStreamPos(pRxStream, rx_block);
	printf("skip_number: %d\n", skip_number);
	return last_timestamp;
}

void DoDump(HANDLE hDevice, ULONG* RadioNo, ULONG Count, char UseTimeStamp, PCSTR szDumpFileName) {

	ULONG mask = 0;
	ULONG i;
	HRESULT err = S_OK;
	PVOID buf[MAX_RADIO_NUMBER] = { 0 };
	ULONG len[MAX_RADIO_NUMBER] = { 0 };
	SORA_RADIO_RX_STREAM RxStream[MAX_RADIO_NUMBER] = { 0 };
	do {
		if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
			err = E_FAIL;
			break;
		}
		for(i=0; i < Count; i++) {
			if (mask & RadioNo[i]) {
				err = E_FAIL;
				break;
			}
			if (SoraURadioMapRxSampleBuf(RadioNo[i], &buf[i], &len[i]) != S_OK) {
				err = E_FAIL;
				break;
			}
			if (SoraURadioAllocRxStream(&RxStream[i], RadioNo[i], buf[i], len[i]) != S_OK) {
				err = E_FAIL;
				break;
			}
		}
		if (err == S_OK) {
			// raise to realtime to dump
			char** dump_buf = __MALLOC(sizeof(char*) * Count);
			memset(dump_buf, 0, sizeof(char*) * Count);
			for(i=0; i < Count; i++) {
				dump_buf[i] = __MALLOC(DUMP_BUFFER_SIZE);
				if (!dump_buf[i]) {
					err = E_FAIL;
					break;
				}
			}
			SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
			
			if (err == S_OK) {
				ULONG max_timestamp;
				ULONG last_timestamp = 0;
				ULONG clear_count = 0;				
#if 1
				if (UseTimeStamp) {
					for(clear_count=0; clear_count < 2; clear_count++) {
						for(i=0; i < Count; i++) {
							last_timestamp = ClearRxBuf(&RxStream[i]);
							printf("radio: %d, last_timestamp: 0x%08x\n", RadioNo[i], last_timestamp);
							if (i) {
								if (last_timestamp > max_timestamp) {
									if (last_timestamp - max_timestamp > 0x7fffffff) { 
										// timestamp overflow !!! max_timestamp is still the max, do nothing
									}
									else 
										max_timestamp = last_timestamp;
								}
								else
								if (max_timestamp > last_timestamp) {
									if (max_timestamp - last_timestamp > 0x7fffffff) { 
										// timestamp overflow !!! 
										max_timestamp = last_timestamp;
									}
								}
							}
							else
								max_timestamp = last_timestamp;
						}
					}
					printf("max_timestamp: 0x%08x\n", max_timestamp);

					DumpFromTimeStamp(RadioNo, RxStream, dump_buf, Count, max_timestamp + DUMP_BUFFER_DELAY_TIMESTAMP, UseTimeStamp, szDumpFileName);
				}
				else {
					last_timestamp = ClearRxBuf(&RxStream[0]);

					DumpFromTimeStamp(RadioNo, RxStream, dump_buf, Count, 0, UseTimeStamp, szDumpFileName);
				}
			}

#endif

#if 0
timestamp_for_dump:
				for(i=0; i < Count; i++) {					
					last_timestamp = ClearRxBuf(&RxStream[i]);
					printf("last_timestamp: 0x%08x\n", last_timestamp);
					if (i) {
						if (last_timestamp > max_timestamp) {
							if (last_timestamp - max_timestamp > 0x7fffffff) { // timestamp overflow !!!
								printf("timestamp overflow, retry...\n");
								goto timestamp_for_dump;
							}
							max_timestamp = last_timestamp;
						}
						else
						if (max_timestamp > last_timestamp) {
							if (max_timestamp - last_timestamp > 0x7fffffff) { // timestamp overflow !!! 
								printf("timestamp overflow, retry...\n");
								goto timestamp_for_dump;
							}
						}
						else
						if (last_timestamp < min_timestamp)
							min_timestamp = last_timestamp;
					}
					else {
						min_timestamp = last_timestamp;
						max_timestamp = last_timestamp;
					}					
				}
				printf("max_timestamp: 0x%08x, min_timestamp: 0x%08x\n", max_timestamp, min_timestamp);
				if ((max_timestamp - min_timestamp) > (DUMP_BUFFER_SIZE/sizeof(RX_BLOCK))/2) {
					printf("prevent out of sync, retry...\n");
					goto timestamp_for_dump;
				}					
				DumpFromTimeStamp(RadioNo, RxStream, dump_buf, Count, max_timestamp + DUMP_BUFFER_DELAY_TIMESTAMP);
			}
#endif			
			SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
			
			for(i=0; i < Count; i++)
				if (!dump_buf[i])
					__FREE(dump_buf[i]);				
			__FREE(dump_buf);
		}
	} while(0);
	for(i=0; i < Count; i++) {
		if (RxStream[i].__VStreamMask)
			SoraURadioReleaseRxStream(&RxStream[i], RadioNo[i]);
		if (buf[i])
			SoraURadioUnmapRxSampleBuf(RadioNo[i],	buf[i]);
	}
	SoraUCleanUserExtension();
}

void DutDump(HANDLE hDevice, ULONG* RadioNo, ULONG Count, PCSTR szDumpFileName)
{
    ULONG nWritten;
    PDEV_RET pOutInfo = (PDEV_RET)__MALLOC(sizeof(DEV_RET) + sizeof(HWT_DETAIL_INFO));
    DeviceIoControl ( 
        hDevice,
        DUT_IO_CODE(CMD_INFO),
        NULL, 
        0, 
        pOutInfo,
        sizeof(DEV_RET) + sizeof(HWT_DETAIL_INFO), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET) + sizeof(HWT_DETAIL_INFO))
        printf("Get Radio Mask failed.\n");
    else {
		ULONG nWritten;
		PDEV_RET pOutVer = (PDEV_RET)__MALLOC(sizeof(DEV_RET) + sizeof(HWT_FW_VERSION));
		DeviceIoControl ( 
			hDevice, 
			DUT_IO_CODE(CMD_FW_VERSION),
			NULL, 
			0, 
			pOutVer,
			sizeof(DEV_RET) + sizeof(HWT_FW_VERSION), 
			&nWritten,
			NULL);
		if (nWritten != sizeof(DEV_RET) + sizeof(HWT_FW_VERSION))
			printf("Get Firmware version failed.\n");
		else {
			if (((HWT_FW_VERSION*)pOutVer->data)->m_fw_version >= 0x02000000) {
				PHWT_DETAIL_INFO hwt_detail_info;
				ULONG i;
				HRESULT r = S_OK;
				hwt_detail_info = (PHWT_DETAIL_INFO)pOutInfo->data;
				if (Count) {
					for(i=0; i < Count; i++) {
						if (hwt_detail_info->RadioMask & (1 << RadioNo[i]))
							continue;
						printf("Radio is not ready.\n");
						r = E_FAIL;
						break;
					}
					if (r == S_OK)
						DoDump(hDevice, RadioNo, Count, TRUE, szDumpFileName);
				}
				else {
					ULONG RadioNo[MAX_RADIO_NUMBER] = { 0 };
					ULONG Count = 0;
					for(i=0; i < MAX_RADIO_NUMBER; i++) {
						if (hwt_detail_info->RadioMask & (1 << i)) {
							RadioNo[Count] = i;
							Count++;
						}
					}
					DoDump(hDevice, RadioNo, Count, TRUE, szDumpFileName);
				}
			}
			else {
				ULONG RadioNo = 0;
				ULONG Count = 1;
				DoDump(hDevice, &RadioNo, Count, FALSE, szDumpFileName);
			}
		}
		__FREE(pOutVer);
    }
    __FREE(pOutInfo);
}

void PrintHwtInfo(PHWT_DETAIL_INFO Info)
{
    ULONG i = 0;
	printf("Radio Mask: 0x%02x\n", Info->RadioMask);
    printf("blocks in time percentage:\t%d /100 \n", 
        (Info->ullBlocksInTime * 100) / (Info->ullBlocksLag + Info->ullBlocksInTime + 1));
    printf("blocks lag: \t\t\t%016x\n", Info->ullBlocksLag);
    printf("blocks in time: \t\t%016x\n", Info->ullBlocksInTime);

    printf("\n------------------------------------------\n\n");
    printf("Total %d signal(s) are cached\n\n", Info->Count);
    printf("ID\tFILE\n");
    for (; i < Info->Count; i++)
    {
        printf("%d\t", Info->CachedSignals[i].PacketID);
        wprintf(L"%s\n", Info->CachedSignals[i].SymFileName);
    }
}
void DutGetInfo(HANDLE hDevice)
{
    ULONG nWritten;
    PDEV_RET pOut = (PDEV_RET)__MALLOC(sizeof(DEV_RET) + sizeof(HWT_DETAIL_INFO));
    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(CMD_INFO),
        NULL, 
        0, 
        pOut,
        sizeof(DEV_RET) + sizeof(HWT_DETAIL_INFO), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET) + sizeof(HWT_DETAIL_INFO))
        printf("Information unvailable %d\n", nWritten);
    else
        PrintHwtInfo((PHWT_DETAIL_INFO)pOut->data);
    __FREE(pOut);
}

void DutStopTX(HANDLE hDevice)
{
    DEV_RET Out;
    BOOL ret = __DutSimpeCmd(hDevice, CMD_STOP_TX, &Out);
    if (!ret)
    {
        printf("Stop TX fails\n");
    }
    else
        printf("Ret:0x%08x\n", Out.hResult);
}
size_t Str2WStr(PCSTR strSrc, PWSTR wstrDest, size_t Count)
{
    size_t i;
    if (strSrc == NULL) return 0;
    for ( i = 0; i < Count - 1 && strSrc[i] != '\0' ; i++)
    {
        mbtowc(wstrDest + i, strSrc + i, sizeof(char));
    }
    if (strSrc[i] != '\0')
    {
        i = 0; //fail
    }
    else
    {
        mbtowc(wstrDest + i, strSrc + i, sizeof(char));
    }
    return i;
}

void DutTransferSignals(HANDLE hDevice, PCSTR szSigFileName)
{
    ULONG nWritten;
    DEV_RET Out;
    size_t len;
    PWSTR wszFileName = (PWSTR)__MALLOC(SYM_FILE_MAX_PATH_LENGTH * sizeof(WCHAR));
    PSTR szFileName = (PSTR) __MALLOC(SYM_FILE_MAX_PATH_LENGTH * sizeof(CHAR));
    szFileName[0] = '\0';
    
    assert(strlen(szSigFileName) + strlen(FILE_OBJ_NAME_PREFIX) <  SYM_FILE_MAX_PATH_LENGTH);
    
    strcat_s(szFileName, SYM_FILE_MAX_PATH_LENGTH, FILE_OBJ_NAME_PREFIX);
    strcat_s(szFileName, SYM_FILE_MAX_PATH_LENGTH, szSigFileName);

    len = Str2WStr(szFileName, wszFileName , SYM_FILE_MAX_PATH_LENGTH);
    if (len == 0)
    {
        printf("Please specify a signal file (name length no more than 100)\n");
        return;
    }
    
    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(CMD_TRANSFER),
        wszFileName, 
        SYM_FILE_MAX_PATH_LENGTH * sizeof(WCHAR), 
        &Out,
        sizeof(DEV_RET), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET))
        printf("Input error!\n");
    else
        printf("Ret:0x%08x\n", Out.hResult);
    __FREE(wszFileName);
    __FREE(szFileName);
}

void DutTxSignals(HANDLE hDevice, ULONG RadioNo, ULONG sigID, ULONG Times)
{
    DEV_RET Out;
    ULONG nWritten = 0;
    TIMES_ID_PAIR input;
	input.RadioNo = RadioNo;
    input.PacketID = sigID;
    input.Times = Times;
    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(CMD_TX),
        &input,
        sizeof(TIMES_ID_PAIR), 
        &Out,
        sizeof(DEV_RET), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET))
    {
        printf("Input error!\n");
    }
    else
    {
        printf("Ret: 0x%08x\n", Out.hResult);
    }
}

void DutTxDone(HANDLE hDevice, ULONG sigID)
{
    DEV_RET Out;
    ULONG nWritten = 0;

    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(CMD_TX_DONE),
        &sigID,
        sizeof(sigID), 
        &Out,
        sizeof(DEV_RET), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET))
    {
        printf("Input error!\n");
    }
    else
    {
        printf("Ret: 0x%08x\n", Out.hResult);
    }
}

void __DutSet(HANDLE hDevice, ULONG RadioNo, DUT_CMD code, LONG Value)
{
    DEV_RET Out;
    ULONG nWritten = 0;
	RADIO_PARAMETER radio_parameter;
	radio_parameter.RadioNo = RadioNo;
	radio_parameter.Value = Value;
    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(code),
        &radio_parameter,
        sizeof(RADIO_PARAMETER), 
        &Out,
        sizeof(DEV_RET), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET))
    {
        printf("Input error!\n");
    }
    else
    {
        printf("Ret: 0x%08x\n", Out.hResult);
    }
}

void DutSetTxGain(HANDLE hDevice, ULONG RadioNo, ULONG gain)
{
    __DutSet(hDevice, RadioNo, CMD_TX_GAIN, gain);
}

void DutSetRxGain(HANDLE hDevice, ULONG RadioNo, ULONG gain)
{
    __DutSet(hDevice, RadioNo, CMD_RX_GAIN, gain);
}

void DutSetCentralFreq(HANDLE hDevice, ULONG RadioNo, ULONG Freq)
{
    __DutSet(hDevice, RadioNo, CMD_CENTRAL_FREQ, Freq);
}

#if 0
void DutGetRxBuffer(HANDLE hDevice)
{
    PDEV_RET pOut = (PDEV_RET)__MALLOC(sizeof(DEV_RET) + sizeof(HWT_DMA_RET));
    ULONG nWritten = 0;
    DeviceIoControl ( 
        hDevice, 
        DUT_IO_CODE(CMD_DMA),
        NULL, 
        0,
        pOut,
        sizeof(DEV_RET) + sizeof(HWT_DMA_RET), 
        &nWritten,
        NULL);
    if (nWritten != sizeof(DEV_RET) + sizeof(HWT_DMA_RET))
    {
        printf("Input error!\n");
    }
    else
    {
        PULONG p = (PULONG)(((PHWT_DMA_RET)(pOut->data))->UserVA);
        printf("Ret: 0x%08x\n", pOut->hResult);
        printf("RX Buffer: 0x%08x\n", p);
        printf("RX Size: 0x%08x\n", ((PHWT_DMA_RET)(pOut->data))->Size);
        printf("content: %08X, %08X, %08X, %08X", p[0], p[1], p[2], p[3]);
        p[0] = 0;
    }
}
#endif

void DutSetFreqOffset(HANDLE hDevice, ULONG RadioNo, LONG Freq)
{
    __DutSet(hDevice, RadioNo, CMD_FREQ_OFFSET, Freq);
}
//int TestParse(IN const int argc, IN const char **argv)
//{
//    int ret = 0;
//                           //szOpt,metaValue, szHelp
//    ARG_LITERAL rrArg       = {{"rr", NULL, "read register with /regname Regname", LIT}, 0};
//    ARG_LITERAL wrArg       = {{"wr", NULL, "write register with /regname Regname /value RegValue option", LIT}, 0};
//    ARG_LITERAL stArg       = {{"st", NULL, "Start first radio", LIT}, 0};
//    ARG_LITERAL clArg       = {{"cl", NULL, "Clean first radio resources", LIT}, 0};
//    ARG_LITERAL tfArg       = {{"tf", NULL, "transfer packet to ULCB buffer", LIT}, 0};
//    ARG_STRING  lenArg      = {{"l", NULL, "transmitted packet length", STR}, 0};
//    ARG_LITERAL tdArg       = {{"td", NULL, "clean TX resources owned by packet", LIT}, 0};
//    ARG_LITERAL txArg       = {{"tx", NULL, "send out packet in ULCB buffer", LIT}, 0};
//    ARG_LITERAL rxArg       = {{"rx", NULL, "Fetch samples in ", LIT}, 0};
//    ARG_LITERAL csArg       = {{"cs", NULL, "Carrie Sense, rx in a thread ", LIT}, 0};
//    ARG_LITERAL nocsArg       = {{"nocs", NULL, "stop Carrie Sense thread", LIT}, 0};
//    ARG_LITERAL norxArg       = {{"norx", NULL, "stop radio rx", LIT}, 0};
//    ARG_LITERAL resetPhy    = {{"reset", NULL, "Reset PHY context when /rx, default is not reset", LIT}, 0};
//    ARG_LITERAL blockArg    = {{"blocks", "blockNum", 
//        "specify signal block number when /rx, if not specified, read signal until reach end", LIT}, 0};
//
//    ARG_STRING  regNameArg  = {{"regname", "RegName", "Specify Register Offset", STR}, NULL, 0};
//    ARG_STRING  regValArg   = {{"value", "RegValue", "Specify Register value", STR}, NULL, 0};
//    ARG_STRING  contentArg  = {{"cont", "Content", "Specify Content to transfer", STR}, NULL, 0};
//
//    ARG_LITERAL chArg       = {{"ch", NULL, "Unit test phy frame cache", LIT}, 0};
//    ARG_LITERAL ackArg       = {{"ack", NULL, "fast TX random ack in cache", LIT}, 0};
//    ARG_LITERAL dumpArg       = {{"dump", NULL, "dump 16MB RX buffer", LIT}, 0};
//    ARG_LITERAL tfDhcp            = {{"tfdhcp", NULL, "transfer dhcp packet", LIT}, 0};
//    ARG_LITERAL txDhcp            = {{"txdhcp", NULL, "tx dhcp packet", LIT}, 0};
//    ARG_STRING  tftx            = {{"tftx", "times", "auto transfer and tx ", STR}, NULL, 0};
//    ARG_STRING  fileArg      = {{"f", "symbolfile", "transmitted filename", STR}, NULL, 0};
//
//    void *ArgTable[]        = {
//                                    &rrArg,
//                                    &wrArg,
//                                    &stArg,
//                                    &clArg,
//                                    &tfArg,
//                                    &lenArg, 
//                                    &tdArg,
//                                    &txArg,
//                                    &rxArg,
//                                    &csArg, 
//                                    &nocsArg,
//                                    &norxArg,
//                                    &resetPhy,
//                                    &blockArg,
//                                    &regNameArg,
//                                    &regValArg,
//                                    &contentArg,
//                                    &chArg,
//                                    &ackArg,
//                                    &dumpArg, 
//                                    &tfDhcp,
//                                    &txDhcp,
//                                    &tftx,
//                                    &fileArg
//                               };
//
//    ret = ParseArgv(argc, argv, ArgTable, sizeof(ArgTable)/sizeof(void*));
//    
//    if (!ret)
//    {
//        Usage(ArgTable, sizeof(ArgTable)/sizeof(void*));
//        return 0;
//    }
//    else
//    {
//        if (wrArg.Count + stArg.Count + clArg.Count +
//            tfArg.Count + tdArg.Count + txArg.Count +
//            rxArg.Count > 1)
//        {
//            Usage(ArgTable, sizeof(ArgTable)/sizeof(void*));
//            return 0;
//        }
//        if (wrArg.Count == 1)
//        {
//            if (regNameArg.Count != 1 || regValArg.Count != 1)
//            {
//                Usage(ArgTable, sizeof(ArgTable)/sizeof(void*));
//                return 0;
//            }
//        }
//    }
//    return 1;
//}

void DutGetFwVersion(HANDLE hDevice) {

	ULONG nWritten;
	PDEV_RET pOut = (PDEV_RET)__MALLOC(sizeof(DEV_RET) + sizeof(HWT_FW_VERSION));
	DeviceIoControl ( 
		hDevice, 
		DUT_IO_CODE(CMD_FW_VERSION),
		NULL, 
		0, 
		pOut,
		sizeof(DEV_RET) + sizeof(HWT_FW_VERSION), 
		&nWritten,
		NULL);
	printf("firmware version: 0x%08x\n", ((HWT_FW_VERSION*)pOut->data)->m_fw_version);
	__FREE(pOut);
}

void DutMimoTx(HANDLE hDevice, ULONG* RadioNo, ULONG* sigID, ULONG Count, ULONG Times) {

	DEV_RET Out;
	ULONG nWritten = 0;
	TIMES_ID_PAIRS input;
	ULONG i;
	for(i=0; i < Count; i++) {
		input.RadioNo[i] = RadioNo[i];
		input.PacketID[i] = sigID[i];		
	}
	input.Count = Count;
	input.Times = Times;
	DeviceIoControl ( 
		hDevice, 
		DUT_IO_CODE(CMD_MIMO_TX),
		&input,
		sizeof(TIMES_ID_PAIRS), 
		&Out,
		sizeof(DEV_RET), 
		&nWritten,
		NULL);
	if (nWritten != sizeof(DEV_RET))
	{
		printf("Input error!\n");
	}
	else
	{
		printf("Ret: 0x%08x\n", Out.hResult);
	}
}

