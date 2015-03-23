#include "common.h"

static const int BORDER_MARGIN = 2;

void pntBorder(HDC hdc, int x, int y, int h, int w, const char * title)
{
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, CS(COLOR_BORDER));
    SelectObject(hdc, hPenBorder);
    MoveToEx(hdc, x, y, NULL); 
    LineTo(hdc, x, y + h);
    LineTo(hdc, x + w, y + h);
    LineTo(hdc, x + w, y);
    LineTo(hdc, x, y);

    pntText(hdc, x + 2, y + 2, title);
    DeleteObject(hPenBorder);
}

int getBorderMargin()
{
    return BORDER_MARGIN;
}

