#include "common.h"

static const int CHASPEC_X = 10;
static const int CHASPEC_Y = 210;
static const int CHASPEC_H = 66;
static const int CHASPEC_W = 200;

static const int RAWSPEC_X = 10;
static const int RAWSPEC_Y = 10+200+66;
static const int RAWSPEC_H = 66;
static const int RAWSPEC_W = 200;

static const int COMPSPEC_X = 10;
static const int COMPSPEC_Y = 10+200+2*66;
static const int COMPSPEC_H = 68;
static const int COMPSPEC_W = 200;

static bool symbolValid = false;

Complex symbol[80];
Complex symbolFreq[64];

static Complex symbolComp[64];
Complex symbolChannel[64];

// kun
double SoftBitModifier[64];

void compSoftBitModifier () {
	double max_mod=0;
	for ( int i=0; i<64; i++) {
		cmod ( SoftBitModifier[i], symbolChannel[i]);
		if ( SoftBitModifier[i] > max_mod )
			max_mod = SoftBitModifier[i];
	}
	for ( int i=0; i<64; i++) {
		SoftBitModifier[i] = SoftBitModifier[i] / max_mod;
	}
}

double getSoftBitModifier ( int ch ) {
	return SoftBitModifier[ch];
}

void
setChannel(const Complex * pc)
{
    memcpy(symbolChannel, pc, sizeof(Complex) * 64);
	compSoftBitModifier ();
}

void
pushSymbol(const Complex * pc)
{
    symbolValid = true;
    memcpy(symbol, pc, sizeof(Complex) * 80);
    
    freqComp(symbol, 80);
    fft(&symbol[8], symbolFreq);

    memcpy(symbolComp, symbolFreq, sizeof(Complex) * 64);
    channelComp(symbolComp);

    pushPilot(symbolComp);
}

void
resetSymbol()
{
    clearSymbol();
    resetPilot();
}

void
clearSymbol()
{
    symbolValid = false;
    clearPilot();
}

static void pntSymbolFreq(HDC hdc, Complex * pc, int x, int y, int h, 
        COLORREF c, double minH, bool floating = true);

static const int SYMBOL_FREQ_WIDTH = 3;

void
pntRawSpec(HDC hdc)
{
    int x = RAWSPEC_X;
    int y = RAWSPEC_Y;
    int h = RAWSPEC_H;
    int w = RAWSPEC_W;
    int m = getBorderMargin();
    int w_ = 64 * SYMBOL_FREQ_WIDTH;

//    if (symbolValid)
        pntSymbolFreq(hdc, symbolFreq, x + (w - w_) / 2, y + m + 15, h - 2 * m - 15, 
                CS(COLOR_ENERGY), 100);
    pntBorder(hdc, x, y, h, w, 
            "Before channel compensation");
}

void
pntModelSpec(HDC hdc)
{
    int x = CHASPEC_X;
    int y = CHASPEC_Y;
    int h = CHASPEC_H;
    int w = CHASPEC_W;
    int m = getBorderMargin();
    int w_ = 64 * SYMBOL_FREQ_WIDTH;

    if (symbolValid)
    {
        pntSymbolFreq(hdc, symbolChannel, 
                x + (w - w_) / 2, y + m + 15, h - 2 * m - 15, 
                CS(COLOR_MODEL), 100);
    }
    pntBorder(hdc, x, y, h, w, 
            "Channel model");
}

void
pntChannelSpec(HDC hdc)
{
    int x = COMPSPEC_X;
    int y = COMPSPEC_Y;
    int h = COMPSPEC_H;
    int w = COMPSPEC_W;
    int m = getBorderMargin();
    int w_ = 64 * SYMBOL_FREQ_WIDTH;

    if (symbolValid)
    {
        pntSymbolFreq(hdc, symbolComp, x + (w - w_) / 2, y + m + 15, h - 2 * m - 15, 
                CS(COLOR_NORMAL), 2.0, false);
    }
    pntBorder(hdc, x, y, h, w, 
            "After channel compensation");
}

void
pntSymbolFreq(HDC hdc, Complex * pc, int x, int y, int h, 
        COLORREF c, double minH, bool floating)
{
    double data[64];

    for (int i = 0; i < 64; i++)
    {
        data[i] = sqrt(pc[i].re * pc[i].re + pc[i].im * pc[i].im);
        if (floating)
            while (data[i] > minH)
                minH *= 1.5;
    }

    double minH_1 = 1 / minH;
    
    HPEN hpen = CreatePen(PS_SOLID, 1, c);
    SelectObject(hdc, hpen);

    for (int i = 0; i < 64; i++)
    {
        int px = x + i * SYMBOL_FREQ_WIDTH;
        double ph = h * (1 - data[(i + 32) % 64] * minH_1);
        if (ph < 0) ph = 0;
        int py = y + int(ph);

        if (i == 0)
            MoveToEx(hdc, px, py, NULL);
        else
            LineTo(hdc, px, py);
    }

    DeleteObject(hpen);
}
