#include "common.h"

static PAINT_FUNC funcs[1024];
static int funcCount = 0;

int addPaintFunc(PAINT_FUNC f)
{
    funcs[funcCount] = f;
    funcCount++;

    return funcCount;
}

void clearPaintFuncs()
{
    funcCount = 0;
}

static unsigned int width;
static unsigned int height;
static HDC bufferDC;
static HBITMAP memBM;

void pntClear(HDC hdc)
{
    HPEN bgPen = CreatePen(PS_NULL, 0, CS(COLOR_BGROUND));
    HBRUSH bgBrush = CreateSolidBrush(CS(COLOR_BGROUND));
    SelectObject(hdc, bgPen);
    SelectObject(hdc, bgBrush);
    Rectangle(hdc, 0, 0, getCanvasWidth(), getCanvasHeight());
    DeleteObject(bgPen);
    DeleteObject(bgBrush);
    SetBkColor(hdc, CS(COLOR_BGROUND));
    SetTextColor(hdc, CS(COLOR_TEXT));
}

int getCanvasWidth()
{
    return (int)(width);
}

int getCanvasHeight()
{
    return (int)(height);
}

void paintInit(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
	width  = rc.right - rc.left;
	height = rc.bottom - rc.top;
    
    HDC hdc = GetDC(hwnd);

	bufferDC = CreateCompatibleDC(hdc);
    memBM    = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(bufferDC, memBM);

    rawDataPaintInit();
    synchronizerInit();
}

void paintClean(HWND hwnd)
{
    DeleteObject(memBM);
	DeleteDC(bufferDC);
}

void paintAll(HWND hwnd)
{
	PAINTSTRUCT ps;

    BeginPaint(hwnd, &ps);

    HDC hdc = GetDC(hwnd);

    for (int i = 0; i < funcCount; i++)
        (*(funcs[i]))(bufferDC);

    BitBlt(hdc, 0, 0, width, height, bufferDC, 0, 0, SRCCOPY);
	ReleaseDC(hwnd, hdc);
    
    EndPaint(hwnd, &ps);
}
