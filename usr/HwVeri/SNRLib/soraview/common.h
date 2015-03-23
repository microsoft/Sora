#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <cstdio>
#include <list>
#include <vector>
#include <string>
#include <cmath>
#include "simplewnd.h"

// #include "color.h"

void startDemoWindow(); // show main window


// info window
extern const char * INFO_WND_NAME;
extern const int INFO_WND_HEIGHT;
extern const int INFO_WND_WIDTH;
extern HWND gHwnd;
extern HWND g_FreqHWnd;
LRESULT CALLBACK FreqWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);




// main procedure
LRESULT CALLBACK mainProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// main window properties
void setMainInstance(HINSTANCE hi);
HINSTANCE getMainInstance();
const char * getClassName();
const char * getWinTitle();
int getWinHeight();
int getWinWidth();

// painter registration and control
typedef void (*PAINT_FUNC)(HDC hdc);
int addPaintFunc(PAINT_FUNC f);
void clearPaintFuncs();
void paintInit(HWND hwnd);
void paintAll(HWND hwnd);
void paintClean(HWND hwnd);
void pntClear(HDC hdc);

int getCanvasWidth();
int getCanvasHeight();

// printf for debug
void dprintf(const char * format, ...);
void fdprintf ( int id, const char * format, ...);

void pntDebugLog(HDC hdc);

// paint for text
void pntText(HDC hdc, int x, int y, const char * text, int len);
void pntText(HDC hdc, int x, int y, const char * text);

// sample painter
void pntSample(HDC hdc);

// dump file reader
int loadDumpFile(char * fn);
Complex * getDumpData(int index);
int getDumpTotalCount();
void pntOverview(HDC hdc);
int getPointerTime(int x, int y);
void setOverviewRange(int start, int end = -1);
void overviewPageUp();
void overviewPageDown();
void overviewCenter();
void overviewZoomIn();
void overviewZoomOut();

// push
void pushData(int count);
void resetData(int point = 0);
int getSampleCounter();
int getDescCounter();
void rawDataPaintInit();

void pntRawData(HDC hdc);

// synchronizer
void pushSynchronizer(Complex & c);
void resetSynchronizer();
void pntCarrierSensor(HDC hdc);

void pntCorrelator(HDC hdc);

void synchronizerInit();

void clearReceiverState();

// border support
void pntBorder(HDC hdc, int x, int y, int h, int w, const char * title);
int getBorderMargin();

// wave data
#define DWAVE_BUFSIZE 800 //1024
struct DWave
{
    double data[DWAVE_BUFSIZE];
    bool mark[DWAVE_BUFSIZE];
    int start;
    int count;
    int hisLen;

    double up;
    double down;
    double normalHeight_;

    COLORREF color;
};

void pntWave(HDC hdc, DWave & wave, int x, int y, int h, int w);
void pntWaveB(HDC hdc, DWave & wave, int x, int y, int h, int w, const char * title);
void waveReset(DWave & w, int hisLen);
void waveSetBound(DWave & w, double b1, double b2);
void wavePush(DWave & w, double data, bool mark = false);
void waveSetColor(DWave & w, COLORREF c);
void waveMark(DWave & w, bool mark = true);
void waveMarkHis(DWave & w, int his, bool mark = true);
void waveMarkClear(DWave & w);

// constel data
#define DCONSTEL_HISLEN 640
struct DConstel
{
    Complex data[DCONSTEL_HISLEN];
    int current;
    int count;

    double normalOne;
    double normalOne_1;
    COLORREF lightColor;
    COLORREF darkColor;
};

double constelGetNormalOne(DConstel & c);
void constelSetNormalOne(DConstel & c, double newNormalOne);
void pntDrawCross(HDC hdc, int x, int y, COLORREF c);
void pntConstel(HDC hdc, DConstel & c, int x, int y, int size);
void pntConstelB(HDC hdc, DConstel & c, int x, int y, int size, const char * title);
void constelClear(DConstel & constel);
void constelPush(DConstel & constel, Complex c);

// common painting tools
HPEN getGreyPen();

// keyboard control
void keyControl(WPARAM key);

// play control
bool isPlaying();
void playOrPause();
void play();
void pause();
void speedUp();
void speedDown();
int getSpeed();

// channel estimation and equalizer
void setEqualizerIndicator(int i);
bool pushEqualizer(Complex & c);
void freqComp(Complex * pc, int count);
void channelComp(Complex * pc);
double getSoftBitModifier ( int ch );

// fft
void fft(Complex * input, Complex * output);

// symbol level
void pushSymbol(const Complex * pc);
void resetSymbol();
void clearSymbol();
void pntRawSpec(HDC hdc);
void pntChannelSpec(HDC hdc);
void pntModelSpec(HDC hdc);
void setChannel(const Complex * pc);

// pilot correction
void pushPilot(const Complex * pc);
void resetPilot();
void clearPilot();
void pntBeforePilot(HDC hdc);
void pntAfterPilot(HDC hdc);
void resetPilotCounter();

// decoder
void pushDecode(const Complex * pc);
void clearDecode();
void pntDecode(HDC hdc);
void resetDecode();
int getDataRate();

// viterbi
void pushViterbi(unsigned char * input, int rate);
void pushEndViterbi(int rate);
void clearViterbi();
void resetViterbi();

// byte (descramble and CRC32)
void pushRawByte(unsigned char c);
void clearByte();
void setByteTotal(int length);
void resetByte();

// color scheme
enum ColorNames
{
    COLOR_BGROUND = 0,
    COLOR_BORDER,
    COLOR_TEXT,
    COLOR_RAW,
    COLOR_RAWBG,
    COLOR_MODEL,
    COLOR_ENERGY,
    COLOR_CORR,
    COLOR_NORMAL,
    COLOR_HL,

    COLOR_BIT_CONF,
    COLOR_BIT_OKAY,
    COLOR_BIT_SUSP,
    COLOR_BIT_AMBI,

    COLOR_CRC_NORMAL,
    COLOR_CRC_CORRECT,
    COLOR_CRC_INCORRECT,

    COLOR_REF,

    COLOR_COUNT,
};

void colorInit();
COLORREF CS(int id);
void setScheme(int id);
void nextScheme();
COLORREF getColor(COLORREF c1, COLORREF c2, int bright);

#endif//COMMON_H

