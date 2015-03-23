#include "common.h"

static int indicator = -1;
static const int COMPLEX_LP = 160;
static Complex longPreamble[COMPLEX_LP];
int index = 0;

Complex channel[64] = { 0 };

double freq = 0;
extern int g_samplingrate;
extern int g_decimation;
extern int g_resample44_40;

static void
freqEstimation()
{
    Complex t = { 0 }, sum = { 0 };
    
    
    for (int i = 0; i < 64; i++)
    {
        int ii = (index + i + 16 + 8) % COMPLEX_LP;
        int jj = (index + i + 80 + 8) % COMPLEX_LP;
        cmulf(t, longPreamble[ii], longPreamble[jj]);
        cadd(sum, t);
    }
    
    dprintf ( "Vector %lf %lf\n", sum.re, sum.im ); 
    
    freq = atan(sum.im / sum.re) / 64;

	int fs = g_samplingrate;

	if ( g_resample44_40 )
		fs = fs * 40 / 44;

	fs /= g_decimation;
	printf ( "Freq offset: arg %.6lf (%.6lfKHz) fs (%d MHz)\n", freq, freq * fs * 1000 / 2 / 3.14159265359, fs);
}

void
freqComp(Complex * pc, int count)
{
    Complex r = { 0 };
    
    for (int i = 0; i < count - 16; i++)
    {
        r.re = cos(freq * i);
        r.im = sin(freq * i);
        cmul(pc[i+8], r);    
    }
}

void
freqComp1(Complex * pc, int count)
{
    Complex r = { 0 };
    // Kun:
    int ii = (index + 16 + 8) % count;
    
    for (int i = 0; i < (count - 24); i++)
    {
        r.re = cos(freq * i);
        r.im = sin(freq * i);
        cmul(pc[ii], r);    
        ii = (ii + 1) % count;
    }
}

static char longPrePositive[64] = {
    0,
    1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 
    0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 37
    1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 
    1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1
};

static void 
channelEstimation()
{
    freqEstimation();

	/*
	printf ( "RAW LT:\n" );
	for (int i=0; i<160-16; i++) {
		Complex c = longPreamble[i];
		printf ( "%lf %lf \n", c.re, c.im  );

	}
	*/
    for (int i = 0; i < 64; i++) {
        Complex c = longPreamble[(index + 16 + 8 + i ) % COMPLEX_LP];

    
    }


    
    freqComp1(longPreamble, COMPLEX_LP);

    Complex r1[64];
    Complex r2[64];

dprintf ( "LT:\n" );

    Complex temp[64];
    for (int i = 0; i < 64; i++) {
//        temp[i] = longPreamble[(index + 16 + (i + 8) % 64) % COMPLEX_LP];
        temp[i] = longPreamble[(index + 16 + 8 + i ) % COMPLEX_LP];

dprintf ( "%lf %lf \n", temp[i].re / 2, temp[i].im /2  );
    }
    memset (r1, 0, sizeof(r1));


    fft(temp, r1);
dprintf ( "LT FFT:\n" );
    for (int i = 0; i < 64; i++) {
        dprintf ( "%lf %lf \n", r1[i].re / 2 / 64, r1[i].im / 2 / 64  );
    }

    setChannel(r1);
    const Complex one = {1.0, 0};
    const Complex mone = {-1.0, 0};
    for (int i = 64 - 26; i < 64; i++)
    {
        if (longPrePositive[i])
            cdiv(channel[i], one, r1[i]);
        else
            cdiv(channel[i], mone, r1[i]);
    }

    for (int i = 1; i <= 26; i++)
    {
        if (longPrePositive[i])
            cdiv(channel[i], one, r1[i]);
        else
            cdiv(channel[i], mone, r1[i]);
    }

    dprintf ("Channel log (%d):\n", getSampleCounter());
    for (int i = 0; i < 64; i++) {
        double sq = r1[i].re * r1[i].re + r1[i].im * r1[i].im;
        dprintf ( "%2d %lf %lf\n", i, channel[i].re*100*512*64, channel[i].im*100*512*64 );
    }
    dprintf ( "\n" );
    
}

void
channelComp(Complex * pc)
{
    for (int i = 0; i < 64; i++)
    {
        cmul(pc[i], channel[i]);
        // dprintf("%d: %f %f", i, pc[i].re, pc[i].im);
    }
}

void 
setEqualizerIndicator(int i)
{
    indicator = i;
}

static int savIndex = -1;
static Complex LTBuf[COMPLEX_LP];
bool
pushEqualizer(Complex & c)
{
    longPreamble[index] = c;
//    LTBuf[index] = c;
    index = (index + 1) % COMPLEX_LP;

    if (indicator >= 0)
    {

        indicator++;
        
        
        if (indicator == COMPLEX_LP)
        {
            channelEstimation();
            return true;
        }
    } // else savIndex = -1;

    return false;
}
