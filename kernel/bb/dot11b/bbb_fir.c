/*++
Copyright (c) Microsoft Corporation

Module Name: bbb_fir.c

Abstract: 
    Finite Impulse Response (FIR) filter implemented using SSE 
    inline assembly instructions.

History: 
          7/7/2009: Modified by senxiang.
--*/
#include <emmintrin.h>
#include "vector128.h"
#include "const.h"
#include "bb/bbb.h"
#include "bbb_tx.h"

#pragma warning( disable : 4127 )

const A16 SHORT SSEFilterTaps[TX_FIR_DEPTH+3][8] = 
{
    {1, 1, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 1, 1, 0, 0, 0, 0}, 
    {-1, -1, 0, 0, 1, 1, 0, 0}, 
    {0, 0, -1, -1, 0, 0, 1, 1}, 
    {1, 1, 0, 0, -1, -1, 0, 0}, // 5
    {0, 0, 1, 1, 0, 0, -1, -1}, 
    {-1, -1, 0, 0, 1, 1, 0, 0}, 
    {0, 0, -1, -1, 0, 0, 1, 1}, 
    {2, 2, 0, 0, -1, -1, 0, 0}, 
    {0, 0, 2, 2, 0, 0, -1, -1}, // 10
    {-3, -3, 0, 0, 2, 2, 0, 0}, 
    {0, 0, -3, -3, 0, 0, 2, 2}, 
    {5, 5, 0, 0, -3, -3, 0, 0}, 
    {0, 0, 5, 5, 0, 0, -3, -3}, 
    {-11, -11, 0, 0, 5, 5, 0, 0},  // 15
    {0, 0, -11, -11, 0, 0, 5, 5}, 
    {54, 54, 0, 0, -11, -11, 0, 0}, 
    {128, 128, 54, 54, 0, 0, -11, -11}, 
    {163, 163, 128, 128, 54, 54, 0, 0}, 
    {128, 128, 163, 163, 128, 128, 54, 54},  // 20
    {54, 54, 128, 128, 163, 163, 128, 128}, 
    {0, 0, 54, 54, 128, 128, 163, 163}, 
    {-11, -11, 0, 0, 54, 54, 128, 128}, 
    {0, 0, -11, -11, 0, 0, 54, 54}, 
    {5, 5, 0, 0, -11, -11, 0, 0},  // 25
    {0, 0, 5, 5, 0, 0, -11, -11}, 
    {-3, -3, 0, 0, 5, 5, 0, 0}, 
    {0, 0, -3, -3, 0, 0, 5, 5}, 
    {2, 2, 0, 0, -3, -3, 0, 0}, 
    {0, 0, 2, 2, 0, 0, -3, -3}, // 30
    {-1, -1, 0, 0, 2, 2, 0, 0}, 
    {0, 0, -1, -1, 0, 0, 2, 2}, 
    {1, 1, 0, 0, -1, -1, 0, 0}, 
    {0, 0, 1, 1, 0, 0, -1, -1}, 
    {-1, -1, 0, 0, 1, 1, 0, 0},  // 35
    {0, 0, -1, -1, 0, 0, 1, 1}, 
    {1, 1, 0, 0, -1, -1, 0, 0}, 
    {0, 0, 1, 1, 0, 0, -1, -1}, 
    {0, 0, 0, 0, 1, 1, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 1, 1},  // 40
};

#define FIR_TEMP_SIZE ((TX_FIR_DEPTH + 3 ) * 8)
static A16 SHORT    FIRTemp[FIR_TEMP_SIZE];
static KSPIN_LOCK   FIRLock;

void FIRInit()
{
    KeInitializeSpinLock(&FIRLock);
}

HRESULT 
FIR37SSE_INLINE(
    PCOMPLEX8   pSrc, 
    const SHORT *     pTaps, 
    DWORD       dwBlockCnt, 
    PSHORT      pTempOutput, 
    PCOMPLEX8   pOutput
    );

HRESULT 
FIR37SSE_INTRINSIC(
    PCOMPLEX8   pSrc, 
    const SHORT *     pTaps, 
    DWORD       dwBlockCnt, 
    PSHORT      pTempOutput, 
    PCOMPLEX8   pOutput
    );

HRESULT BB11BPMDSpreadFIR4SSE(
    IN PCOMPLEX8  pcSrc, 
    IN UINT       uiInputSize, 
    IN PCOMPLEX8  pcDest, 
    IN PULONG     puiOutputSize
    )
{
    KIRQL OldIrq;
    if (uiInputSize & 0x7)
    {
        return E_FAIL;
    }
    KeAcquireSpinLock(&FIRLock, &OldIrq);
    memset(FIRTemp, 0, FIR_TEMP_SIZE * sizeof(SHORT));
    FIR37SSE_INTRINSIC(pcSrc, SSEFilterTaps[0], (uiInputSize >> 3), FIRTemp, pcDest);
    KeReleaseSpinLock(&FIRLock, OldIrq);
    *(puiOutputSize) = uiInputSize;

    return S_OK;
}

#if !defined(_M_X64)
HRESULT BB11BPMDSpreadFIR4ASM(
    IN PCOMPLEX8  pcSrc, 
    IN UINT       uiInputSize, 
    IN PCOMPLEX8  pcDest, 
    IN PULONG     puiOutputSize
    )
{
    KIRQL OldIrq;
    if (uiInputSize & 0x7)
    {
        return E_FAIL;
    }
    KeAcquireSpinLock(&FIRLock, &OldIrq);
    memset(FIRTemp, 0, FIR_TEMP_SIZE * sizeof(SHORT));
    FIR37SSE_INLINE(pcSrc, SSEFilterTaps[0], (uiInputSize >> 3), FIRTemp, pcDest);
    KeReleaseSpinLock(&FIRLock, OldIrq);
    *(puiOutputSize) = uiInputSize;

    return S_OK;
}

#pragma warning( disable : 4127 )

HRESULT FIR37SSE_INLINE(
    PCOMPLEX8   pSrc, 
    const SHORT *     pTaps, 
    DWORD       dwBlockCnt, 
    PSHORT      pTempOutput, 
    PCOMPLEX8   pOutput
    )
{
    HRESULT hRes = S_OK;
    do
    {
        //////////////////////////////////////////////////////////////////////////
        // Make sure each buffer is 16-byte aligned!!!
        //////////////////////////////////////////////////////////////////////////

        if (((UPOINTER)pSrc & 0x0000000F ) &&
            ((UPOINTER)pTaps & 0x0000000F) &&
            ((UPOINTER)pTempOutput & 0x0000000F ) &&
            ((UPOINTER)pOutput & 0x0000000F ))
        {
            hRes = E_FAIL;
            break;
        }
        
        __asm{
            ;// set up pointers for
            mov esi, pSrc;                  // input IQ
            
            mov ebx, pOutput;               // output
            mov eax, dwBlockCnt;            // how many blocks (16-bytes, 8 IQs)

InnerBlockLoop:
			mov edx, pTaps;                 // taps
            mov edi, pTempOutput;           // temp output buffer

            movdqa xmm4, [esi + 16];	    //load input[n~n+7]
            pshufd xmm1, xmm4, 0xE4;        //save a copy of xmm4

            pxor   xmm2, xmm2;              //zero xmm2
            pcmpgtb xmm2, xmm4;             //xmm2 contains sign extension

            punpcklbw xmm4, xmm2;           //unpack low IQ
            punpckhbw xmm1, xmm2;           //unpack high IQ
            ;// now, xmm4 and xmm1 contain the data

            movdqa xmm2, xmm4;              // copy xmm4 to xmm2
            ;//######################################################
            ;// 0
            ;//######################################################
            pmullw xmm4, [edx];             // xmm0 = input[0-3] * c_0
            paddsw xmm4, [edi];             // save output_temp[0]

            ;//######################################################
            ;// 1
            ;//######################################################
            pshufd xmm5, xmm2, 0xE4;        // recover xmm4
            pmullw xmm5, [edx + 16];        // xmm5 = input[0-3] * c_1
            paddsw xmm5, [edi + 16];        // save output_temp[1]

            ;//######################################################
            ;// 2
            ;//######################################################
            pshufd xmm6, xmm2, 0xE4;        // recover xmm4
            pmullw xmm6, [edx + 32];        // xmm6 = input[0-3] * c_2
            paddsw xmm6, [edi + 32];        // save output_temp[2]

            ;//######################################################
            ;// 3
            ;//######################################################
            pshufd xmm7, xmm2, 0xE4;         // recover xmm4
            pmullw xmm7, [edx + 48];         // xmm7 = input[0-3] * c_3
            paddsw xmm7, [edi + 48];         // save output_temp[3]

            ;//######################################################
            ;// now unpack xmm4 xmm5 xmm6 xmm7 to get the output[0~3]
            ;//######################################################
            pshufd xmm0, xmm4, 0xE4;         // move xmm4 to xmm0
            punpckldq xmm0, xmm6;            //xmm0 = 6.1 4.1 6.0 4.0
            punpckhdq xmm4, xmm6;            //xmm4 = 6.3 4.3 6.2 4.2
            paddsw xmm4, xmm0;               //xmm4 = 6.1+6.3 4.1+4.3 6.0+6.2 4.0+4.2
            pshufd xmm0, xmm5, 0xE4;         // move xmm5 to xmm0
            punpckldq xmm0, xmm7;            // xmm0 = 7.1 5.1 7.0 5.0
            punpckhdq xmm5, xmm7;            // xmm5 = 7.3 5.3 7.2 5.2
            paddsw xmm5, xmm0;               // xmm5 = 7.1+7.3 5.1+5.3 7.0+7.2 5.0+5.2
            pshufd xmm6, xmm4, 0xE4;         // move xmm4 to xmm6
            punpckldq xmm6, xmm5;            // xmm6 = 7.0+7.2  6.0+6.2  5.0+5.2  4.0+4.2
            punpckhdq xmm4, xmm5;            // xmm4 = 7.1+7.3  6.1+6.3  5.1+5.3  4.1+4.3
            paddsw xmm4, xmm6;               // xmm4 = out[3]    out[2]     out[1]    out[0]
            pshufd xmm3, xmm4, 0xE4;		 // xmm3 contains the result

            ;//######################################################
            ;// now calculate the rest values
            ;//######################################################

            ;//######################################################
            ;// 4-35
            ;//######################################################
			mov ecx, 32;
loop1:
			movdqa xmm4, xmm2; 
			pmullw xmm4, [edx + 64];               // xmm4 = input[0-3] * c_x
            paddsw xmm4, [edi + 64];               // xmm4 = xmm4 + TempBuffer[x]
            movdqa [edi], xmm4;                    // save to TempBuffer[x]
			add edi, 16;
			add edx, 16;
			dec ecx;
			jnz loop1;


            ;//######################################################
            ;// 36-39 special case
            ;//######################################################
            movdqa xmm4, xmm2;               // recover xmm4
            pmullw xmm4, [edx+64];               // xmm4 = input[0-3] * c_36
            movdqa [edi], xmm4;               // save to TempBuffer[32]

            ;//######################################################
            ;// 37
            ;//######################################################
            movdqa xmm4, xmm2;               // recover xmm4
            pmullw xmm4, [edx+80];               // xmm4 = input[0-3] * c_36
            movdqa [edi+16], xmm4;               // save to TempBuffer[32]

            ;//######################################################
            ;// 38
            ;//######################################################
            movdqa xmm4, xmm2;               // recover xmm4
            pmullw xmm4, [edx+96];               // xmm4 = input[0-3] * c_36
            movdqa [edi+32], xmm4;               // save to TempBuffer[32]

            ;//######################################################
            ;// 39
            ;//######################################################
            movdqa xmm4, xmm2;               // recover xmm4
            pmullw xmm4, [edx+112];               // xmm4 = input[0-3] * c_36
            movdqa [edi+48], xmm4;               // save to TempBuffer[32]

			// restore edx; edi;
			mov edx, pTaps;                 // taps
            mov edi, pTempOutput;           // temp output buffer

            movdqa xmm2, xmm1;        //save a copy of xmm1

            ;//######################################################
            ;// 0
            ;//######################################################
            pmullw xmm1, [edx];             // xmm1 = input[4-7] * c_0
            paddsw xmm1, [edi];             // save output_temp[0]
            
			;//######################################################
            ;// 1
            ;//######################################################
            pshufd xmm5, xmm2, 0xE4;        // recover xmm1
            pmullw xmm5, [edx + 16];        // xmm5 = input[4-7] * c_1
            paddsw xmm5, [edi + 16];        // save output_temp[1]
            
			;//######################################################
            ;// 2
            ;//######################################################
            pshufd xmm6, xmm2, 0xE4;        // recover xmm1
            pmullw xmm6, [edx + 32];        // xmm6 = input[4-7] * c_2
            paddsw xmm6, [edi + 32];        // save output_temp[2]
            
			;//######################################################
            ;// 3
            ;//######################################################
            pshufd xmm7, xmm2, 0xE4;         // recover xmm1
            pmullw xmm7, [edx + 48];         // xmm7 = input[4-7] * c_3
            paddsw xmm7, [edi + 48];         // save output_temp[3]
            
			;//######################################################
            ;// now unpack xmm1 xmm5 xmm6 xmm7 to get the output[4~7]
            ;//######################################################
            pshufd xmm0, xmm1, 0xE4;         // move xmm1 to xmm0
            punpckldq xmm0, xmm6;            // xmm0 = 6.1 1.1 6.0 1.0
            punpckhdq xmm1, xmm6;            // xmm1 = 6.3 1.3 6.2 1.2
            paddsw xmm1, xmm0;               // xmm1 = 6.1+6.3 1.1+1.3 6.0+6.2 1.0+1.2
            pshufd xmm0, xmm5, 0xE4;         // move xmm5 to xmm0
            punpckldq xmm0, xmm7;            // xmm0 = 7.1 5.1 7.0 5.0
            punpckhdq xmm5, xmm7;            // xmm5 = 7.3 5.3 7.2 5.2
            paddsw xmm5, xmm0;               // xmm5 = 7.1+7.3 5.1+5.3 7.0+7.2 5.0+5.2
            pshufd xmm6, xmm1, 0xE4;         // move xmm1 to xmm6
            punpckldq xmm6, xmm5;            // xmm6 = 7.0+7.2  6.0+6.2  5.0+5.2  1.0+1.2
            punpckhdq xmm1, xmm5;            // xmm1 = 7.1+7.3  6.1+6.3  5.1+5.3  1.1+1.3
            paddsw xmm1, xmm6;               // xmm1 = out[3]    out[2]     out[1]    out[0]

            psraw xmm3, 8;
            psraw xmm1, 8;
            packsswb xmm3, xmm1;
            movntdq [ebx], xmm3;
            add ebx, 0x10;

            ;//######################################################
            ;// now calculate the rest values
            ;//######################################################

			;//######################################################
            ;// 4-35
            ;//######################################################
			mov ecx, 32;
loop2:
			movdqa xmm4, xmm2; 
			pmullw xmm4, [edx + 64];               // xmm4 = input[0-3] * c_x
            paddsw xmm4, [edi + 64];               // xmm4 = xmm4 + TempBuffer[x]
            movdqa [edi], xmm4;                    // save to TempBuffer[x]
			add edi, 16;
			add edx, 16;
			dec ecx;
			jnz loop2;


            ;//######################################################
            ;// 36-39 special case
            ;//######################################################
            movdqa xmm4, xmm2;               // recover xmm4
            pmullw xmm4, [edx+64];               // xmm4 = input[0-3] * c_36
            movdqa [edi], xmm4;               // save to TempBuffer[32]

            ;//######################################################
            ;// 37
            ;//######################################################
            movdqa xmm4, xmm2;               // recover xmm4
            pmullw xmm4, [edx+80];               // xmm4 = input[0-3] * c_36
            movdqa [edi+16], xmm4;               // save to TempBuffer[32]

            ;//######################################################
            ;// 38
            ;//######################################################
            movdqa xmm4, xmm2;               // recover xmm4
            pmullw xmm4, [edx+96];               // xmm4 = input[0-3] * c_36
            movdqa [edi+32], xmm4;               // save to TempBuffer[32]

            ;//######################################################
            ;// 39
            ;//######################################################
            movdqa xmm4, xmm2;               // recover xmm4
            pmullw xmm4, [edx+112];               // xmm4 = input[0-3] * c_36
            movdqa [edi+48], xmm4;               // save to TempBuffer[32]


			;//######################################################
            ;// continue load data in current rx_descriptor
            ;//######################################################
            add esi, 16;    // move to next input data
            dec eax;
            jnz InnerBlockLoop;
        }// end of asm
        
    } while(FALSE);
    return hRes;
}
#endif

#define __MULLO_ADDS(x, y, pTags, pTempOutput) \
    do {\
        x = _mm_shuffle_epi32(y, 0xE4); \
        x = _mm_mullo_epi16(x, *(__m128i*)(pTaps)); \
        x = _mm_adds_epi16(x, *(__m128i*)(pTempOutput)); \
    }while(FALSE);

#define __CAL_LEFT_VALUE(d, s, pTaps, pTempSrc, pTempOut) \
    do { \
        d = _mm_shuffle_epi32(s, 0xE4); \
        d = _mm_mullo_epi16(d, *(__m128i*)(pTaps));  \
        d = _mm_adds_epi16(d, *(__m128i*)(pTempSrc));   \
        _mm_store_si128((__m128i*)(pTempOut), d); \
    } while(FALSE);

#define __UNPACK_ADDS(srcdst, temp, src) \
    do { \
        temp = _mm_shuffle_epi32(srcdst, 0xE4); \
        temp = _mm_unpacklo_epi32(temp, src); \
        srcdst = _mm_unpackhi_epi32(srcdst, src); \
        srcdst = _mm_adds_epi16(srcdst, temp); \
    } while(FALSE);


HRESULT 
FIR37SSE_INTRINSIC(
    PCOMPLEX8   pSrc, 
    const SHORT *     pTaps, 
    DWORD       dwBlockCnt, 
    PSHORT      pTempOutput, 
    PCOMPLEX8   pOutput
    )
{
    HRESULT hRes = S_OK;
    register __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    int i;
    if (((UPOINTER)pSrc & 0x0000000F ) &&
        ((UPOINTER)pTaps & 0x0000000F) &&
        ((UPOINTER)pTempOutput & 0x0000000F ) &&
        ((UPOINTER)pOutput & 0x0000000F ))
    {
        hRes = E_FAIL;
        return hRes;
    }
    do 
    {
        xmm4 = _mm_shuffle_epi32(*(__m128i*)((char*)pSrc + 16), 0xE4);
        xmm1 = _mm_shuffle_epi32(xmm4, 0xE4);
        // Note: do not use _mm_xor_si128(x,x) because of build error in C++ compiler
        xmm2 = _mm_setzero_si128(); 
        xmm2 = _mm_cmpgt_epi8(xmm2, xmm4);
        xmm4 = _mm_unpacklo_epi8(xmm4, xmm2);
        xmm1 = _mm_unpackhi_epi8(xmm1, xmm2);

        xmm2 = _mm_shuffle_epi32(xmm4, 0xE4);
        
        xmm4 = _mm_mullo_epi16(xmm4, *(__m128i*)pTaps);
        xmm4 = _mm_adds_epi16(xmm4, *(__m128i*)pTempOutput);
        __MULLO_ADDS(xmm5, xmm2, (char*)pTaps + 16, (char*)pTempOutput + 16);
        __MULLO_ADDS(xmm6, xmm2, (char*)pTaps + 32, (char*)pTempOutput + 32);
        __MULLO_ADDS(xmm7, xmm2, (char*)pTaps + 48, (char*)pTempOutput + 48);
        
        xmm0 = _mm_shuffle_epi32(xmm4, 0xE4);
        xmm0 = _mm_unpacklo_epi32(xmm0, xmm6);
        xmm4 = _mm_unpackhi_epi32(xmm4, xmm6);
        xmm4 = _mm_adds_epi16(xmm4, xmm0);
        xmm0 = _mm_shuffle_epi32(xmm5, 0xE4);
        xmm0 = _mm_unpacklo_epi32(xmm0, xmm7);
        xmm5 = _mm_unpackhi_epi32(xmm5, xmm7);
        xmm5 = _mm_adds_epi16(xmm5, xmm0);
        xmm6 = _mm_shuffle_epi32(xmm4, 0xE4);
        xmm6 = _mm_unpacklo_epi32(xmm6, xmm5);
        xmm4 = _mm_unpackhi_epi32(xmm4, xmm5);
        xmm4 = _mm_adds_epi16(xmm4, xmm6);
        xmm3 = _mm_shuffle_epi32(xmm4, 0xE4);

        for (i = 0; i < 29; i += 4)
        {
            __CAL_LEFT_VALUE(xmm4, xmm2, 
                (char*)pTaps + 64 + i * 16,
                (char*)pTempOutput + 64 + i * 16, 
                (char*)pTempOutput + i * 16);
            __CAL_LEFT_VALUE(xmm5, xmm2, 
                (char*)pTaps + 80 + i * 16, 
                (char*)pTempOutput + 80 + i * 16, 
                (char*)pTempOutput + 16 + i * 16);
            __CAL_LEFT_VALUE(xmm6, xmm2, 
                (char*)pTaps + 96 + i * 16, 
                (char*)pTempOutput + 96 + i * 16, 
                (char*)pTempOutput + 32 + i * 16);
            __CAL_LEFT_VALUE(xmm7, xmm2, 
                (char*)pTaps + 112 + i * 16, 
                (char*)pTempOutput + 112 + i * 16, (char*)pTempOutput + 48 + i * 16);
        }
        xmm4 = _mm_shuffle_epi32(xmm2, 0xE4); 
        xmm4 = _mm_mullo_epi16(xmm4, *(__m128i*)((char*)pTaps + 576));  
        _mm_store_si128((__m128i*)((char*)pTempOutput + 512), xmm4); 

        xmm5 = _mm_shuffle_epi32(xmm2, 0xE4); 
        xmm5 = _mm_mullo_epi16(xmm4, *(__m128i*)((char*)pTaps + 592));  
        _mm_store_si128((__m128i*)((char*)pTempOutput + 528), xmm5); 

        xmm6 = _mm_shuffle_epi32(xmm2, 0xE4); 
        xmm6 = _mm_mullo_epi16(xmm4, *(__m128i*)((char*)pTaps + 608));  
        _mm_store_si128((__m128i*)((char*)pTempOutput + 544), xmm6); 

        xmm7 = _mm_shuffle_epi32(xmm2, 0xE4); 
        xmm7 = _mm_mullo_epi16(xmm7, *(__m128i*)((char*)pTaps + 624));  
        _mm_store_si128((__m128i*)((char*)pTempOutput + 560), xmm7); 

        xmm2 = _mm_shuffle_epi32(xmm1, 0xE4);
        xmm1 = _mm_mullo_epi16(xmm1, *(__m128i*)pTaps);
        xmm1 = _mm_adds_epi16(xmm1, *(__m128i*)pTempOutput);

        xmm5 = _mm_shuffle_epi32(xmm2, 0xE4);
        xmm5 = _mm_mullo_epi16(xmm5, *(__m128i*)((char*)pTaps + 16));
        xmm5 = _mm_adds_epi16(xmm5, *(__m128i*)((char*)pTempOutput + 16));

        xmm6 = _mm_shuffle_epi32(xmm2, 0xE4);
        xmm6 = _mm_mullo_epi16(xmm6, *(__m128i*)((char*)pTaps + 32));
        xmm6 = _mm_adds_epi16(xmm6, *(__m128i*)((char*)pTempOutput + 32));

        xmm7 = _mm_shuffle_epi32(xmm2, 0xE4);
        xmm7 = _mm_mullo_epi16(xmm7, *(__m128i*)((char*)pTaps + 48));
        xmm7 = _mm_adds_epi16(xmm7, *(__m128i*)((char*)pTempOutput + 48));
        //unpack xmm1, xmm5 xmm6, xmm7 

        __UNPACK_ADDS(xmm1, xmm0, xmm6);
        __UNPACK_ADDS(xmm5, xmm0, xmm7);
        __UNPACK_ADDS(xmm1, xmm6, xmm5);

        xmm3 = _mm_srai_epi16(xmm3, 8);
        xmm1 = _mm_srai_epi16(xmm1, 8);
        xmm3 = _mm_packs_epi16(xmm3, xmm1);
        _mm_stream_si128((__m128i*)pOutput, xmm3);
        pOutput = (PCOMPLEX8)((char*)pOutput + 16);

        for (i = 0; i < 29; i += 4)
        {
            __CAL_LEFT_VALUE(xmm1, xmm2, 
                (char*)pTaps + 64 + i * 16,
                (char*)pTempOutput + 64 + i * 16, 
                (char*)pTempOutput + i * 16);
            __CAL_LEFT_VALUE(xmm5, xmm2, 
                (char*)pTaps + 80 + i * 16, 
                (char*)pTempOutput + 80 + i * 16, 
                (char*)pTempOutput + 16 + i * 16);
            __CAL_LEFT_VALUE(xmm6, xmm2, 
                (char*)pTaps + 96 + i * 16, 
                (char*)pTempOutput + 96 + i * 16, 
                (char*)pTempOutput + 32 + i * 16);
            __CAL_LEFT_VALUE(xmm7, xmm2, 
                (char*)pTaps + 112 + i * 16, 
                (char*)pTempOutput + 112 + i * 16, (char*)pTempOutput + 48 + i * 16);
        }

        xmm1 = _mm_shuffle_epi32(xmm2, 0xE4); 
        xmm1 = _mm_mullo_epi16(xmm1, *(__m128i*)((char*)pTaps + 576));  
        _mm_store_si128((__m128i*)((char*)pTempOutput + 512), xmm1); 

        xmm5 = _mm_shuffle_epi32(xmm2, 0xE4); 
        xmm5 = _mm_mullo_epi16(xmm4, *(__m128i*)((char*)pTaps + 592));  
        _mm_store_si128((__m128i*)((char*)pTempOutput + 528), xmm5); 

        xmm6 = _mm_shuffle_epi32(xmm2, 0xE4); 
        xmm6 = _mm_mullo_epi16(xmm4, *(__m128i*)((char*)pTaps + 608));  
        _mm_store_si128((__m128i*)((char*)pTempOutput + 544), xmm6); 

        xmm7 = _mm_shuffle_epi32(xmm2, 0xE4); 
        xmm7 = _mm_mullo_epi16(xmm7, *(__m128i*)((char*)pTaps + 624));  
        _mm_store_si128((__m128i*)((char*)pTempOutput + 560), xmm7); 
        
        pSrc = (PCOMPLEX8)((char*)pSrc + 16);
        dwBlockCnt--;
    } while (dwBlockCnt != 0);
    return hRes;
}
