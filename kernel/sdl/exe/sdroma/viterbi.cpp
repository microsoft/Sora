#include "common.h"

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
#define GETA_1(c, ret) GETA_0(c, ret); ret ^= 0x1
#define GETB_1(c, ret) GETB_0(c, ret); ret ^= 0x1

static unsigned char gValues_[INPUT_MAX * SPACE_SIZE];

void
viterbiHeader(char * inp, char * outp)
{
    unsigned int uiSectionCounter = 0;
    unsigned char i, j, j1, j2;
    unsigned char * pbLastValues = gValues_;
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

        /*
        // normalize
        unsigned char min = pbLastValues[0];
        for (i = 1; i < 64; i++)
            if (pbLastValues[i] < min)
                min = pbLastValues[i];
        min &= 0xFE;
        for (i = 0; i < 64; i++)
            pbLastValues[i] -= min;
        */
    }
}

static unsigned char * viterbiPointer = gValues_;
static int uiSectionCounter = 0;

void
clearViterbi()
{
    uiSectionCounter = 0;
    viterbiPointer = gValues_;
    clearByte();
}

static const int TRACEBACK_DEPTH = 96;

static void 
viterbiOutput()
{
    if (uiSectionCounter > (6 + TRACEBACK_DEPTH) &&
            ((uiSectionCounter - 6 - TRACEBACK_DEPTH) % 8 == 0))
    {
        unsigned char min = viterbiPointer[0];
        int minIndex = 0;
        unsigned char * pbTraceBack = viterbiPointer;
        int byteCount = (uiSectionCounter - 6 - TRACEBACK_DEPTH) / 8;
        
        for (int i = 1; i < 64; i++)
        {
            if (viterbiPointer[i] < min)
            {
                min = viterbiPointer[i];
                minIndex = i;
            }
        }
        min &= 0xFE;
        for (int i = 0; i < 64; i++)
            viterbiPointer[i] -= min;

        pbTraceBack = viterbiPointer;
        
        for (int i = 0; i < TRACEBACK_DEPTH; i++)
        {
            minIndex |= (pbTraceBack[minIndex] & 0x1) << 6;
            minIndex >>= 1;
            pbTraceBack -= 64;
        }

        for (int i = 0; i < byteCount; i++)
        {
            unsigned char c = 0;
            for (int j = 0; j < 8; j++)
            {
                c <<= 1;
                c |= (pbTraceBack[minIndex] & 0x1);
                minIndex |= (pbTraceBack[minIndex] & 0x1) << 6;
                minIndex >>= 1;
                pbTraceBack -= 64;
            }
            // dprintf("%02X", (unsigned int)(c));
            pushRawByte(c);
        }
        uiSectionCounter -= 8;
    }
}

void
pushViterbi(unsigned char * inp, int rate)
{
    if (uiSectionCounter == 0)
    {
        for (int i = 0; i < 64; i++)
            viterbiPointer[i] = 0x30;
        viterbiPointer[0] = 0;
    }

    unsigned char * nextValues = viterbiPointer + 64;
    unsigned char iA, iB, A, B;
    unsigned char m1, m2;
    unsigned char j1, j2;

    while (1)
    {
        if (*inp == 0xFF)
            return;

        if (rate == 6 || rate == 12 || rate == 24)
        {
            iA = *(inp++) << 1;
            iB = *(inp++) << 1;

            for (int i = 0; i < 64; i++)
            {
                j1 = i >> 1;
                j2 = j1 | 0x20;
                
                GETA_0(i, A);
                GETB_0(i, B);
                m1 = viterbiPointer[j1];
                if (A) m1 += 14-iA; else m1 += iA;
                if (B) m1 += 14-iB; else m1 += iB;

                GETA_1(i, A);
                GETB_1(i, B);
                m2 = viterbiPointer[j2];
                if (A) m2 += 14-iA; else m2 += iA;
                if (B) m2 += 14-iB; else m2 += iB;

                if (m1 < m2)
                    nextValues[i] = m1 & 0xFE;
                else
                    nextValues[i] = m2 | 0x01;
            }


            nextValues += 64;
            viterbiPointer += 64;

            uiSectionCounter++;
            viterbiOutput();
        }
        else if (rate == 48) // 2_3
        {
            iA = *(inp++) << 1;
            iB = *(inp++) << 1;

            for (int i = 0; i < 64; i++)
            {
                j1 = i >> 1;
                j2 = j1 | 0x20;
                
                GETA_0(i, A);
                GETB_0(i, B);
                m1 = viterbiPointer[j1];
                if (A) m1 += 14-iA; else m1 += iA;
                if (B) m1 += 14-iB; else m1 += iB;

                GETA_1(i, A); 
                GETB_1(i, B);
                m2 = viterbiPointer[j2];
                if (A) m2 += 14-iA; else m2 += iA;
                if (B) m2 += 14-iB; else m2 += iB;

                if (m1 < m2)
                    nextValues[i] = m1 & 0xFE;
                else
                    nextValues[i] = m2 | 0x01;
            }

            nextValues += 64;
            viterbiPointer += 64;

            uiSectionCounter++;
            viterbiOutput();

            iA = *(inp++) << 1;

            for (int i = 0; i < 64; i++)
            {
                j1 = i >> 1;
                j2 = j1 | 0x20;
                
                GETA_0(i, A);
                m1 = viterbiPointer[j1];
                if (A) m1 += 14 - iA; else m1 += iA;

                GETA_1(i, A);
                m2 = viterbiPointer[j2];
                if (A) m2 += 14 - iA; else m2 += iA;

                if (m1 < m2)
                    nextValues[i] = m1 & 0xFE;
                else
                    nextValues[i] = m2 | 0x01;
            }

            nextValues += 64;
            viterbiPointer += 64;

            uiSectionCounter++;
            viterbiOutput();
        }
        else
        {
            iA = *(inp++) << 1;
            iB = *(inp++) << 1;

            for (int i = 0; i < 64; i++)
            {
                j1 = i >> 1;
                j2 = j1 | 0x20;
                
                GETA_0(i, A);
                GETB_0(i, B);
                m1 = viterbiPointer[j1];
                if (A) m1 += 14 - iA; else m1 += iA;
                if (B) m1 += 14 - iB; else m1 += iB;

                GETA_1(i, A);
                GETB_1(i, B);
                m2 = viterbiPointer[j2];
                if (A) m2 += 14 - iA; else m2 += iA;
                if (B) m2 += 14 - iB; else m2 += iB;

                if (m1 < m2)
                    nextValues[i] = m1 & 0xFE;
                else
                    nextValues[i] = m2 | 0x01;
            }

            nextValues += 64;
            viterbiPointer += 64;

            uiSectionCounter++;
            viterbiOutput();

            iA = *(inp++) << 1;

            for (int i = 0; i < 64; i++)
            {
                j1 = i >> 1;
                j2 = j1 | 0x20;
                
                GETA_0(i, A);
                m1 = viterbiPointer[j1];
                if (A) m1 += 14 - iA; else m1 += iA;

                GETA_1(i, A);
                m2 = viterbiPointer[j2];
                if (A) m2 += 14 - iA; else m2 += iA;

                if (m1 < m2)
                    nextValues[i] = m1 & 0xFE;
                else
                    nextValues[i] = m2 | 0x01;
            }

            nextValues += 64;
            viterbiPointer += 64;
            
            uiSectionCounter++;
            viterbiOutput();

            iB = *(inp++) << 1;

            for (int i = 0; i < 64; i++)
            {
                j1 = i >> 1;
                j2 = j1 | 0x20;
                
                GETB_0(i, B);
                m1 = viterbiPointer[j1];
                if (B) m1 += 14-iB; else m1 += iB;
                
                GETB_1(i, B);
                m2 = viterbiPointer[j2];
                if (B) m2 += 14-iB; else m2 += iB;

                if (m1 < m2)
                    nextValues[i] = m1 & 0xFE;
                else
                    nextValues[i] = m2 | 0x01;
            }
            
            nextValues += 64;
            viterbiPointer += 64;
 
            uiSectionCounter++;
            viterbiOutput();
        }

        // normalize
        unsigned char min = viterbiPointer[0];
        for (int i = 1; i < 64; i++)
            if (viterbiPointer[i] < min)
                min = viterbiPointer[i];
        min &= 0xFE;
        for (int i = 0; i < 64; i++)
            viterbiPointer[i] -= min;
    }
}

static unsigned char allZero[289] = { 0 };

void
pushEndViterbi(int rate)
{
    allZero[288] = 0xFF;
    pushViterbi(allZero, rate);

    clearViterbi();
    return;    
}

void
resetViterbi()
{
    resetByte();
    clearViterbi();
}
