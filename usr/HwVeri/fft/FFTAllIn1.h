#pragma once
#include "complex.h"

//
// All these functions are weak checked, so make sure that:
//     Input and output buffer are 16-byte aligned
// Radix-4
// Time Complexity: O( N*log(N) )
//

#ifdef __cplusplus
extern "C" {
#endif

void FFT4(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT8(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT16(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT32(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT64(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT128(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT256(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT512(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT1024(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT2048(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT4096(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT8192(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT16384(COMPLEX16* pInput, COMPLEX16* pOutput);
void FFT32768(COMPLEX16* pInput, COMPLEX16* pOutput);

void IFFT4(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT8(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT16(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT32(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT64(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT128(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT256(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT512(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT1024(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT2048(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT4096(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT8192(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT16384(COMPLEX16* pInput, COMPLEX16* pOutput);
void IFFT32768(COMPLEX16* pInput, COMPLEX16* pOutput);

#define FFTNSafe( _N, _pInput, _pOutput)   \
if( ( ((_pInput)  & 0x0000000F) == 0 )     \
&&  ( ((_pOutput) & 0x0000000F) == 0 ) )   \
FFT##_N##(_pInput, _pOutput)

#define IFFTNSafe( _N, _pInput, _pOutput)  \
if( ( ((_pInput)  & 0x0000000F) == 0 )     \
&&  ( ((_pOutput) & 0x0000000F) == 0 ) )   \
IFFT##_N##(_pInput, _pOutput)

#ifdef __cplusplus
}
#endif