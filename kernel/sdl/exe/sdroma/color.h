#ifndef COLOR_H
#define COLOR_H

#include "common.h"

// color structure
struct Color
{
    unsigned char R, G, B;
    unsigned char padding;
};

inline COLORREF 
getColor(const Color & c)
{
    return RGB(c.R, c.G, c.B);
}

inline COLORREF 
getColor(const Color & c, int bright)
{
    bright = 255 - bright;
    return RGB(c.R + ((((255 - c.R) * bright ) >> 8)),
            (c.G + (((255 - c.G) * bright) >> 8)),
            (c.B + (((255 - c.B) * bright) >> 8)));
}

#endif // COLOR_H
