#include "sora.h"
#include "vector128.h"
#include "_user_mode_ext.h"

#ifndef INLINE_ASM
#pragma error "inline assembly implementation of rx stream"
#endif 
 
__inline HRESULT __WaitNewSignals(
                    IN PRX_BLOCK       pScanPoint, 
                    IN USHORT          uRetries,
                    IN ULONG           VStreamMask,
                    OUT FLAG           *fReachEnd
                    )
{
    ULONG   uSpinCount  = uRetries * 2;
    *fReachEnd = 0;
    while(uSpinCount != 0)
    {    
        if (SORA_C_RXBUF_IS_VALID_EX(pScanPoint, VStreamMask))
        {
            return S_OK;
        }
        else
        {
            _mm_clflush(pScanPoint); 
            _mm_mfence(); 
            *fReachEnd = 1;
        }
        uSpinCount--;
    }
    
    return E_FETCH_SIGNAL_HW_TIMEOUT;
}

HRESULT 
SORAAPI
SoraCheckSignalBlock(
    IN PRX_BLOCK       pScanPoint, 
    IN ULONG           VStreamMask,
    IN USHORT          uRetries, 
    OUT FLAG           *fReachEnd)
{
    HRESULT hr = S_OK;
    hr = __WaitNewSignals(pScanPoint, uRetries, VStreamMask, fReachEnd);
    if (SUCCEEDED(hr))
    {
        SORA_C_INVALIDATE_SIGNAL_BLOCK_EX(pScanPoint, VStreamMask);
    }
    return hr;
}

void __SnapShotSignalBlockWithDesc(IN PRX_BLOCK pScanPoint, IN PVOID Buffer)
{
    vcs *src = (vcs *)pScanPoint;
    vcs *dst = (vcs *)Buffer;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
}

void __DumpSignalBlockWithDesc(IN PRX_BLOCK pScanPoint, IN PVOID Buffer)
{
    __SnapShotSignalBlockWithDesc(pScanPoint, Buffer);
    *((PUCHAR)Buffer) = 0x01;
}

__inline void __GenRadioRxStream(
    OUT PSORA_RADIO_RX_STREAM   pRxStream, 
    IN PUCHAR                   pInput, 
    IN ULONG                    Size,
    IN ULONG                    VStreamMask)
{
    pRxStream->__pStartPt   = pInput;
    pRxStream->__nRxBufSize = Size;
    pRxStream->__pEndPt     = pRxStream->__pStartPt + pRxStream->__nRxBufSize;
    pRxStream->__pScanPt    = pInput;
    pRxStream->__VStreamMask= VStreamMask;
}

VOID
SORAAPI 
SoraGenRadioRxStreamOffline(
    OUT PSORA_RADIO_RX_STREAM   pRxStream, 
    IN PUCHAR                   pInput, 
    IN ULONG                    Size)
{
    __GenRadioRxStream(pRxStream, pInput, Size, 0x1);
}

#ifdef USER_MODE
HRESULT
SORAAPI 
SoraURadioAllocRxStream(
    OUT PSORA_RADIO_RX_STREAM   pRxStream, 
    IN ULONG                    RadioNo,
    IN PUCHAR                   pInput, 
    IN ULONG                    Size
    )
{
	extern HRESULT
	SoraURadioAllocVStreamMask(
		IN ULONG RadioNo,
		OUT PULONG pVStreamMask);
    ULONG VStreamMask;
    HRESULT hr = SoraURadioAllocVStreamMask(RadioNo, &VStreamMask);
    if (FAILED(hr))
    {
        // Set virtual stream mask to zero, to minimize the future misuse destruction by app user
        VStreamMask = 0;
    }
    __GenRadioRxStream(pRxStream, pInput, Size, VStreamMask);
    return hr;
}

void
SORAAPI
SoraURadioReleaseRxStream(
    IN PSORA_RADIO_RX_STREAM    pRxStream,
    IN ULONG                    RadioNo
    )
{
	extern HRESULT
	SoraURadioReleaseVStreamMask(
		IN ULONG RadioNo,
		IN ULONG VStreamMask);
    SoraURadioReleaseVStreamMask(RadioNo, pRxStream->__VStreamMask);
}
#endif
