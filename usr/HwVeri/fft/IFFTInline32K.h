__forceinline void
IFFTSSE32 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT32
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_32_0:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 32], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 64], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 96], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT32_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 32], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT32_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT32_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 64], xmm0;
    movdqa[esi + 96], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 32;
    jne L_32_0;
  }				// end of asm
}				// end of IFFTSSE32
__forceinline void
IFFTSSE64 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT64
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_64_1:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 64], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 128], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 192], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT64_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 64], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT64_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT64_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 128], xmm0;
    movdqa[esi + 192], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 64;
    jne L_64_1;
  }				// end of asm
}				// end of IFFTSSE64
__forceinline void
IFFTSSE128 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT128
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_128_2:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 128], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 256], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 384], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT128_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 128], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT128_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT128_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 256], xmm0;
    movdqa[esi + 384], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 128;
    jne L_128_2;
  }				// end of asm
}				// end of IFFTSSE128
__forceinline void
IFFTSSE256 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT256
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_256_3:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 256], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 512], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 768], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT256_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 256], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT256_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT256_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 512], xmm0;
    movdqa[esi + 768], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 256;
    jne L_256_3;
  }				// end of asm
}				// end of IFFTSSE256
__forceinline void
IFFTSSE512 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT512
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_512_4:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 512], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 1024], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 1536], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT512_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 512], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT512_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT512_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 1024], xmm0;
    movdqa[esi + 1536], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 512;
    jne L_512_4;
  }				// end of asm
}				// end of IFFTSSE512
__forceinline void
IFFTSSE1024 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT1024
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_1024_5:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 1024], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 2048], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 3072], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT1024_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 1024], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT1024_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT1024_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 2048], xmm0;
    movdqa[esi + 3072], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 1024;
    jne L_1024_5;
  }				// end of asm
}				// end of IFFTSSE1024
__forceinline void
IFFTSSE2048 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT2048
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_2048_6:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 2048], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 4096], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 6144], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT2048_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 2048], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT2048_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT2048_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 4096], xmm0;
    movdqa[esi + 6144], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 2048;
    jne L_2048_6;
  }				// end of asm
}				// end of IFFTSSE2048
__forceinline void
IFFTSSE4096 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT4096
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_4096_7:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 4096], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 8192], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 12288], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT4096_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 4096], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT4096_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT4096_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 8192], xmm0;
    movdqa[esi + 12288], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 4096;
    jne L_4096_7;
  }				// end of asm
}				// end of IFFTSSE4096
__forceinline void
IFFTSSE8192 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT8192
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_8192_8:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 8192], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 16384], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 24576], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT8192_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 8192], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT8192_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT8192_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 16384], xmm0;
    movdqa[esi + 24576], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 8192;
    jne L_8192_8;
  }				// end of asm
}				// end of IFFTSSE8192
__forceinline void
IFFTSSE16384 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT16384
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_16384_9:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 16384], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 32768], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 49152], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT16384_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 16384], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT16384_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT16384_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 32768], xmm0;
    movdqa[esi + 49152], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 16384;
    jne L_16384_9;
  }				// end of asm
}				// end of IFFTSSE16384
__forceinline void
IFFTSSE32768 (COMPLEX16 * pInput)
{
  __asm
  {
    ;				//////////////////////////////////////////////////
    ;				// IFFT32768
    ;				/////////////////////////////////////////////////
    mov esi, pInput;
    xor ecx, ecx;
  L_32768_10:
    pshufd xmm0,[esi], 0xe4;	// a
    psraw xmm0, 0x02;
    pshufd xmm1,[esi + 32768], 0xe4;	// b
    psraw xmm1, 0x02;
    pshufd xmm2,[esi + 65536], 0xe4;	// c
    psraw xmm2, 0x02;
    pshufd xmm3,[esi + 98304], 0xe4;	// d
    psraw xmm3, 0x02;
    pshufd xmm4, xmm0, 0xe4;	// backup xmm0
    pshufd xmm5, xmm1, 0xe4;	// backup xmm1
    paddsw xmm0, xmm2;		// a + c
    paddsw xmm1, xmm3;		// b + d
    psubsw xmm4, xmm2;		// a - c
    psubsw xmm5, xmm3;		// b - d
    pshufd xmm2, xmm0, 0xe4;	// backup xmm0
    paddsw xmm0, xmm1;
    movdqa[esi], xmm0;
    psubsw xmm2, xmm1;
    ;				// xmm0 xmm1 and xmm3 are no use now
    ;				// ------------ X(4k+2) Start -------------------
    lea ebx, wFFTLUT32768_2;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm2;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm2;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    movdqa[esi + 32768], xmm0;
    ;				// ------------ X(4k+2) End -------------------
    ;				// get (b-d)*j
    pshuflw xmm3, xmm5, 0xb1;	// xmm5 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm3, xmm3, 0xb1;	// xmm3 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm3, xmm6;		// xmm3 =(b-d)*j = I3 -Q3 I2 -Q2 I1 -Q1 I0 -Q0 xmm4 = (a-c)
    pshufd xmm5, xmm4, 0xe4;	// backup xmm4=a-c
    ;				// ------------X(4k+1) start-------------------------
    paddsw xmm4, xmm3;
    lea ebx, wFFTLUT32768_1;
    pshufd xmm0,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm1, xmm0, 0xb1;	// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm1, xmm1, 0xb1;	// xmm1 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm1, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm0, xmm4;		// xmm0 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm1, xmm4;		// xmm1 = Q3 Q2 Q1 Q0
    psrad xmm0, 0x0f;
    psrad xmm1, 0x0f;
    pand xmm0, xmm6;
    pand xmm1, xmm6;
    pslld xmm1, 0x10;
    por xmm0, xmm1;		// xmm0 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+1) end---------------------------
    ;				// xmm1 xmm2 xmm4 are no use now
    ;				// ------------X(4k+3) start-------------------------
    psubsw xmm5, xmm3;
    lea ebx, wFFTLUT32768_3;
    pshufd xmm1,[ebx + ecx], 0xe4;
    ;				// --- Complex Multiplication Start---
    pshuflw xmm2, xmm1, 0xb1;	// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    pshufhw xmm2, xmm2, 0xb1;	// xmm2 = I3 Q3 I2 Q2 I1 Q1 I0 Q0
    pxor xmm2, xmm6;		// xmm0 = Q3 -I3 Q2 -I2 Q1 -I1 Q0 -I0
    pmaddwd xmm1, xmm5;		// xmm1 = I3 I2 I1 I0, where Ii=ac-bd (a+bj)*(c+dj)=(ac-bd)+(ad+bc)j
    pmaddwd xmm2, xmm5;		// xmm2 = Q3 Q2 Q1 Q0
    psrad xmm1, 0x0f;
    psrad xmm2, 0x0f;
    pand xmm1, xmm6;
    pand xmm2, xmm6;
    pslld xmm2, 0x10;
    por xmm1, xmm2;		// xmm1 = Q3 I3 Q2 I2 Q1 I1 Q0 I0
    ;				// --- Complex Multiplication End---
    ;				// ------------X(4k+3) end---------------------------
    movdqa[esi + 65536], xmm0;
    movdqa[esi + 98304], xmm1;
    add ecx, 16;
    add esi, 16;
    cmp ecx, 32768;
    jne L_32768_10;
  }				// end of asm
}				// end of IFFTSSE32768
