#include "common.h"

static HPEN hPenGrey = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
HPEN getGreyPen() { 
    DeleteObject(hPenGrey);
    hPenGrey = CreatePen(PS_SOLID, 1, CS(COLOR_REF));
    return hPenGrey; };
