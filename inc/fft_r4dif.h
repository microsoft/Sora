#pragma once

#include <vector128.h>
#include "fft_lut_bitreversal.h"
#include "fft_lut_twiddle.h"

//////////////////////////////////////////////////
// First stage of FFT Radix-4 decimation-in-frequency algorithm 
// with input in normal order and output in bit-reversed order
/////////////////////////////////////////////////
template<int N>
DSP_INLINE1 void FFTSSE(vcs* pInput)
{
    const int INPUT_SHIFT = 2;
    const int OUTPUT_SHIFT = 15;
    const int nArray = N / vcs::size;

    vcs *pi = pInput;
    for (int n = 0; n < nArray/4; n++, pi++)
    {
        vcs a = shift_right(pi[0], INPUT_SHIFT);
        vcs b = shift_right(pi[nArray/4], INPUT_SHIFT);
        vcs c = shift_right(pi[nArray/2], INPUT_SHIFT);
        vcs d = shift_right(pi[nArray/4*3], INPUT_SHIFT);

        vcs ac = saturated_add(a, c);
        vcs bd = saturated_add(b, d);
        vcs a_c = saturated_sub(a, c);
        vcs b_d = saturated_sub(b, d);

        // Calc X(4k) Start
        pi[0] = saturated_add(ac, bd);

        // Calc X(4k+2) Start
        vcs x2 = saturated_sub(ac, bd);
        pi[nArray/4] = mul_shift(x2, FFT_GetTwiddleConst<N, 2>()[n], OUTPUT_SHIFT);

        // Calc X(4k+1) Start
        vcs jb_d = mul_j(b_d);
        vcs x4 = saturated_sub(a_c, jb_d);
        pi[nArray/2] = mul_shift(x4, FFT_GetTwiddleConst<N, 1>()[n], OUTPUT_SHIFT);

        // Calc X(4k+3) Start
        vcs x5 = saturated_add(a_c, jb_d);
        pi[nArray/4*3] = mul_shift(x5, FFT_GetTwiddleConst<N, 3>()[n], OUTPUT_SHIFT);
    }
}

template<int N>
DSP_INLINE1 void
FFTSSEEx (vcs* pInput)
{
    const int nArray = N / vcs::size;

    FFTSSE<N> (pInput);
    FFTSSEEx<N/4> (pInput);
    FFTSSEEx<N/4> (pInput + nArray / 4);
    FFTSSEEx<N/4> (pInput + nArray / 2);
    FFTSSEEx<N/4> (pInput + nArray / 4 * 3);
}

template<>
DSP_INLINE void
FFTSSEEx<4> (vcs* pInput)
{
    const int INPUT_SHIFT = 2;

    vcs xmm3 = vector128_consts::__0xFFFFFFFF00000000FFFFFFFF00000000<vcs>();
    vcs xmm5 = vector128_consts::__0xFFFFFFFFFFFFFFFF0000000000000000<vcs>();
    vcs xmm0 = shift_right(*pInput, INPUT_SHIFT);

    vcs xmm4 = permutate<0x4e>(xmm0); // xmm4 =  Q1  I1  Q0  I0 Q3 I3 Q2 I2
    xmm0 = xor(xmm0, xmm5);             // xmm0 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
    xmm0 = saturated_add(xmm0, xmm4);             // xmm0 =  A-B, A+B'

    xmm5 = vector128_consts::__0xFFFF0000000000000000000000000000<vcs>();
    xmm0 = permutate_high<0xb4>(xmm0);   // xmm0 =  I3 Q3 Q2 I2 Q1 I1 Q0 I0
    xmm0 = xor(xmm0, xmm5);             // xmm0 = -I3 Q3 Q2 I2 Q1 I1 Q0 I0 = upper.4 * -j

    vcs xmm2 = permutate<0xb1>(xmm0); // xmm2 =  I2  Q2 I3 Q3  I0  Q0 I1 Q1
    xmm0 = xor(xmm0, xmm3);             // xmm0 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0

    *pInput = saturated_add(xmm0, xmm2);
}

template<>
DSP_INLINE1 void
FFTSSEEx<8> (vcs* pInput)
{
    const int INPUT_SHIFT = 3;
    const int OUTPUT_SHIFT = 15;

    vcs xmm0 = shift_right(pInput[0], INPUT_SHIFT);     // xmm0 = a
    vcs xmm1 = shift_right(pInput[1], INPUT_SHIFT);		// xmm1 = b

    vcs xmm2 = saturated_sub(xmm0, xmm1);              // xmm2 = a - b
    xmm0 = saturated_add(xmm0, xmm1);					// xmm0 = a + b, for 4-point FFT

    vcs xmm3 = permutate_high<0xb1>(xmm2);            // xmm3 = I3 Q3 I2 Q2 Q1 I1 Q0 I0
    vcs xmm5 = vector128_consts::__0xFFFF0000FFFF00000000000000000000<vcs>();
    xmm3 = xor(xmm3, xmm5);                             // xmm3 = -I3 Q3 -I2 Q2 Q1 I1 Q0 I0 = (a-b).lower34*-j
                                                        // xmm0 and xmm3 store 4-point data
    xmm5 = permutate<0x4e>(xmm3);                     // xmm5 = Q1 I1 Q0 I0 I3 Q3 I2 Q2, xmm3 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    vcs xmm4 = vector128_consts::__0xFFFFFFFFFFFFFFFF0000000000000000<vcs>();
    xmm3 = xor(xmm3, xmm4);                             // xmm3 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
    xmm3 = saturated_add(xmm3, xmm5);                  // xmm3 = xmm3 + xmm5

    xmm1 = mul_shift(xmm3, FFT_GetTwiddleConst<8, 1>()[0], OUTPUT_SHIFT);                 // lower multiplied by wLUT

    xmm3 = permutate<0xc8>(xmm4);                     // xmm3 = 0xFFFFFFFF00000000FFFFFFFF00000000
    xmm2 = permutate<0xb1>(xmm1);                     // xmm2 = I2 Q2 I3 Q3 I0 Q0 I1 Q1
    xmm1 = xor(xmm1, xmm3);                             // xmm1 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0
    xmm1 = saturated_add(xmm1, xmm2);                  // 4-DFT over

    xmm5 = permutate<0x4e>(xmm0);                     // xmm5 = Q1 I1 Q0 I0 Q3 I3 Q2 I2 xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    xmm0 = xor(xmm0, xmm4);                             // xmm0 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
    xmm0 = saturated_add(xmm0, xmm5);                  // A-B A+B

    xmm4 = vector128_consts::__0xFFFF0000000000000000000000000000<vcs>();
                                                        // xmm4 = 0xFFFF0000000000000000000000000000
    xmm0 = permutate_high<0xb4>(xmm0);                // xmm0 = I3 Q3 Q2 I2 Q1 I1 Q0 I0
    xmm0 = xor(xmm0, xmm4);                             // xmm0 = upper.4 * -j

    xmm2 = permutate<0xb1>(xmm0);                     // xmm2 = I2 Q2 I3 Q3 I0  Q0 I1 Q1
    xmm0 = xor(xmm0, xmm3);                             // xmm0 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0
    xmm0 = saturated_add(xmm0, xmm2);                  // 4-FFT Over

    pInput[0] = xmm0;                                   // output upper 2 2-point DFT
    pInput[1] = xmm1;                                   // output lower2 2-point DFT
}

// Note: side-effect: pInput[] will be destroyed after calling
template<int N>
DSP_INLINE1 void FFT(vcs * pInput, vcs * pOutput)
{
    FFTSSEEx<N>(pInput);
    int i;
    for (i = 0; i < N; i++)
        ((COMPLEX16*)pOutput)[i] = ((COMPLEX16*)pInput) [FFTLUTMapTable<N>(i)];

}

// Note: no side-effect
template<int N>
DSP_INLINE1 void FFTSafe(const vcs * pInput, vcs * pOutput)
{
    vcs temp [N / vcs::size];
    memcpy(temp, pInput, sizeof(temp));
    FFT<N>(temp, pOutput);
}
