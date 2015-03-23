#include "common.h"

static const int SCHEME_COUNT = 2;
static COLORREF schemes[2][COLOR_COUNT];
static int scheme = 0;

void 
colorInit()
{
    schemes[0][COLOR_BGROUND]       = RGB(0x00, 0x00, 0x00);
    schemes[0][COLOR_BORDER]        = RGB(0x99, 0x99, 0x99);
    schemes[0][COLOR_TEXT]          = RGB(0xFF, 0xFF, 0xFF);
    schemes[0][COLOR_RAW]           = RGB(0x66, 0xFF, 0x66);
    schemes[0][COLOR_RAWBG]         = RGB(0x00, 0x00, 0x00);
    schemes[0][COLOR_MODEL]         = RGB(0xFF, 0XFF, 0xCC);
    schemes[0][COLOR_ENERGY]        = RGB(0x66, 0xFF, 0x66);
    schemes[0][COLOR_CORR]          = RGB(0xCC, 0xCC, 0xFF);
    schemes[0][COLOR_NORMAL]        = RGB(0xCC, 0x33, 0x33);
    schemes[0][COLOR_HL]            = RGB(0xFF, 0xFF, 0x33);
    schemes[0][COLOR_BIT_CONF]      = RGB(0x66, 0xFF, 0x66);
    schemes[0][COLOR_BIT_OKAY]      = RGB(0xCC, 0xFF, 0x66);
    schemes[0][COLOR_BIT_SUSP]      = RGB(0xFF, 0xFF, 0x66);
    schemes[0][COLOR_BIT_AMBI]      = RGB(0xFF, 0x66, 0x66);
    schemes[0][COLOR_CRC_NORMAL]    = RGB(0xCC, 0xCC, 0xCC);
    schemes[0][COLOR_CRC_CORRECT]   = RGB(0x33, 0xFF, 0x33);
    schemes[0][COLOR_CRC_INCORRECT] = RGB(0xFF, 0x33, 0x33);
    schemes[0][COLOR_REF]           = RGB(0x99, 0x99, 0x99);

    schemes[1][COLOR_BGROUND]       = RGB(0xFF, 0xFF, 0xF0);
    schemes[1][COLOR_BORDER]        = RGB(0x00, 0x00, 0x00);
    schemes[1][COLOR_TEXT]          = RGB(0x00, 0x00, 0x00);
    schemes[1][COLOR_RAW]           = RGB(0x00, 0x33, 0x00);
    schemes[1][COLOR_RAWBG]         = RGB(0xFF, 0xFF, 0xFF);
    schemes[1][COLOR_MODEL]         = RGB(0x99, 0X99, 0x00);
    schemes[1][COLOR_ENERGY]        = RGB(0x00, 0x99, 0x00);
    schemes[1][COLOR_CORR]          = RGB(0x00, 0x00, 0xCC);
    schemes[1][COLOR_NORMAL]        = RGB(0x00, 0x33, 0x33);
    schemes[1][COLOR_HL]            = RGB(0xFF, 0x00, 0x00);
    schemes[1][COLOR_BIT_CONF]      = RGB(0x00, 0x99, 0x00);
    schemes[1][COLOR_BIT_OKAY]      = RGB(0x66, 0x99, 0x00);
    schemes[1][COLOR_BIT_SUSP]      = RGB(0x99, 0x99, 0x00);
    schemes[1][COLOR_BIT_AMBI]      = RGB(0xCC, 0x00, 0x00);
    schemes[1][COLOR_CRC_NORMAL]    = RGB(0x66, 0x66, 0x66);
    schemes[1][COLOR_CRC_CORRECT]   = RGB(0x00, 0x99, 0x00);
    schemes[1][COLOR_CRC_INCORRECT] = RGB(0xFF, 0x00, 0x00);
    schemes[1][COLOR_REF]           = RGB(0x66, 0x66, 0x66);
}

COLORREF CS(int name)
{
    return schemes[scheme][name];
}

void setScheme(int id)
{
    scheme = id;
    if (scheme > SCHEME_COUNT)
        scheme = 0;
}

void nextScheme()
{
    scheme++;
    if (scheme >= SCHEME_COUNT)
        scheme = 0;
}

COLORREF getColor(COLORREF c1, COLORREF c2, int bright)
{
    double r1 = GetRValue(c1);
    double g1 = GetGValue(c1);
    double b1 = GetBValue(c1);

    double r2 = GetRValue(c2);
    double g2 = GetGValue(c2);
    double b2 = GetBValue(c2);

    bright = 255 - bright;

    unsigned char r = (unsigned char)(r1 + (r2 - r1) * bright / 255);
    unsigned char g = (unsigned char)(g1 + (g2 - g1) * bright / 255);
    unsigned char b = (unsigned char)(b1 + (b2 - b1) * bright / 255);
    
    return RGB(r, g, b);
}
