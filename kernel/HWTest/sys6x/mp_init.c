#include "mp_6x.h"

#ifndef _M_X64
__forceinline void CLI()
{
    __asm
    {
        cli;
    }
}

__forceinline void STI()
{
    __asm 
    {
        sti;
    }
}
#endif

//VOID FnRxThread(IN PVOID Context)
//{
//    ULONGLONG begin, end, interval;
//    PHWT_ADAPTER Adapter = (PHWT_ADAPTER)Context;
//    //CLI();
//    begin = __rdtsc();
//    __asm 
//    {
//        
//        mov edx, 0
//loopbody:   
//        add edx, 1 ;
//        cmp edx, 40;
//        jnz loopbody;
//    }
//    end = __rdtsc();
//    //STI();
//    interval = ((end - begin) * 1000 * 1000 /(Adapter->Clock.Frequency.QuadPart));
//
//
//    if (interval > 10)
//    {
//        Adapter->InterruptedCount++;
//        DbgPrint("[Perf] time=%llu us", interval);
//    }
//
//    Adapter->TestCount++;
//
//    if (Adapter->TestCount > 100000)
//    {
//        DbgPrint("[Stat] High priority : %u /100,000 \n", Adapter->InterruptedCount );
//        Adapter->TestCount = 0;
//        Adapter->InterruptedCount = 0;
//    }
//    
//}
//
//
//void NicInitTestThread(PHWT_ADAPTER Adapter)
//{
//    SoraEthreadInit(&Adapter->RxThread, FnRxThread, Adapter, CORE_2_AFFINITY);
//    SoraEthreadStart(&Adapter->RxThread);
//
//    Adapter->TestCount = 0;
//    Adapter->InterruptedCount = 0;
//
//    SoraInitClock(&Adapter->Clock, 0);
//}