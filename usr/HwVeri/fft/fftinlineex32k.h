void
FFTSSE32Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE32 (pInput);
  FFTSSE8Ex (pInput + 0, pOutput + 0);
  FFTSSE8Ex (pInput + 8, pOutput + 8);
  FFTSSE8Ex (pInput + 16, pOutput + 16);
  FFTSSE8Ex (pInput + 24, pOutput + 24);
}

void
FFTSSE64Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE64 (pInput);
  FFTSSE16Ex (pInput + 0, pOutput + 0);
  FFTSSE16Ex (pInput + 16, pOutput + 16);
  FFTSSE16Ex (pInput + 32, pOutput + 32);
  FFTSSE16Ex (pInput + 48, pOutput + 48);
}

void
FFTSSE128Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE128 (pInput);
  FFTSSE32Ex (pInput + 0, pOutput + 0);
  FFTSSE32Ex (pInput + 32, pOutput + 32);
  FFTSSE32Ex (pInput + 64, pOutput + 64);
  FFTSSE32Ex (pInput + 96, pOutput + 96);
}

void
FFTSSE256Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE256 (pInput);
  FFTSSE64Ex (pInput + 0, pOutput + 0);
  FFTSSE64Ex (pInput + 64, pOutput + 64);
  FFTSSE64Ex (pInput + 128, pOutput + 128);
  FFTSSE64Ex (pInput + 192, pOutput + 192);
}

void
FFTSSE512Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE512 (pInput);
  FFTSSE128Ex (pInput + 0, pOutput + 0);
  FFTSSE128Ex (pInput + 128, pOutput + 128);
  FFTSSE128Ex (pInput + 256, pOutput + 256);
  FFTSSE128Ex (pInput + 384, pOutput + 384);
}

void
FFTSSE1024Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE1024 (pInput);
  FFTSSE256Ex (pInput + 0, pOutput + 0);
  FFTSSE256Ex (pInput + 256, pOutput + 256);
  FFTSSE256Ex (pInput + 512, pOutput + 512);
  FFTSSE256Ex (pInput + 768, pOutput + 768);
}

void
FFTSSE2048Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE2048 (pInput);
  FFTSSE512Ex (pInput + 0, pOutput + 0);
  FFTSSE512Ex (pInput + 512, pOutput + 512);
  FFTSSE512Ex (pInput + 1024, pOutput + 1024);
  FFTSSE512Ex (pInput + 1536, pOutput + 1536);
}

void
FFTSSE4096Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE4096 (pInput);
  FFTSSE1024Ex (pInput + 0, pOutput + 0);
  FFTSSE1024Ex (pInput + 1024, pOutput + 1024);
  FFTSSE1024Ex (pInput + 2048, pOutput + 2048);
  FFTSSE1024Ex (pInput + 3072, pOutput + 3072);
}

void
FFTSSE8192Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE8192 (pInput);
  FFTSSE2048Ex (pInput + 0, pOutput + 0);
  FFTSSE2048Ex (pInput + 2048, pOutput + 2048);
  FFTSSE2048Ex (pInput + 4096, pOutput + 4096);
  FFTSSE2048Ex (pInput + 6144, pOutput + 6144);
}

void
FFTSSE16384Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE16384 (pInput);
  FFTSSE4096Ex (pInput + 0, pOutput + 0);
  FFTSSE4096Ex (pInput + 4096, pOutput + 4096);
  FFTSSE4096Ex (pInput + 8192, pOutput + 8192);
  FFTSSE4096Ex (pInput + 12288, pOutput + 12288);
}

void
FFTSSE32768Ex (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  FFTSSE32768 (pInput);
  FFTSSE8192Ex (pInput + 0, pOutput + 0);
  FFTSSE8192Ex (pInput + 8192, pOutput + 8192);
  FFTSSE8192Ex (pInput + 16384, pOutput + 16384);
  FFTSSE8192Ex (pInput + 24576, pOutput + 24576);
}
