void
IFFTSSE32Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE32(pInput);
IFFTSSE8Ex(pInput + 0, pOutput + 0);
IFFTSSE8Ex(pInput + 8, pOutput + 8);
IFFTSSE8Ex(pInput + 16, pOutput + 16);
IFFTSSE8Ex(pInput + 24, pOutput + 24);
}
void
IFFTSSE64Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE64(pInput);
IFFTSSE16Ex(pInput + 0, pOutput + 0);
IFFTSSE16Ex(pInput + 16, pOutput + 16);
IFFTSSE16Ex(pInput + 32, pOutput + 32);
IFFTSSE16Ex(pInput + 48, pOutput + 48);
}
void
IFFTSSE128Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE128(pInput);
IFFTSSE32Ex(pInput + 0, pOutput + 0);
IFFTSSE32Ex(pInput + 32, pOutput + 32);
IFFTSSE32Ex(pInput + 64, pOutput + 64);
IFFTSSE32Ex(pInput + 96, pOutput + 96);
}
void
IFFTSSE256Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE256(pInput);
IFFTSSE64Ex(pInput + 0, pOutput + 0);
IFFTSSE64Ex(pInput + 64, pOutput + 64);
IFFTSSE64Ex(pInput + 128, pOutput + 128);
IFFTSSE64Ex(pInput + 192, pOutput + 192);
}
void
IFFTSSE512Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE512(pInput);
IFFTSSE128Ex(pInput + 0, pOutput + 0);
IFFTSSE128Ex(pInput + 128, pOutput + 128);
IFFTSSE128Ex(pInput + 256, pOutput + 256);
IFFTSSE128Ex(pInput + 384, pOutput + 384);
}
void
IFFTSSE1024Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE1024(pInput);
IFFTSSE256Ex(pInput + 0, pOutput + 0);
IFFTSSE256Ex(pInput + 256, pOutput + 256);
IFFTSSE256Ex(pInput + 512, pOutput + 512);
IFFTSSE256Ex(pInput + 768, pOutput + 768);
}
void
IFFTSSE2048Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE2048(pInput);
IFFTSSE512Ex(pInput + 0, pOutput + 0);
IFFTSSE512Ex(pInput + 512, pOutput + 512);
IFFTSSE512Ex(pInput + 1024, pOutput + 1024);
IFFTSSE512Ex(pInput + 1536, pOutput + 1536);
}
void
IFFTSSE4096Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE4096(pInput);
IFFTSSE1024Ex(pInput + 0, pOutput + 0);
IFFTSSE1024Ex(pInput + 1024, pOutput + 1024);
IFFTSSE1024Ex(pInput + 2048, pOutput + 2048);
IFFTSSE1024Ex(pInput + 3072, pOutput + 3072);
}
void
IFFTSSE8192Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE8192(pInput);
IFFTSSE2048Ex(pInput + 0, pOutput + 0);
IFFTSSE2048Ex(pInput + 2048, pOutput + 2048);
IFFTSSE2048Ex(pInput + 4096, pOutput + 4096);
IFFTSSE2048Ex(pInput + 6144, pOutput + 6144);
}
void
IFFTSSE16384Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE16384(pInput);
IFFTSSE4096Ex(pInput + 0, pOutput + 0);
IFFTSSE4096Ex(pInput + 4096, pOutput + 4096);
IFFTSSE4096Ex(pInput + 8192, pOutput + 8192);
IFFTSSE4096Ex(pInput + 12288, pOutput + 12288);
}
void
IFFTSSE32768Ex (COMPLEX16* pInput, COMPLEX16* pOutput)
{
IFFTSSE32768(pInput);
IFFTSSE8192Ex(pInput + 0, pOutput + 0);
IFFTSSE8192Ex(pInput + 8192, pOutput + 8192);
IFFTSSE8192Ex(pInput + 16384, pOutput + 16384);
IFFTSSE8192Ex(pInput + 24576, pOutput + 24576);
}