#include "common.h"

static const int PX_PERLINE = 13;
static const int PX_PERCOL = 5;
static HFONT hDefFont = CreateFontA(
        PX_PERLINE,                     // height of font
        PX_PERCOL,                      // average character width
        0,                  // angle of escapement
        0,                  // base-line orientation angle
        FW_DONTCARE,        // font weight
        FALSE,              // italic attribute option
        FALSE,              // underline attribute option
        FALSE,              // strikeout attribute option
        ANSI_CHARSET,                   // character set identifier
        OUT_CHARACTER_PRECIS,           // output precision
        CLIP_CHARACTER_PRECIS,          // clipping precision
        DEFAULT_QUALITY,                // output quality
        DEFAULT_PITCH | FF_DONTCARE,    // pitch and family
        NULL
        // "Courier New"                   // typeface name
    );


void
pntText(HDC hdc, int x, int y, const char * text, int len)
{
    HFONT hOldFont = (HFONT) SelectObject(hdc, hDefFont); 
    if (hOldFont) 
    { 
        TextOutA(hdc, x, y, text, len);
        SelectObject(hdc, hOldFont); 
    }
}

void
pntText(HDC hdc, int x, int y, const char * text)
{
    pntText(hdc, x, y, text, strlen(text));
}

void
pntSample(HDC hdc)
{
    pntText(hdc, 0, 0, "This is just a sample text");
}
