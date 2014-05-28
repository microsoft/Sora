#include <xmmintrin.h>
#include <stdio.h>
#include <math.h>
#include "bb/bba.h"
#include "bb/mod.h"

#define __SYNC_MIN_IQ_COUNT         (32)

#define __BB11A_RX_WAIT_DATA        ((HRESULT)0x00000300L) // insufficient samples in FiFo for current processing
#define __BB11A_RX_MODE_SWITCH      ((HRESULT)0x00000301L) // enter into new RX mode.

#define RM_SYNC     1 // RX sync mode
#define RM_PRE      2 // RX preamble mode
#define RM_HEADER   3 // RX PLCP header mode
#define RM_DATA     4 // RX service data unit mode

#define LT_CP_SKIP  6
#define SYM_CP_SKIP 2

// signal parsing
//
// return 1 for success
// return 0 for bad signal
//
HRESULT BB11ARxTrySync(PBB11A_RX_CONTEXT pRxContextA)
{
    HRESULT hr = __BB11A_RX_WAIT_DATA;
    
    while (1)
    {
        vcs *pcTemp;
        char bRet;
        if (pRxContextA->__iSyncPeakPos < 0) // not sync yet
        {
            bRet = pRxContextA->rxFifos->g11a_lbSync.RCheck(4);
            if (!bRet)
                break;

            pcTemp = pRxContextA->rxFifos->g11a_lbSync.Peek(); //get IQ Sync fifo head

            // Kun: I assume EstablishSync is okay. May revisit later
            pRxContextA->__iSyncPeakPos = EstablishSync((vi*)pRxContextA->corrRe, pcTemp, 
                                                  (int*)&pRxContextA->__uiSyncMax);

            if (pRxContextA->__iSyncPeakPos < 0) // sync failed
            {
                hr = BB11A_E_SYNC_FAIL;
                KdPrint(("FD_NEGATIVE: Symbol sync failed.\n"));
                break;
            }
            else // sync succeed
            {
                int iLo2bits    = pRxContextA->__iSyncPeakPos & 0x3;
                int iHi30bits   = pRxContextA->__iSyncPeakPos & ~0x3;

                if (iHi30bits > 0)
                {
                    pRxContextA->rxFifos->g11a_lbSync.Pop(iHi30bits / 4 ); //trim redundant old IQ in Fifo
                }
                pRxContextA->__iSyncPeakPos = iLo2bits;
            }
        }
        else // synced already
        {
            bRet = pRxContextA->rxFifos->g11a_lbSync.RCheck(4);
            if (!bRet)
                break;

            pcTemp = pRxContextA->rxFifos->g11a_lbSync.Peek();
            bRet = CheckSync ( (vi*)pRxContextA->corrRe, pcTemp, 
                               pRxContextA->__iSyncPeakPos,
                               pRxContextA->__uiSyncMax,
                               pRxContextA->digitalAGC);
            pRxContextA->rxFifos->g11a_lbSync.Pop(4);

            if (!bRet) // sync end, long preamble now
            {
                pRxContextA->__uRxMode = RM_PRE;
                hr = __BB11A_RX_MODE_SWITCH;
                
                // will branch mode
                break;
            }
            // else sync continues
        }   
    }
    return hr;
}

HRESULT BB11ARxTryPreambleFFT(PBB11A_RX_CONTEXT pRxContextA)
{
    HRESULT hr;
    do
    {
        // wait for 160 complexes, __pcTemp will point to the first one
        char __bRet;
        vcs *__pcTemp;
        // Note: tatal 40 vcs for preamble, the first 4 is consumed by BB11ARxTrySync
        __bRet = pRxContextA->rxFifos->g11a_lbSync.RCheck(36); 
        if (!__bRet)
        {
            hr = __BB11A_RX_WAIT_DATA;
            break;
        }
        __pcTemp = pRxContextA->rxFifos->g11a_lbSync.Peek();

        // this should be approximately start of long preamble

        // ignore the first 24 complexes, 
        // we use no. 24 - 152 to estimate freqency offset
        __pcTemp += LT_CP_SKIP - 4;

        _dump_symbol<128> ("LTS", (COMPLEX16*) __pcTemp );

        // estimate frequency offset from 128 IQs
        pRxContextA->freqEstimated = EstimateFreqOffset ( (COMPLEX16*)__pcTemp, 
                                      pRxContextA->freqFactorCos, 
                                      pRxContextA->freqFactorSin );

        // freqency offset is estimated, compensate 64 IQs immediately
        FreqComp64(__pcTemp, pRxContextA->freqEstimated, 
                    (vs*)pRxContextA->freqFactorCos, 
                    (vs*)pRxContextA->freqFactorSin );
		
        _dump_symbol<64> ("After Freq Comp", (COMPLEX16*) __pcTemp );

        // do fft, on 24 - 88 complex? 64 IQs
        FFT<64>(__pcTemp, (vcs*)pRxContextA->cFFTOut);

        _dump_symbol<64> ("After FFT", (COMPLEX16*) pRxContextA->cFFTOut );

        // estimate channel compensation
        EstimateChannel(pRxContextA->cFFTOut, pRxContextA->channelFactor);
        _dump_symbol32<64> ("Channel", (COMPLEX32*) pRxContextA->channelFactor);

        pRxContextA->rxFifos->g11a_lbSync.Pop(36);
        
        pRxContextA->__bPilotCounter = 0;
        
        pRxContextA->__uRxMode = RM_HEADER;
        
#ifdef USER_MODE
        TimerStop(&pRxContextA->ri_tOfflineTimings[0]);
        TimerStart(&pRxContextA->ri_tOfflineTimings[1]);
#endif
        hr = __BB11A_RX_MODE_SWITCH;
    } while(FALSE);

    return hr;
}

HRESULT BB11ARxTryHeader(PBB11A_RX_CONTEXT pRxContextA)
{
    HRESULT hr;
    BOOL __bRet;
    vcs *__pcTemp;
    unsigned int uiDBPS, uiSignal;

    do
    {
        // wait for 80 complexes, __pcTemp will point to the first one
        __bRet = pRxContextA->rxFifos->g11a_lbSync.RCheck(20);
        if (!__bRet)
        {
            hr = __BB11A_RX_WAIT_DATA;
            break;
        }
        // this should be approximately the symbol for the header
        __pcTemp = pRxContextA->rxFifos->g11a_lbSync.Peek();

        // ignore first 8 complexes,
        // we use no. 8 - 72 to do fft,
        // this has the same alignment for 24 - 84 complex in
        // long preamble
        __pcTemp += SYM_CP_SKIP;

       
#if BB_DBG        
        dumpSymbol16(__pcTemp);
#endif        
		_dump_symbol<64> ( "OFDM Symbol", (COMPLEX16*)__pcTemp );

        // do frequency compensation on 64 complexes
        FreqComp64(__pcTemp, pRxContextA->freqEstimated, 
                    (vs*)pRxContextA->freqFactorCos, 
                    (vs*)pRxContextA->freqFactorSin);
#if BB_DBG
        dumpSymbol16(__pcTemp);
#endif

		_dump_symbol<64> ( "After freq comp", (COMPLEX16*)__pcTemp );

        // do fft, on 8 - 72 complex
        FFT<64>(__pcTemp, (vcs*)pRxContextA->cFFTOut);

		_dump_symbol<64> ( "After FFT", (COMPLEX16*)pRxContextA->cFFTOut);

#if BB_DBG        
        dumpSymbol16(pRxContextA->cFFTOut);
#endif

        // channel compensation on 64 complexes
        ChannelComp(pRxContextA->channelFactor, pRxContextA->cFFTOut);

        // pilot freq compensation on 64 complexes
        Pilot(pRxContextA->cFFTOut, 
              pRxContextA->channelFactor,
              &pRxContextA->__bPilotCounter);

		_dump_symbol<64> ( "after pilot", (COMPLEX16*)pRxContextA->cFFTOut );
		
        // demap
        Demap_11a(pRxContextA->cFFTOut, pRxContextA->bDemapped, 0x3);

        // deinterleave
        Deinterleave(pRxContextA->bDemapped, pRxContextA->bDeinterleaved, 0x3);
        
        // viterbi decoder, specially for dot11a signal symbol
        memset(pRxContextA->bDeinterleaved + 48, 0, 48);

        Viterbi_asig((vub *)pRxContextA->trellisAsig, (char *)pRxContextA->bDeinterleaved, (char *)(&uiSignal));
        //KdPrint(("Signal: 0x%06x\n", (uiSignal & 0xFFFFFF)));

        // header check
#ifdef USER_MODE
        QueryPerformanceCounter(&pRxContextA->ullSignalFindTimeStamp);
#else
        pRxContextA->ullSignalFindTimeStamp = KeQueryPerformanceCounter(NULL);
#endif
        __bRet = ParseSignal(uiSignal, &pRxContextA->__usLength, (PUCHAR)&pRxContextA->bRate, &uiDBPS);
        //KdPrint(("Frame length: %d\n", usLength));

        if (!__bRet)
        {
            hr = BB11A_E_INVALID_SIG;
            KdPrint(("FD_NEGATIVE: Invalid signal: 0x%06x\n", 
                        (uiSignal & 0xFFFFFF)));
            break;
        }

        if (pRxContextA->__usLength > pRxContextA->ri_uiFrameMaxSize || pRxContextA->__usLength < 4)
        {
            hr = BB11A_E_FRAME_SIZE;
            KdPrint(("FD_NEGATIVE: frame size(%db) "
                        "larger than max size(%db), or less than 4\n",
                        pRxContextA->__usLength, pRxContextA->ri_uiFrameMaxSize));
            break;
        }

        // pop it out, header symbol done
        pRxContextA->rxFifos->g11a_lbSync.Pop(20);

        pRxContextA->__uRxMode = RM_DATA;
        pRxContextA->__uiSymbolCount = (((pRxContextA->__usLength << 3) + (22 - 1 + uiDBPS)) / uiDBPS);
        if (pRxContextA->bRate == 0xF && (pRxContextA->__uiSymbolCount & 0x1))
        {
            // pad one symbol so that it 
            // will not produce haft byte
            pRxContextA->__uiSymbolCount++;
        }
        // KdPrint(("Symbol Count:%d\n", uiSymbolCount));
         
        pRxContextA->uiVitSymbolCount   = pRxContextA->__uiSymbolCount;
        pRxContextA->uiVitFrameLen      = pRxContextA->__usLength;
        pRxContextA->ri_uiFrameSize     = pRxContextA->__usLength;
        pRxContextA->pbVitFrameOutput   = pRxContextA->ri_pbFrame;

        switch (pRxContextA->bRate & 0x3)
        {
            case 0x3:
                pRxContextA->rxFifos->vb1.Clear();
                break;
            case 0x2:
                pRxContextA->rxFifos->vb2.Clear();
                break;
            case 0x1:
                pRxContextA->rxFifos->vb3.Clear();
                break;
            case 0x0:
                pRxContextA->rxFifos->vb4.Clear();
                break;
        }

        BB11A_VITERBIDONE_CLEAR_EVENT(pRxContextA);
        _mm_mfence(); // guarantee viterbi parameters are prepared.
        BB11A_VITERBIRUN_SET_EVENT(pRxContextA); //start viterbi processing.

        pRxContextA->__bPilotCounter = 0;

#ifdef USER_MODE
        TimerStop(&pRxContextA->ri_tOfflineTimings[1]);
        TimerStart(&pRxContextA->ri_tOfflineTimings[2]);
        TimerStart(&pRxContextA->ri_tOfflineTimings[4]);
#endif
        hr = __BB11A_RX_MODE_SWITCH;
    } while (FALSE);

    return hr;
}

FINL void __BB11APushSignalBlockToSyncFiFo(PBB11A_RX_CONTEXT pRxContextA, const SignalBlock& block)
{
    vcs *pcTemp;
    pcTemp = pRxContextA->rxFifos->g11a_lbSync.Push(7); // allocate buffer space
    *(SignalBlock*)pcTemp = block;
}

void __BB11ARxPrintFrame(PBB11A_RX_CONTEXT pRxContextA)
{
    int i;
    for (i = 0; i < pRxContextA->__usLength; i++)
    {
        if (i % 32 == 0)
        {
            KdPrint(("\n  %04x: ", i));
        }
        else if (i % 4 == 0)
        {
            KdPrint((" "));
        }

        KdPrint(("%02x", 
            (unsigned char)(pRxContextA->pbVitFrameOutput[i])));
    }
    KdPrint(("\n"));
}

HRESULT BB11ARxTryData(PBB11A_RX_CONTEXT pRxContextA)
{
    HRESULT hr;
    volatile FLAG *pbWorkIndicator = pRxContextA->ri_pbWorkIndicator;
    do
    {
        // wait for 80 complexes, __pcTemp will point to the first one
        unsigned char *pbTemp;
        char bRet;
        vcs *__pcTemp;
        bRet = pRxContextA->rxFifos->g11a_lbSync.RCheck(20);
        if (!bRet)
        {
            hr = __BB11A_RX_WAIT_DATA;
            break;
        }
        // this should be approximately one symbol of data
        __pcTemp = pRxContextA->rxFifos->g11a_lbSync.Peek();

        // ignore first 8 complexes
        // we use no. 8 - 72 to do fft, 
        // this has the same alignment for 24 - 84 complex in
        // long preamble
        __pcTemp += SYM_CP_SKIP;


		_dump_symbol<64> ( "OFDM symbol", (COMPLEX16*) __pcTemp );

        // do frequency compensation on 64 complexes
        FreqComp64(__pcTemp, pRxContextA->freqEstimated, (vs*)pRxContextA->freqFactorCos, (vs*)pRxContextA->freqFactorSin);

		_dump_symbol<64> ("After freq compensation", (COMPLEX16*) __pcTemp);        
		
        // do fft, on 8 - 72 complex
        FFT<64>(__pcTemp, (vcs*)pRxContextA->cFFTOut);
		_dump_symbol<64> ( "after FFT", (COMPLEX16*)pRxContextA->cFFTOut );
        // channel compensation
        ChannelComp(pRxContextA->channelFactor, pRxContextA->cFFTOut);

		_dump_symbol<64> ("After equalization", (COMPLEX16*) pRxContextA->cFFTOut);        
		
        // pilot freq compensation
        Pilot(pRxContextA->cFFTOut, 
              pRxContextA->channelFactor,
              &pRxContextA->__bPilotCounter);
        
        // demap
        Demap_11a(pRxContextA->cFFTOut, pRxContextA->bDemapped, pRxContextA->bRate);

        // deinterleave and sent to viterbi buffer
        switch (pRxContextA->bRate & 0x3)
        {
            case 0x3:
                pRxContextA->rxFifos->vb1.SpaceWait(1, pbWorkIndicator);
                pbTemp = pRxContextA->rxFifos->vb1.Push();
                Deinterleave(pRxContextA->bDemapped, pbTemp, pRxContextA->bRate);
                pRxContextA->rxFifos->vb1.Flush();
                break;
            case 0x2:
                pRxContextA->rxFifos->vb2.SpaceWait(1, pbWorkIndicator);
                pbTemp = pRxContextA->rxFifos->vb2.Push();
                Deinterleave(pRxContextA->bDemapped, pbTemp, pRxContextA->bRate);
                pRxContextA->rxFifos->vb2.Flush();
                break;
            case 0x1:
                pRxContextA->rxFifos->vb3.SpaceWait(1, pbWorkIndicator);
                pbTemp = pRxContextA->rxFifos->vb3.Push();
                Deinterleave(pRxContextA->bDemapped, pbTemp, pRxContextA->bRate);
                pRxContextA->rxFifos->vb3.Flush();
                break;
            case 0x0:
                pRxContextA->rxFifos->vb4.SpaceWait(1, pbWorkIndicator);
                pbTemp = pRxContextA->rxFifos->vb4.Push();
                Deinterleave(pRxContextA->bDemapped, pbTemp, pRxContextA->bRate);
                pRxContextA->rxFifos->vb4.Flush();
                break;
        }

        pRxContextA->rxFifos->g11a_lbSync.Pop(20);

        pRxContextA->__uiSymbolCount--;
        if (pRxContextA->__uiSymbolCount == 0)
        {
            // zero padding for traceback
            switch (pRxContextA->bRate & 0x3)
            {
                case 0x3:
                    pRxContextA->rxFifos->vb1.SpaceWait(3, pbWorkIndicator);
                    pbTemp = pRxContextA->rxFifos->vb1.Push();
                    memset(pbTemp, 0, 48);
                    pRxContextA->rxFifos->vb1.Flush();
                    pbTemp = pRxContextA->rxFifos->vb1.Push();
                    memset(pbTemp, 0, 48);
                    pRxContextA->rxFifos->vb1.Flush();
                    pbTemp = pRxContextA->rxFifos->vb1.Push();
                    memset(pbTemp, 0, 48);
                    pRxContextA->rxFifos->vb1.Flush();
                    break;
                case 0x2:
                    pRxContextA->rxFifos->vb2.SpaceWait(2, pbWorkIndicator);
                    pbTemp = pRxContextA->rxFifos->vb2.Push();
                    memset(pbTemp, 0, 96);
                    pRxContextA->rxFifos->vb2.Flush();
                    pbTemp = pRxContextA->rxFifos->vb2.Push();
                    memset(pbTemp, 0, 96);
                    pRxContextA->rxFifos->vb2.Flush();
                    break;
                case 0x1:
                    pRxContextA->rxFifos->vb3.SpaceWait(1, pbWorkIndicator);
                    pbTemp = pRxContextA->rxFifos->vb3.Push();
                    memset(pbTemp, 0, 192);
                    pRxContextA->rxFifos->vb3.Flush(); 
                    break;
                case 0x0:
                    pRxContextA->rxFifos->vb4.SpaceWait(1, pbWorkIndicator);
                    pbTemp = pRxContextA->rxFifos->vb4.Push();
                    memset(pbTemp, 0, 288);
                    pRxContextA->rxFifos->vb4.Flush();                              
                    break;
            }

#ifdef USER_MODE
            TimerStop(&pRxContextA->ri_tOfflineTimings[2]);
#endif
            while (!BB11A_VITERBIDONE_WAIT_EVENT(pRxContextA))
            {
                if (!(*pbWorkIndicator))
                {
                    hr = BB11A_E_FORCE_STOP;
                    break;
                }
                _mm_pause();
            }
            
#ifdef USER_MODE
            TimerStop(&pRxContextA->ri_tOfflineTimings[4]);
#endif
            
            if (pRxContextA->bCRCCorrect)
            {
                hr = BB11A_OK_FRAME;
            }
            else
            {
                hr = BB11A_E_CRC32;
                KdPrint(("FD_NEGATIVE: crc32 error.\n"));
#ifdef USER_MODE
               // __BB11ARxPrintFrame(pRxContextA);
#endif                      
            }
            break;
        } // expect more symbols 
    } while (TRUE); //continue wait symbols

    return hr;
}

//
// rx baseband main function
//
HRESULT BB11ARxFrameDemod(PBB11A_RX_CONTEXT pRxContextA, PSORA_RADIO_RX_STREAM pRxStream)
{
    volatile FLAG *pbWorkIndicator = pRxContextA->ri_pbWorkIndicator;   
    FLAG fReachEnd;
    HRESULT hr = E_FAIL;

#ifdef USER_MODE
    TimerStart(&pRxContextA->ri_tOfflineTimings[0]);
#endif
    pRxContextA->__uRxMode = RM_SYNC;
    pRxContextA->__iSyncPeakPos = -1;

    pRxContextA->rxFifos->g11a_lbSync.Clear();

    do
    {
        // KdPrint(("%08x: fetch restart\n", pbScanPoint));
        SignalBlock block;

        if (pRxContextA->SampleRate == 40)
            hr = FetchDMADataTouchDownSampled40(pRxStream, &fReachEnd, block);
        else if (pRxContextA->SampleRate == 44)
            hr = FetchDMADataTouchDownSampled44(pRxStream, &fReachEnd, block);
        else
        {
            KdPrint(("SampleRate is wrong\n"));
            return E_INVALIDARG;
        }

        if (FAILED(hr))
        {
            KdPrint(("too long waiting.\n"));
            break;
        }

        // Check whether force stopped
        if (*pbWorkIndicator == 0)
        {
            hr = BB11A_E_FORCE_STOP;
            break;
        }
        
        // xmmAdjustSignBit();     // adjust sign bit from 14 to 16
        // ShiftRight2(block);     // Kun: consider harmful!
        // RemoveDC(block);        // Kun: remove dc offset consider harmful
                                   //      don't know why - revisit later

        __BB11APushSignalBlockToSyncFiFo(pRxContextA, block);

        do
        {
            switch (pRxContextA->__uRxMode)
            {
                case RM_SYNC:
                    hr = BB11ARxTrySync(pRxContextA);
                    break;
                case RM_PRE:
                    hr = BB11ARxTryPreambleFFT(pRxContextA);
                    break;
                case RM_HEADER:
                    hr = BB11ARxTryHeader(pRxContextA);
                    break;
                case RM_DATA:
                    hr = BB11ARxTryData(pRxContextA);
                    if (hr == BB11A_OK_FRAME || hr == BB11A_E_CRC32)
					{
                        return hr;
					}
                    break;
            }
        } while (hr == __BB11A_RX_MODE_SWITCH);
            
    } while (SUCCEEDED(hr));

    return hr;
}

void BB11APrepareRx(
        PBB11A_RX_CONTEXT pRxContextA,
        char* pcFrame,
        unsigned int unFrameMaxSize)
{
	pRxContextA->ri_pbFrame = pcFrame;
	pRxContextA->ri_uiFrameMaxSize = unFrameMaxSize;
}
