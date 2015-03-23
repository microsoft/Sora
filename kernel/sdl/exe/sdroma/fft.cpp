#include "common.h"

static const int THE_N = 64;
static const double PI = 3.14159265359;
static double c[THE_N];
static double s[THE_N];
static bool first = true;

static const int M1 = 0x21, M2 = 0x12, M3 = 0x0C;

void fft(Complex * input, Complex * output)
{
    for (int i=0; i<THE_N; i++)
    {
        int j = i;
        if (((i & M1) ^ M1) && (i & M1))
            j ^= M1;
        if (((i & M2) ^ M2) && (i & M2))
            j ^= M2;
        if (((i & M3) ^ M3) && (i & M3))
            j ^= M3;
        output[j] = input[i];
    }

    if (first)
    {
        for (int i=0; i<THE_N; i++)
        {
            c[i] = cos(2 * PI * i / THE_N);
            s[i] = -sin(2 * PI * i / THE_N);
        }
        first = false;
    }
    
    int deltatheta = (THE_N >> 1);
    
    for (int i=1; i<THE_N; i<<=1)
    {
        int t = i << 1;
        for (int j=0; j<THE_N; j+=t)
        {
            int theta = 0;
            for (int k=0; k<i; k++)
            {
                int e1 = j + k;
                int e2 = j + k + i;
                
                double tempre = output[e2].re * c[theta] - output[e2].im * s[theta];
                double tempim = output[e2].re * s[theta] + output[e2].im * c[theta];
                output[e2].re = output[e1].re - tempre;
                output[e2].im = output[e1].im - tempim;
                output[e1].re += tempre;
                output[e1].im += tempim;
                
                theta += deltatheta;
            }
        }
        deltatheta >>= 1;
    }
}
