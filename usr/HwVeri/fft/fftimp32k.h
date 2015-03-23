void
FFT4 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE4Ex (pInput, pOutput);
  for (i = 0; i < 4; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (4)[i]];
    }
}
void
FFT8 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE8Ex (pInput, pOutput);
  for (i = 0; i < 8; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (8)[i]];
    }
}
void
FFT16 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE16Ex (pInput, pOutput);
  for (i = 0; i < 16; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (16)[i]];
    }
}
void
FFT32 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE32Ex (pInput, pOutput);
  for (i = 0; i < 32; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (32)[i]];
    }
}
void
FFT64 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE64Ex (pInput, pOutput);
  for (i = 0; i < 64; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (64)[i]];
    }
}
void
FFT128 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE128Ex (pInput, pOutput);
  for (i = 0; i < 128; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (128)[i]];
    }
}
void
FFT256 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE256Ex (pInput, pOutput);
  for (i = 0; i < 256; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (256)[i]];
    }
}
void
FFT512 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE512Ex (pInput, pOutput);
  for (i = 0; i < 512; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (512)[i]];
    }
}
void
FFT1024 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE1024Ex (pInput, pOutput);
  for (i = 0; i < 1024; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (1024)[i]];
    }
}
void
FFT2048 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE2048Ex (pInput, pOutput);
  for (i = 0; i < 2048; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (2048)[i]];
    }
}
void
FFT4096 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE4096Ex (pInput, pOutput);
  for (i = 0; i < 4096; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (4096)[i]];
    }
}
void
FFT8192 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE8192Ex (pInput, pOutput);
  for (i = 0; i < 8192; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (8192)[i]];
    }
}
void
FFT16384 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE16384Ex (pInput, pOutput);
  for (i = 0; i < 16384; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (16384)[i]];
    }
}
void
FFT32768 (COMPLEX16 * pInput, COMPLEX16 * pOutput)
{
  int i;
  $_FFTSSE_$ ();
  FFTSSE32768Ex (pInput, pOutput);
  for (i = 0; i < 32768; i++)
    {
      pOutput[i] = pInput[FFTLUTMap (32768)[i]];
    }
}
