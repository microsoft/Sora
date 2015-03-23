#include "hwtest_miniport.h"

//volatile ULONG   fDisplaySample = 0;
//volatile ULONG   fPhyFrameBlockSize = 1024;
//extern volatile UCHAR IsAck;
//void DbgPrintSampleBlock(PUCHAR pt, PSORA_SAMPLE_BLOCK buffer, ULONG BlockNo);
//
//volatile ULONG fDumpMode = 0;

//void __SnapShotRxBuffer(PSORA_RADIO pRadio, PVOID pDumpBuffer)
//{
//    PSORA_RADIO_RX_STREAM pRxStream = SORA_GET_RX_STREAM(pRadio);
//    PUCHAR pPeek = SoraGetRadioCurrentScanPoint(pRxStream);    
//    PUCHAR pStart = pRxStream->__pStartPt;
//    ULONG Offset = (ULONG)(pPeek - pStart);
//    ULONG i;
//    
//    DbgPrint("[CS] Current Peek point offset %08x \n", Offset);
//    for (i = 0; i * sizeof(RX_BLOCK) < 16 * 1024 * 1024; i++)
//    {
//        __SnapShotSignalBlockWithDesc((PRX_BLOCK)pStart, ((PUCHAR)pDumpBuffer) + i * sizeof(RX_BLOCK));
//        pStart = SoraGetRadioNextScanPoint(pRxStream, pStart);
//    }
//    
//}
void TryDump(PSORA_RADIO pRadio)
{
    PVOID pDumpBuffer = NULL;
    ULONG nDumpped = 0;
    
    NdisAllocateMemoryWithTag(
        &pDumpBuffer, 
        16 * 1024 * 1024, 
        NIC_TAG);
    if (pDumpBuffer != NULL)
    {
	    HANDLE  hFile;
		WCHAR* file = L"\\??\\C:\\HWTRxDump";		
		__ClearNewRxBuffer(pRadio);
		
        SoraDumpNewRxBuffer(pRadio, pDumpBuffer, 16 * 1024 * 1024, &nDumpped);
        //__SnapShotRxBuffer(pRadio, pDumpBuffer);
        hFile = __CreateDumpFile(file);
		if (!hFile)
	        hFile = __CreateDumpFile(L"\\Device\\HarddiskVolume1\\HWTRxDump");
		
        if(hFile != NULL)
        {
            __DumpBuffer2File(hFile, pDumpBuffer, nDumpped);
            __CloseDumpFile(hFile);
        }

        NdisFreeMemory(pDumpBuffer, 16 * 1024 * 1024, 0);
        pDumpBuffer = NULL;
        DbgPrint("[HWT] Dumpped bytes %08x\n", nDumpped);
    }
}

void RealTimeTest()
{
#ifndef _M_X64
    ULONGLONG begin, end, interval;
    LARGE_INTEGER   Frequency;
    static ULONG nExceeded  = 0;
    static ULONG nTotal     = 0;
    KeQueryPerformanceCounter(&Frequency);

    begin = __rdtsc();
    __asm 
    {
            
        mov edx, 0
loopbody:   
        add edx, 1 ;
        cmp edx, 400000;
        jnz loopbody;
    }
    end = __rdtsc();
    interval = ((end - begin) * 1000  /(Frequency.QuadPart));

    if (interval > 295) //290-300
    {
        nExceeded++;
        DbgPrint("[HWTest][Perf] time=%llu us, Frequency=%llu\n", interval, Frequency.QuadPart);
    }

    nTotal++;
    if (nTotal > 100000)
    {
        DbgPrint("[HWTest][Perf] High priority: %u /100,000 \n", nExceeded );
        nTotal = 0;
        nExceeded = 0;
    }

    return;
#endif
}

