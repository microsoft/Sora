#pragma once
#include <dspcomm.h>

#define BYTE_MAX    4096
#define BIT_MAX     (BYTE_MAX * 8)
#define SYMBOL_MAX  160
#define INPUT_MAX   (SYMBOL_MAX * 288)

#define SPACE_SIZE      64

#define METRIC_SIZE     128
#define METRIC_OFFSET   7

#define GETSUM(c, ret) \
    ret = c; \
    ret ^= (ret >> 4); \
    ret ^= (ret >> 2); \
    ret ^= (ret >> 1); \
    ret &= 0x1

// pre 0
#define GETA_0(c, ret) GETSUM((c & 0x2D), ret)
#define GETB_0(c, ret) GETSUM((c & 0x0F), ret)

// pre 1
#define GETA_1(c, ret) {GETA_0(c, ret); ret ^= 0x1;}
#define GETB_1(c, ret) {GETB_0(c, ret); ret ^= 0x1;}

extern uchar gTrilles_[INPUT_MAX * SPACE_SIZE];

FINL void ViterbiHeader(uchar * inp, uchar * outp)
{
    unsigned int uiSectionCounter = 0;
    unsigned char i, j, j1, j2;
    unsigned char * pbLastValues = gTrilles_;
    unsigned char * pbNextValues = pbLastValues + 64;
    unsigned char iA, iB, A, B;
    unsigned char m1, m2;
    
    for (i = 0; i < 64; i++)
    {
        pbLastValues[i] = 0x30;
    }
    pbLastValues[0] = 0;

    while (1)
    {
        // multiple by 2
        iA = *(inp++) << 1;
        iB = *(inp++) << 1;

        // calculate values
        for (i = 0; i < 64; i++)
        {
            j1 = i >> 1;
            j2 = j1 | 0x20;
            
            GETA_0(i, A);
            GETB_0(i, B);
            
            m1 = pbLastValues[j1];

            if (A) m1 += 14-iA; else m1 += iA;
            if (B) m1 += 14-iB; else m1 += iB;

            GETA_1(i, A);
            GETB_1(i, B);

            m2 = pbLastValues[j2];

            if (A) m2 += 14-iA; else m2 += iA;
            if (B) m2 += 14-iB; else m2 += iB;

            if (m1 < m2)
                pbNextValues[i] = m1 & 0xFE;
            else
                pbNextValues[i] = m2 | 0x01;
        }

        pbNextValues += 64;
        pbLastValues += 64;

        uiSectionCounter++;
        if (uiSectionCounter == 24)
        {
            int minIndex = 0;
            unsigned char * pbTraceBack = pbLastValues;

            outp += 3; // move forward pointer
            for (i = 0; i < 3; i++)
            {
                unsigned char c = 0;
                for (j = 0; j < 8; j++)
                {
                    c <<= 1;
                    if (i == 0 && j < 6)
                        ;
                    else
                    {
                        c |= (pbTraceBack[minIndex] & 0x1);
                        minIndex |= (pbTraceBack[minIndex] & 0x1) << 6;
                        minIndex >>= 1;
                        pbTraceBack -= 64;
                    }
                }
                *(--outp) = c;
            }
            return;
        }
    }
}

