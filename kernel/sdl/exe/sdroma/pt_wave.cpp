#include "common.h"

void
pntWave(HDC hdc, DWave & wave, int x, int y, int h, int w)
{
    
    double step = 1.0 * w / wave.hisLen;
    // int _w = step * wave.hisLen;
    // int leftm = (w - _w) / 2;
    // int _x = x + leftm;
    
    int index = wave.start;

    HPEN hpen = CreatePen(PS_SOLID, 1, wave.color);
    SelectObject(hdc, hpen);
    for (int i = 0; i < wave.count; i++)
    {
        double d = wave.data[index];
        if (d > wave.up) d = 1;
        else if (d < wave.down) d = 0;
        else 
            d = (d - wave.down) * wave.normalHeight_;

        int py = y + int(h * (1 - d));
       // int px = _x + _w - i * step;
        int px = (int)(x + w - i * step);
        if (i == 0)
            MoveToEx(hdc, px, py, NULL);
        else
            LineTo(hdc, px, py);

        index++;
        index %= DWAVE_BUFSIZE;
    }
    DeleteObject(hpen);

    index = wave.start;
    SelectObject(hdc, getGreyPen());
    for (int i = 0; i < wave.count; i++)
    {
        if (wave.mark[index])
        {
            // int px = _x + _w - i * step;
            int px = (int)(x + w - i * step);

            MoveToEx(hdc, px, y, NULL);
            LineTo(hdc, px, y + h);
        }

        index = (index + 1) % DWAVE_BUFSIZE;
    }
}

void
pntWaveB(HDC hdc, DWave & wave, int x, int y, int h, int w, const char * title)
{
    int m = getBorderMargin();
    pntWave(hdc, wave, x + m, y + m, h - 2 * m, w - 2 * m);
    pntBorder(hdc, x, y, h, w, title);
}

void
waveReset(DWave & w, int hisLen)
{
    w.start = 0;
    w.count = 0;
    w.hisLen = hisLen;
}

void
waveSetBound(DWave & w, double up, double down)
{
    if (up < down)
    {
        waveSetBound(w, down, up);
        return;
    }

    w.up = up;
    w.down = down;
    w.normalHeight_ = 1 / (up - down);
}

void
wavePush(DWave & w, double d, bool mark)
{
    w.start--;
    if (w.start < 0)
        w.start += DWAVE_BUFSIZE;
    if (w.count < w.hisLen)
        w.count++;

    w.data[w.start] = d;
    w.mark[w.start] = mark;
}

void
waveMark(DWave & w, bool mark)
{
    w.mark[w.start] = mark;
}

void
waveMarkHis(DWave & w, int his, bool mark)
{
    int index = (w.start + his) % DWAVE_BUFSIZE;
    w.mark[index] = true;
}

void
waveMarkClear(DWave & w)
{
    for (int i = 0; i < DWAVE_BUFSIZE; i++)
        w.mark[i] = false;
}

void
waveSetColor(DWave & w, COLORREF c)
{
    w.color = c;
}
