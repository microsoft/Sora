
__forceinline
void
$_IFFTSSE_$()
{
    __asm
    {
        pcmpeqd xmm7, xmm7;   // xmm7=0xffffffffffffffffffffffffffffffff
        pslld   xmm7, 0x10;   // xmm7=0xffff0000ffff0000ffff0000ffff0000
        pcmpeqd xmm6, xmm6;   // xmm6=0xffffffffffffffffffffffffffffffff
        psrld   xmm6, 0x10;   // xmm6=0x0000ffff0000ffff0000ffff0000ffff
    }
}


__forceinline
void
IFFTSSE4Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
    __asm
    {
        ;//////////////////////////////////////////////////
        ;// IFFT4: pInput
        ;/////////////////////////////////////////////////
        mov     esi,  pInput;

        pshufd  xmm0, [esi], 0xe4;
        
        pcmpeqd xmm5, xmm5;       // xmm5 = 0xfffffffffffffffffffffffffffffff
        pshufd  xmm3, xmm5, 0xe4; // xmm3 = xmm5
        psllq   xmm3, 32;         // xmm3 = 0xFFFFFFFF00000000FFFFFFFF0000000
        pslldq  xmm5, 0x08;       // xmm5 = 0xffffffffffffffff000000000000000
        
        pshufd  xmm4, xmm0, 0x4e; // xmm4 =  Q1  I1  Q0  I0 Q3 I3 Q2 I2
        pxor    xmm0, xmm5;       // xmm0 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
        paddsw  xmm0, xmm4;       // xmm0 =  A-B, A+B
        psllq   xmm5, 0x30;       // xmm5 = 0xff000000000000000000000000000000
        
        pxor    xmm0, xmm5;       // xmm0 = -Q3  I3 Q2 I2 Q1 I1 Q0 I0
        pshufhw xmm0, xmm0, 0xb4; // xmm0 =  I3 -Q3 Q2 I2 Q1 I1 Q0 I0 = upper.4 * j
        
        pshufd  xmm2, xmm0, 0xb1; // xmm2 =  I2  Q2 I3 Q3  I0  Q0 I1 Q1
        pxor    xmm0, xmm3;       // xmm0 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0
        paddsw  xmm0, xmm2;
        
        movdqa  [esi], xmm0;      // output upper 2 2-point DFT
    }   // end of asm
}       // end of IFFTSSE4

__forceinline
void
IFFTSSE8Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
    __asm
    {
        ;//////////////////////////////////////////////////
        ;// IFFT8: pInput
        ;//////////////////////////////////////////////////
        mov     esi,  pInput;

        pshufd  xmm0, [esi], 0xe4; // a
        psraw   xmm0, 0x03;
        pshufd  xmm1, [esi + 16], 0xe4; // b
        psraw   xmm1, 0x03;
        
        pshufd  xmm2, xmm0, 0xe4; // backup xmm0=a
        paddsw  xmm0, xmm1;       // xmm0 = a+b, for 4-point IFFT
        psubsw  xmm2, xmm1;       // xmm2 = a-b
        
        pcmpeqd xmm5, xmm5        // xmm5 = 0xfffffffffffffffffffffffffffffff
        pslldq  xmm5, 0x08;       // xmm5 = 0xffffffffffffffff000000000000000
        pshufd  xmm4, xmm5, 0xe4; // backup xmm5
        pslld   xmm5, 0x10;       // xmm5 = 0xffff0000ffff00000000000000000000
        
        pxor    xmm2, xmm5;       // xmm3 = -Q3  I3 -Q2  I2 Q1 I1 Q0 I0 = (a-b).lower34*j
        pshufhw xmm3, xmm2, 0xb1; // xmm3 =  I3 -Q3  I2 -Q2 Q1 I1 Q0 I0
        
        
        ; // xmm0 and xmm3 store 4-point data
        pshufd  xmm5, xmm3, 0x4e; // xmm5 =  Q1  I1  Q0  I0 Q3 I3 Q2 I2, xmm3 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
        pxor    xmm3, xmm4;       // xmm3 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
        paddsw  xmm3, xmm5;       // xmm3 = xmm3 + xmm5
        
        ; // lower multiplied by wLUT
        
        lea     ebx,  wFFTLUT8;
        pshufd  xmm1, [ebx], 0xe4;
        pshuflw xmm2, xmm1, 0xb1;
        pshufhw xmm2, xmm2, 0xb1;
        pxor    xmm2, xmm6;
        pmaddwd xmm1, xmm3;
        pmaddwd xmm2, xmm3;
        psrad   xmm1, 0x0f;
        psrad   xmm2, 0x0f;
        pand    xmm1, xmm6;
        pand    xmm2, xmm6;
        pslld   xmm2, 0x10;
        por     xmm1, xmm2;       // xmm1=lower
        
        pshufd  xmm3, xmm4, 0xc8; // xmm3 = 0xFFFFFFFF00000000FFFFFFFF00000000
        
        pshufd  xmm2, xmm1, 0xb1; // xmm2 =  I2  Q2 I3 Q3  I0  Q0 I1 Q1
        pxor    xmm1, xmm3;       // xmm1 = -Q3 -I2 Q2 I2 -Q1 -I1 Q0 I0
        paddsw  xmm1, xmm2;       // 4-DFT over        
        
        pshufd  xmm5, xmm0, 0x4e; // xmm5 =  Q1  I1  Q0  I0 Q3 I3 Q2 I2 xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
        pxor    xmm0, xmm4;       // xmm0 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
        paddsw  xmm0, xmm5;       // A-B A+B
        
        psllq   xmm4, 0x30        // xmm4 = 0xff000000000000000000000000000000
        pxor    xmm0, xmm4;       // xmm0 = upper.4 * j
        pshufhw xmm0, xmm0, 0xb4; // xmm0 = I3 -Q3 Q2 I2 Q1 I1 Q0 I0
        
        pshufd  xmm2, xmm0, 0xb1; // xmm2 =  I2  Q2 I3 Q3  I0  Q0 I1 Q1
        pxor    xmm0, xmm3;       // xmm0 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0
        paddsw  xmm0, xmm2;       // 4-IFFT Over
        
        movdqa  [esi], xmm0; // output upper 2 2-point DFT
        movdqa  [esi + 16], xmm1; // output lower2 2-point DFT
    }// end of asm
}// end of IFFTSSE8

// Radix-4
__forceinline
void
IFFTSSE16Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
    __asm
    {
        ;//////////////////////////////////////////////////
        ;// FFT16: pInput  0
        ;/////////////////////////////////////////////////
        mov     esi,  pInput;
        pshufd  xmm0, [esi], 0xe4; // a
        psraw   xmm0, 0x02;
        pshufd  xmm1, [esi + 16], 0xe4; // b
        psraw   xmm1, 0x02;
        pshufd  xmm2, [esi + 32], 0xe4; // c
        psraw   xmm2, 0x02;
        pshufd  xmm3, [esi + 48], 0xe4; // d
        psraw   xmm3, 0x02;
        
        pshufd  xmm4, xmm0, 0xe4;
        pshufd  xmm5, xmm1, 0xe4;
        
        paddsw  xmm0, xmm2; // a+c
        paddsw  xmm1, xmm3; // b+d        
        psubsw  xmm4, xmm2; // a-c
        psubsw  xmm5, xmm3; // b-d
        
        pshufd  xmm2, xmm0, 0xe4; // backup xmm0
        paddsw  xmm0, xmm1;
        movdqa  [esi], xmm0;
        psubsw  xmm2, xmm1;
        
        ;// xmm0 xmm1 xmm3 
        ;// ------------X(4k+2) start---------------------------
        lea     ebx,  wFFTLUT16_2;
        pshufd  xmm0, [ebx], 0xe4;
        
        ;// ----------------complex multiplication start-----------------------------------
        pshuflw xmm1, xmm0, 0xb1; // xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
        pshufhw xmm1, xmm1, 0xb1; // xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
        pxor    xmm1, xmm6; // xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
        pmaddwd xmm0, xmm2; // xmm0 = I3 I2 I1 I0 Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
        pmaddwd xmm1, xmm2; // xmm1 = Q3 Q2 Q1 Q0        
        psrad   xmm0, 0x0f; // Get high 16-bit
        psrad   xmm1, 0x0f; // Get high 16-bit        
        pand    xmm0, xmm6;
        pand    xmm1, xmm6;
        pslld   xmm1, 0x10;        
        por     xmm0, xmm1; // xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0        
        ;// ----------------complex multiplication end-------------------------------------
        ;// ------------X(4k+2) end-----------------------------
        
        movdqa  [esi + 16], xmm0;
        
        ;// xmm0 xmm1 xmm2 xmm3 are no use now
        ;//////////////////////////////////////////////////////////////////////
        
        ;// get (b-d)*j
        pshuflw xmm3, xmm5, 0xb1; // xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
        pshufhw xmm3, xmm3, 0xb1; // xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
        pxor    xmm3, xmm6;       // xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
        
        pshufd  xmm5, xmm4, 0xe4; // backup xmm4=a-c
        
        ;// ------------X(4k+1) start---------------------------
        ;//psubsw  xmm4, xmm3;
        paddsw  xmm4, xmm3;
        
        lea     ebx,  wFFTLUT16_1;
        pshufd  xmm0, [ebx], 0xe4;
        pshuflw xmm1, xmm0, 0xb1; // xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
        pshufhw xmm1, xmm1, 0xb1; // xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
        pxor    xmm1, xmm6; // xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
        pmaddwd xmm0, xmm4; // xmm0 = I3 I2 I1 I0 Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
        pmaddwd xmm1, xmm4; // xmm1 = Q3 Q2 Q1 Q0
        psrad   xmm0, 0x0f; // Get high 16-bit
        psrad   xmm1, 0x0f; // Get high 16-bit
        pand    xmm0, xmm6;
        pand    xmm1, xmm6;
        pslld   xmm1, 0x10;
        por     xmm0, xmm1; // xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
        ;// ------------X(4k+1) end-----------------------------

        ;// xmm1 xmm2 xmm4 are no use now
        ;// ------------X(4k+3) start---------------------------
        //paddsw  xmm5, xmm3;
        psubsw  xmm5, xmm3;
        
        lea     ebx,  wFFTLUT16_3;
        pshufd  xmm1, [ebx], 0xe4;
        pshuflw xmm2, xmm1, 0xb1;
        pshufhw xmm2, xmm2, 0xb1;
        pxor    xmm2, xmm6; // xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
        pmaddwd xmm1, xmm5;
        pmaddwd xmm2, xmm5;
        psrad   xmm1, 0x0f; // Get high 16-bit
        psrad   xmm2, 0x0f; // Get high 16-bit
        pand    xmm1, xmm6;
        pand    xmm2, xmm6;
        pslld   xmm2, 0x10;
        por     xmm1, xmm2;        
        ;// ------------X(4k+3) end-----------------------------        
        
        ;//---------------------------------------------------------------------------------------------------------
        ;// xmm0 xmm1 hold data for FFT4
        ;// xmm2 xmm3 xmm4 xmm5 are no use now
        psraw   xmm0, 0x02;       // xmm0 >>= 2;
        psraw   xmm1, 0x02;       // xmm1 >>= 2;
        
        ;// ------------------4-FFT Prepare start------------------------------------
        pcmpeqd xmm5, xmm5;       // xmm5 = 0xfffffffffffffffffffffffffffffff
        pshufd  xmm3, xmm5, 0xe4; // xmm3 = 0xfffffffffffffffffffffffffffffff
        psllq   xmm3, 32;         // xmm3 = 0xFFFFFFFF00000000FFFFFFFF0000000
        pslldq  xmm5, 0x08;       // xmm5 = 0xffffffffffffffff000000000000000
        pshufd  xmm4, xmm5, 0xe4; // xmm4 = 0xffffffffffffffff000000000000000
        psllq   xmm4, 0x30;       // xmm4 = 0xff00000000000000000000000000000
        ;// ------------------4-FFT Prepare end--------------------------------------
        
        ;// ---xmm0---
        ;// ------------------4-FFT start--------------------------------------------
        pshufd  xmm2, xmm0, 0x4e; // xmm2 =  Q1  I1  Q0  I0 Q3 I3 Q2 I2
        pxor    xmm0, xmm5;       // xmm0 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
        paddsw  xmm0, xmm2;       // xmm0 =  A-B, A+B
        
        pxor    xmm0, xmm4;       // xmm0 = -Q3 I3 Q2 I2 Q1 I1 Q0 I0 = upper.4 * j
        pshufhw xmm0, xmm0, 0xb4; // xmm0 =  I3 -Q3 Q2 I2 Q1 I1 Q0 I0
        
        pshufd  xmm2, xmm0, 0xb1; // xmm2 =  I2  Q2 I3 Q3  I0  Q0 I1 Q1
        pxor    xmm0, xmm3;       // xmm0 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0
        paddsw  xmm0, xmm2;
        ;// ------------------4-FFT end----------------------------------------------
        
        ;// ---xmm1---
        ;// ------------------4-FFT start--------------------------------------------
        pshufd  xmm2, xmm1, 0x4e; // xmm2 =  Q1  I1  Q0  I0 Q3 I3 Q2 I2
        pxor    xmm1, xmm5;       // xmm0 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
        paddsw  xmm1, xmm2;       // xmm0 =  A-B, A+B
        
        pxor    xmm1, xmm4;       // xmm0 = -Q3 I3 Q2 I2 Q1 I1 Q0 I0 = upper.4 * j
        pshufhw xmm1, xmm1, 0xb4; // xmm0 =  I3 -Q3 Q2 I2 Q1 I1 Q0 I0
        
        pshufd  xmm2, xmm1, 0xb1; // xmm2 =  I2  Q2 I3 Q3  I0  Q0 I1 Q1
        pxor    xmm1, xmm3;       // xmm0 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0
        paddsw  xmm1, xmm2;
        ;// ------------------4-FFT end----------------------------------------------

        movdqa  [esi + 32], xmm0;
        movdqa  [esi + 48], xmm1;
        
        ;//------------------------------------------------------------------------------
        pshufd  xmm0, [esi], 0xe4; // a
        pshufd  xmm1, [esi + 16], 0xe4; // b
        
        psraw   xmm0, 0x02;       // xmm0 >>= 2;
        psraw   xmm1, 0x02;       // xmm1 >>= 2;

        ;// ---xmm0---
        ;// ------------------4-FFT start--------------------------------------------
        pshufd  xmm2, xmm0, 0x4e; // xmm2 =  Q1  I1  Q0  I0 Q3 I3 Q2 I2
        pxor    xmm0, xmm5;       // xmm0 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
        paddsw  xmm0, xmm2;       // xmm0 =  A-B, A+B
        
        pxor    xmm0, xmm4;       // xmm0 = -Q3 I3 Q2 I2 Q1 I1 Q0 I0 = upper.4 * j
        pshufhw xmm0, xmm0, 0xb4; // xmm0 =  I3 -Q3 Q2 I2 Q1 I1 Q0 I0
        
        pshufd  xmm2, xmm0, 0xb1; // xmm2 =  I2  Q2 I3 Q3  I0  Q0 I1 Q1
        pxor    xmm0, xmm3;       // xmm0 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0
        paddsw  xmm0, xmm2;
        ;// ------------------4-FFT end----------------------------------------------
        
        ;// ---xmm1---
        ;// ------------------4-FFT start--------------------------------------------
        pshufd  xmm2, xmm1, 0x4e; // xmm2 =  Q1  I1  Q0  I0 Q3 I3 Q2 I2
        pxor    xmm1, xmm5;       // xmm0 = -Q3 -I3 -Q2 -I2 Q1 I1 Q0 I0
        paddsw  xmm1, xmm2;       // xmm0 =  A-B, A+B
        
        pxor    xmm1, xmm4;       // xmm0 = -Q3 I3 Q2 I2 Q1 I1 Q0 I0 = upper.4 * j
        pshufhw xmm1, xmm1, 0xb4; // xmm0 =  I3 -Q3 Q2 I2 Q1 I1 Q0 I0
        
        pshufd  xmm2, xmm1, 0xb1; // xmm2 =  I2  Q2 I3 Q3  I0  Q0 I1 Q1
        pxor    xmm1, xmm3;       // xmm0 = -Q3 -I3 Q2 I2 -Q1 -I1 Q0 I0
        paddsw  xmm1, xmm2;
        ;// ------------------4-FFT end----------------------------------------------
        
        movdqa  [esi], xmm0;
        movdqa  [esi + 16], xmm1;
    }
}