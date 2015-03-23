#include "FFTAllIn1.h"

#include "FFTLutMap32k.h"
#include "FFTwLUT32K.h"
//
#include "FFTInlineBase.h"
#include "FFTInline32K.h"
#include "FFTInlineEx32K.h"
//
#include "IFFTInlineBase.h"
#include "IFFTInline32K.h"
#include "IFFTInlineEx32K.h"
//
#include "FFTImp32K.h"
#include "IFFTImp32K.h"

//
#ifdef _DEBUG

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#pragma warning(disable : 4996) // disable warnings of fopen

#define N 1024
__declspec(align(16)) COMPLEX16 Input[N];
__declspec(align(16)) COMPLEX16 Output[N];

int main(int argc, char* argv[])
{
    LARGE_INTEGER Start, End, Freq;
    FILE* hInput = NULL;

    int i = 0;
    int iCount = 20000;

    QueryPerformanceFrequency(&Freq);

#pragma region "Src2File"
    hInput = fopen("FFTInput.txt", "w");

    for (i = 0; i < N; i++)
    {
        Input[i].re = (rand() % 2 == 0) ? 1024 : -1024;
        Input[i].im = (rand() % 2 == 0) ? 1024 : -1024;
        fprintf(hInput, "%d\t%d\n", Input[i].re, Input[i].im);
    }

    fclose(hInput);
#pragma endregion

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    SetProcessAffinityMask(GetCurrentProcess(), 1<<1);

    while (iCount--)
    {
        QueryPerformanceCounter(&Start);

        FFT1024(Input, Output);

        QueryPerformanceCounter(&End);

        printf("%f\n", (float)(End.QuadPart - Start.QuadPart) * 1000000 / Freq.QuadPart);
    }

#pragma region "Results2File"
    hInput = fopen("FFTOutput.txt", "w");

    for (i = 0; i < N; i++)
    {
        fprintf(hInput, "%d\t%d\n", Output[i].re, Output[i].im);
    }

    fclose(hInput);
#pragma endregion

    return 0;
}

#endif