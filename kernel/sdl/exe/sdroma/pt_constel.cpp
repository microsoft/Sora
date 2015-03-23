#include "common.h"

double 
constelGetNormalOne(DConstel & constel)
{
    return constel.normalOne;
}

void 
constelSetNormalOne(DConstel & constel, double newNO)
{
    if (newNO < 0)
        newNO = -newNO;
    constel.normalOne = newNO;
    constel.normalOne_1 = 1 / newNO;
}

void 
pntDrawCross(HDC hdc, int x, int y, COLORREF c)
{
    HPEN hpen = CreatePen(PS_SOLID, 1, c);
    SelectObject(hdc, hpen);

    MoveToEx(hdc, x - 1, y, NULL);
    LineTo(hdc, x + 2, y);
    MoveToEx(hdc, x, y - 1, NULL);
    LineTo(hdc, x, y + 2);

    DeleteObject(hpen);
}

void
pntConstelB(HDC hdc, DConstel & constel, int x, int y, int size, const char * title)
{
    int m = getBorderMargin();
    pntConstel(hdc, constel, x + m, y + m, size - 2 * m);
    pntBorder(hdc, x, y, size, size, title);
}

void 
pntConstel(HDC hdc, DConstel & constel, int x, int y, int size)
{
    int w = size, h = size;
    int centerX = x + w / 2;
    int centerY = y + h / 2;

    int index = constel.current - constel.count + DCONSTEL_HISLEN;
    index %= DCONSTEL_HISLEN;

    // this draw the circles
    SelectObject(hdc, getGreyPen());
    Arc(hdc, x, y, x + w, y + h, 0, 0, 0, 0);
    Arc(hdc, centerX - w / 4, centerY - h / 4,
            centerX + w / 4, centerY + h / 4, 0, 0, 0, 0);
    
    MoveToEx(hdc, x, centerY, NULL); 
    LineTo(hdc, x + w, centerY);
    MoveToEx(hdc, centerX, y, NULL); 
    LineTo(hdc, centerX, y + h);

    for (int i = 0; i < constel.count; i++)
    {
        int b = (i + DCONSTEL_HISLEN - constel.count + 1) * 255 / DCONSTEL_HISLEN;
        // dprintf("%d: %d", i, b);
        double sx = constel.data[index].re * constel.normalOne_1;
        if (sx > 1.0) sx = 1.0; if (sx < -1.0) sx = -1.0;
        double sy = constel.data[index].im * constel.normalOne_1;
        if (sy > 1.0) sy = 1.0; if (sy < -1.0) sy = -1.0;

        int px = centerX + (int)(sx * w / 2);
        int py = centerY + (int)(sy * h / 2);

        pntDrawCross(hdc, px, py, 
                getColor(constel.lightColor, constel.darkColor, b));

        index++;
        index %= DCONSTEL_HISLEN;
    }

    char buf[1000];
    sprintf(buf, "r = %.2f", constel.normalOne);
    pntText(hdc, x + 1, y + h - 15, buf);
}

void 
constelClear(DConstel & constel)
{
    constel.count = 0;
    constel.current = 0;
}

void constelPush(DConstel & constel, Complex c)
{
    constel.data[constel.current] = c;
    constel.current++;
    constel.current %= DCONSTEL_HISLEN;
    if (constel.count < DCONSTEL_HISLEN)
        constel.count++;
}
