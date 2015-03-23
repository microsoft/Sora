void
IFFT4 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE4Ex (pInput, pOutput);
for ( i = 0; i < 4; i++)
{
pOutput[i] = pInput[ FFTLUTMap(4)[i] ];
}
}
void
IFFT8 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE8Ex (pInput, pOutput);
for ( i = 0; i < 8; i++)
{
pOutput[i] = pInput[ FFTLUTMap(8)[i] ];
}
}
void
IFFT16 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE16Ex (pInput, pOutput);
for ( i = 0; i < 16; i++)
{
pOutput[i] = pInput[ FFTLUTMap(16)[i] ];
}
}
void
IFFT32 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE32Ex (pInput, pOutput);
for ( i = 0; i < 32; i++)
{
pOutput[i] = pInput[ FFTLUTMap(32)[i] ];
}
}
void
IFFT64 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE64Ex (pInput, pOutput);
for ( i = 0; i < 64; i++)
{
pOutput[i] = pInput[ FFTLUTMap(64)[i] ];
}
}
void
IFFT128 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE128Ex (pInput, pOutput);
for ( i = 0; i < 128; i++)
{
pOutput[i] = pInput[ FFTLUTMap(128)[i] ];
}
}
void
IFFT256 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE256Ex (pInput, pOutput);
for ( i = 0; i < 256; i++)
{
pOutput[i] = pInput[ FFTLUTMap(256)[i] ];
}
}
void
IFFT512 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE512Ex (pInput, pOutput);
for ( i = 0; i < 512; i++)
{
pOutput[i] = pInput[ FFTLUTMap(512)[i] ];
}
}
void
IFFT1024 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE1024Ex (pInput, pOutput);
for ( i = 0; i < 1024; i++)
{
pOutput[i] = pInput[ FFTLUTMap(1024)[i] ];
}
}
void
IFFT2048 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE2048Ex (pInput, pOutput);
for ( i = 0; i < 2048; i++)
{
pOutput[i] = pInput[ FFTLUTMap(2048)[i] ];
}
}
void
IFFT4096 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE4096Ex (pInput, pOutput);
for ( i = 0; i < 4096; i++)
{
pOutput[i] = pInput[ FFTLUTMap(4096)[i] ];
}
}
void
IFFT8192 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE8192Ex (pInput, pOutput);
for ( i = 0; i < 8192; i++)
{
pOutput[i] = pInput[ FFTLUTMap(8192)[i] ];
}
}
void
IFFT16384 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE16384Ex (pInput, pOutput);
for ( i = 0; i < 16384; i++)
{
pOutput[i] = pInput[ FFTLUTMap(16384)[i] ];
}
}
void
IFFT32768 (COMPLEX16* pInput, COMPLEX16* pOutput)
{
int i;
$_IFFTSSE_$();
IFFTSSE32768Ex (pInput, pOutput);
for ( i = 0; i < 32768; i++)
{
pOutput[i] = pInput[ FFTLUTMap(32768)[i] ];
}
}