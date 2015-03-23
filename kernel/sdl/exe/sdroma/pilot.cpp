#include "common.h"

static const int BEFORE_X = 10 + 200;
static const int BEFORE_Y = 10 + 200;
static const int BEFORE_H = 200;
static const int BEFORE_W = 200;

static const int AFTER_X = 10 + 200 + 200;
static const int AFTER_Y = 10 + 200;
static const int AFTER_H = 200;
static const int AFTER_W = 200;

static bool pilotValid = false;

static Complex beforePilot[64];
static Complex afterPilot[64];

// added by Kun
#define CONST_HIS (640*2)
static Complex constHis[CONST_HIS];
static int hisSize = 0;
static int hisPointer = 0;

static const char pilotSeq[127] = {
	0, 0, 0,-1, -1,-1, 0,-1, -1,-1,-1, 0,  0,-1, 0,-1,
   -1, 0, 0,-1,  0, 0,-1, 0,  0, 0, 0, 0,  0,-1, 0, 0,
    0,-1, 0, 0, -1,-1, 0, 0,  0,-1, 0,-1, -1,-1, 0,-1,
    0,-1,-1, 0, -1,-1, 0, 0,  0, 0, 0,-1, -1, 0, 0,-1,
   -1, 0,-1, 0, -1, 0, 0,-1, -1,-1, 0, 0, -1,-1,-1,-1,
    0,-1,-1, 0, -1, 0, 0, 0,  0,-1, 0,-1,  0,-1, 0,-1,
   -1,-1,-1,-1,  0,-1, 0, 0, -1, 0,-1, 0,  0, 0,-1,-1,
    0,-1,-1,-1,  0, 0, 0,-1, -1,-1,-1,-1, -1,-1, 0	
};

static int pilotCount = 0;


void clearConstHisInfo () {
   hisPointer = hisSize = 0;
}

void pushConstHis(Complex * pSample) {
  int i; 
  int nCount = 64; 
  for ( i = 1; i < 26; i++ ) {
    if ( hisSize < CONST_HIS ) hisSize ++;
    constHis[hisPointer] = pSample[i];
    hisPointer = (hisPointer + 1) % hisSize; 
  }

  for ( i = 64-26; i < 64; i++ ) {
    if ( hisSize < CONST_HIS ) hisSize ++;
    constHis[hisPointer] = pSample[i];
    hisPointer = (hisPointer + 1) % hisSize; 
  }

}



static int getArg(double re, double im)
{
    if (re == 0)
    {
        if (im < 0)
            return 0xFFFF / 4 * 3;
        else
            return 0xFFFF / 4;
    }
    else
    {
        double ag = atan(double(im) / re);
        ag = ag * 0xFFFF / 2 / 3.14159265359;
        if (re < 0)
            ag += 0xFFFF / 2;
        if (ag < 0)
            ag += 0xFFFF;

        if (ag > 0xFFFF / 2)
            ag -= 0xFFFF;
        return static_cast<int>(ag);
    }        
}

extern Complex channel[64];

static void 
pilotCorrection(const Complex * bef, Complex * aft)
{
    int theta1 = getArg(bef[64-21].re, bef[64-21].im);
    int theta2 = (getArg(bef[64-7].re, bef[64-7].im)) & 0xFFFF;
    int theta3 = (getArg(bef[7].re, bef[7].im)) & 0xFFFF;
    int theta4 = (getArg(bef[21].re, bef[21].im) + 0xFFFF / 2) & 0xFFFF;

    int delTheta = (((short)(theta3 - theta1)) / 28 
            + ((short)(theta4 - theta2)) / 28) / 2;

    int avgTheta = (short)((theta1 +
        ((((short)(theta2 - theta1)) 
        + ((short)(theta3 - theta1)) 
        + ((short)(theta4 - theta1))) / 4)));

    // dprintf("pilot count: %d", pilotCount);
    if (pilotSeq[pilotCount])
        avgTheta = (avgTheta + 0xFFFF / 2) & 0xFFFF;
    pilotCount++;
    if (pilotCount == 127) pilotCount = 0;

    int th = avgTheta - delTheta * 26;

    for (int i = 64-26; i < 64; i++)
    {
        double SIN = sin(((double)th) / 0xFFFF * 2 * 3.14159265359);
        double COS = cos(((double)th) / 0xFFFF * 2 * 3.14159265359);

        aft[i].re = bef[i].re * COS + bef[i].im * SIN;
        aft[i].im = bef[i].im * COS - bef[i].re * SIN;
        
        double re, im;

        re = channel[i].re * COS + channel[i].im * SIN;
        im = channel[i].im * COS - channel[i].re * SIN;
        channel[i].re = re;
        channel[i].im = im;
 
        th += delTheta;
        th &= 0xFFFF;
    }

    th += delTheta;
    th &= 0xFFFF;
    for (int i = 1; i <= 26; i++)
    {
        double SIN = sin((double)th / 0xFFFF * 2 * 3.14159265359);
        double COS = cos((double)th / 0xFFFF * 2 * 3.14159265359);

        aft[i].re = bef[i].re * COS + bef[i].im * SIN;
        aft[i].im = bef[i].im * COS - bef[i].re * SIN;
  
        double re, im;
        re = channel[i].re * COS + channel[i].im * SIN;
        im = channel[i].im * COS - channel[i].re * SIN;
        channel[i].re = re;
        channel[i].im = im;
      
        th += delTheta;
        th &= 0xFFFF;
    }
}

void 
pushPilot(const Complex * pc)
{
    pilotValid = true;
    memcpy(beforePilot, pc, sizeof(Complex) * 64);
    pilotCorrection(beforePilot, afterPilot);

    // add by Kun
    pushConstHis(afterPilot);
    
    pushDecode(afterPilot);
}

void
resetPilot()
{
    clearPilot();
    resetDecode();
}

void
clearPilot()
{
    pilotValid = false;
    clearDecode();
}

void
resetPilotCounter()
{
    pilotCount = 0;
}

inline void 
pntDrawPoint(HDC hdc, int centerX, int centerY, int size,
        double r, double r_1, Complex & c, COLORREF color)
{
    double x = c.re * r_1;
    double y = c.im * r_1;

    if (x > 1) x = 1;
    if (x < -1) x = -1;
    if (y > 1) y = 1;
    if (y < -1) y = -1;

    int px = centerX + int(x * size);
    int py = centerY + int(y * size);

    pntDrawCross(hdc, px, py, color);
}

void
pntBeforePilot(HDC hdc)
{
    int m = getBorderMargin();

    int centerX = BEFORE_X + (BEFORE_W / 2);
    int centerY = BEFORE_Y + (BEFORE_H / 2);
    int size = BEFORE_H / 2 - m;

    double r = 1.5;
    double r_1 = 1 / r;
 
    SelectObject(hdc, getGreyPen());
    int one = int(size * r_1);
    Arc(hdc, centerX - one, centerY - one,
            centerX + one, centerY + one,
            0, 0, 0, 0);
    
    MoveToEx(hdc, centerX - size, centerY, NULL); 
    LineTo(hdc, centerX + size, centerY);
    MoveToEx(hdc, centerX, centerY - size, NULL); 
    LineTo(hdc, centerX, centerY + size);

    if (pilotValid)
    {
        COLORREF nm = CS(COLOR_NORMAL);
        for (int i = 1; i <= 26; i++)
        {
            pntDrawPoint(hdc, centerX, centerY, size, r, r_1,
                    beforePilot[i], nm);
        }

        for (int i = 64 - 26; i < 64; i++)
        {
            pntDrawPoint(hdc, centerX, centerY, size, r, r_1,
                    beforePilot[i], nm);
        }

        COLORREF hl = CS(COLOR_HL);
        pntDrawPoint(hdc, centerX, centerY, size, r, r_1, 
                beforePilot[7], hl);
        pntDrawPoint(hdc, centerX, centerY, size, r, r_1, 
                beforePilot[21], hl);
        pntDrawPoint(hdc, centerX, centerY, size, r, r_1, 
                beforePilot[64 - 7], hl);
        pntDrawPoint(hdc, centerX, centerY, size, r, r_1, 
                beforePilot[64 - 21], hl);
    }

    pntBorder(hdc, BEFORE_X, BEFORE_Y, BEFORE_H, BEFORE_W, "Before pilot correction");
}

void
pntAfterPilot(HDC hdc)
{
    int m = getBorderMargin();

    int centerX = AFTER_X + (AFTER_W / 2);
    int centerY = AFTER_Y + (AFTER_H / 2);
    int size = AFTER_H / 2 - m;

    double r = 1.5;
    double r_1 = 1 / r;

    SelectObject(hdc, getGreyPen());
    /*
    int one = int(size * r_1);
    Arc(hdc, centerX - one, centerY - one,
            centerX + one, centerY + one,
            0, 0, 0, 0);
    */
    
    MoveToEx(hdc, centerX - size, centerY, NULL); 
    LineTo(hdc, centerX + size, centerY);
    MoveToEx(hdc, centerX, centerY - size, NULL); 
    LineTo(hdc, centerX, centerY + size);

    if (pilotValid)
    {
        COLORREF nm = CS(COLOR_NORMAL);
    // draw history
    for ( int i = 0; i < hisSize; i++) 
    {
            pntDrawPoint(hdc, centerX, centerY, size, r, r_1,
                   constHis[i], nm);
}
        for (int i = 1; i <= 26; i++)
        {
            pntDrawPoint(hdc, centerX, centerY, size, r, r_1,
                    afterPilot[i], nm);
        }

        for (int i = 64 - 26; i < 64; i++)
        {
            pntDrawPoint(hdc, centerX, centerY, size, r, r_1,
                    afterPilot[i], nm);
        }

        COLORREF hl = CS(COLOR_HL);
        pntDrawPoint(hdc, centerX, centerY, size, r, r_1, 
                afterPilot[7], hl);
        pntDrawPoint(hdc, centerX, centerY, size, r, r_1, 
                afterPilot[21], hl);
        pntDrawPoint(hdc, centerX, centerY, size, r, r_1, 
                afterPilot[64 - 7], hl);
        pntDrawPoint(hdc, centerX, centerY, size, r, r_1, 
                afterPilot[64 - 21], hl);
    }
// kun: no circle around
/*
    int rate = getDataRate();
    SelectObject(hdc, getGreyPen());
    double one = size * r_1;
    int cir, x, y;
    double step = 0;
    switch (rate)
    {
        case 6: case 9:
            cir = 45;
            x = centerX - int(one);
            y = centerY;
            Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);
            x = centerX + int(one);
            Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);

            break;
        case 12: case 18:
            cir = 7;
            x = centerX - int(one);
            y = centerY;
            Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);
            x = centerX + int(one);
            Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);

            cir = 33;
            step = one * sqrt(double(1)/2);
            for (int i = -1; i <= 1; i += 2)
            {
                for (int j = -1; j <= 1; j += 2)
                {
                    x = centerX + int(step * i);
                    y = centerY + int(step * j);
                    Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);
                }
            }            
            break;
        case 24: case 36:
            cir = 7;
            x = centerX - int(one);
            y = centerY;
            Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);
            x = centerX + int(one);
            Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);

            cir = 23;
            step = one * sqrt(double(1)/10);
            for (int i = -3; i <= 3; i += 2)
            {
                for (int j = -3; j <= 3; j += 2)
                {
                    x = centerX + int(step * i);
                    y = centerY + int(step * j);
                    Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);
                }
            }
            break;
        case 48: case 54:
            cir = 7;
            x = centerX - int(one);
            y = centerY;
            Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);
            x = centerX + int(one);
            Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);

            cir = 10;
            step = one * sqrt(double(1)/42);
            for (int i = -7; i <= 7; i += 2)
            {
                for (int j = -7; j <= 7; j += 2)
                {
                    x = centerX + int(step * i);
                    y = centerY + int(step * j);
                    Arc(hdc, x - cir, y - cir, x + cir, y + cir, 0, 0, 0, 0);
                }
            }
            break;
    }
*/
    pntBorder(hdc, AFTER_X, AFTER_Y, AFTER_H, AFTER_W, "After pilot correction");
}
