#include "common.h"

static int dataRate = -1;
static unsigned char softBits[1024];
static unsigned char diSoftBits[1024];
static unsigned int header;
static unsigned short usLength;
static int symbolLeft = 0;

inline static char 
parseHeader(unsigned int uiSignal, 
        unsigned short * pusLength,
        unsigned char * pbRate)
{
    unsigned int uiParity;
    int fails = 0;
    uiSignal &= 0xFFFFFF;
    printf ( "PLCP Header detected (%X)\n", uiSignal);
    
    if (uiSignal & 0xFC0010) // all these bits should be always zero
    {
        printf ( "Error: Tail or reserved bit are non-zeor! (%x)\n", 
                 uiSignal & 0xFC0010);
//      return 0;
        fails ++;
    }
    
    uiParity = (uiSignal >> 16) ^ (uiSignal);
    uiParity = (uiParity >> 8) ^ (uiParity);
    uiParity = (uiParity >> 4) ^ (uiParity);
    uiParity = (uiParity >> 2) ^ (uiParity);
    uiParity = (uiParity >> 1) ^ (uiParity);
    if (uiParity & 0x1) {
        printf ( "Error: Parity check failed!\n" );
        fails ++;
    }


    (*pbRate) = uiSignal & 0xF;
    if (!((*pbRate) & 0x8)) {
        printf ( "Error: Incompatible rate %x\n", *pbRate );
        fails ++;
    }
    
    (*pusLength) = (uiSignal >> 5) & 0xFFF;

    if ( fails ) return 0;
    else  return 1;
}

void demap(const Complex * pc, unsigned char * poutput, int rate);
void deinterleave(unsigned char * input, unsigned char * output, int rate);
void viterbiHeader(char * softBits, char * output);

double calc_snr ( const Complex* pc ) {
	double evm = 0;

	int i;
	for (i = 64 - 26; i < 64; i++) {
		double dre = abs(pc[i].re) - 1;
		double dim = abs(pc[i].im) ;

		evm += (dre * dre + dim * dim);
//		printf ( "subcarrier %d: <%.3lf, %.3lf> - evm %.3lf\n", i, pc[i].re, pc[i].im, (dre * dre + dim * dim));
	}

	for (i = 1; i < 26; i++) {
		double dre = abs(pc[i].re) - 1;
		double dim = abs(pc[i].im) ;

		evm += (dre * dre + dim * dim);
//		printf ( "subcarrier %d: <%.3lf, %.3lf> - evm %.3lf\n", i, pc[i].re, pc[i].im, (dre * dre + dim * dim));
	}

	evm /= 52; // average evm

	double snr = 10 * log10 ( 1/evm );
	return snr;
}

void pushDecode(const Complex * pc)
{
    unsigned char bRate;

    if (dataRate < 0)
    {
        // Kun:
        int i;
        for (i = 64 - 26; i < 64; i++)
        {
            if ( i == 64 -7 || i == 64 - 21 ) continue;
            fdprintf ( 1, " %lf %lf\n", 
                    pc[i].re, pc[i].im);
        }


        for (i = 1; i <= 26; i++)
        {
            if ( i == 7 || i == 21 ) continue;
            fdprintf ( 1, " %lf %lf\n", 
                    pc[i].re, pc[i].im);
        }
        fdprintf ( 1, "\n" ); 

		// compute snr here
		double snr = calc_snr ( pc );
		printf ( "SNR estimated: %.3lf dB\n", snr );

        // this should be header symbol
        demap(pc, softBits, 6);
        deinterleave(softBits, diSoftBits, 6);
        viterbiHeader((char *)diSoftBits, (char *)(&header));
        if (!parseHeader(header, &usLength, &bRate))
        {
            RECT rc;
            GetClientRect  ( gHwnd, &rc );
            InvalidateRect ( gHwnd, &rc, FALSE ); 
            UpdateWindow   ( gHwnd );

            clearReceiverState();
           
        }
        else
        {
        
            setByteTotal(usLength);
            resetPilotCounter();
            switch (bRate)
            {
                case 8: dataRate = 48; break;
                case 9: dataRate = 24; break;
                case 10: dataRate = 12; break;
                case 11: dataRate = 6; break;
                case 12: dataRate = 54; break;
                case 13: dataRate = 36; break;
                case 14: dataRate = 18; break;
                case 15: dataRate = 9; break;
                default: 
                    dprintf("ERROR: unknown data rate in pushDecode()");
                    dataRate = -1;
                    clearReceiverState();
                    break;
            }
			
            printf ( "Data rate %d Mbps | Frame len %d bytes \n", 
				dataRate, usLength );        
            /*
            dprintf("%06X len:%d %02X rate: %d", 
                    header, (int)(usLength), (int)(bRate), dataRate);
            */

            if (dataRate > 0)
            {
                int DBPS = 0;
                switch (bRate)
                {
                    case 8: DBPS = 192; break;
                    case 9: DBPS = 96; break;
                    case 10: DBPS = 48; break;
                    case 11: DBPS = 24; break;
                    case 12: DBPS = 216; break;
                    case 13: DBPS = 144; break;
                    case 14: DBPS = 72; break;
                    case 15: DBPS = 36; break;
                }

                symbolLeft = (((usLength << 3) + (22 - 1 + DBPS)) / DBPS);
            }
            else
            {
                symbolLeft = 0;
            }
        }
    }
    else if (symbolLeft > 0)
    {
        // dprintf("symbol left: %d", symbolLeft);
        symbolLeft--;
        demap(pc, softBits, dataRate);
        deinterleave(softBits, diSoftBits, dataRate);
        pushViterbi(diSoftBits, dataRate);

        if (symbolLeft == 0)
        {
            pushEndViterbi(dataRate);
            clearReceiverState();
        }
    }
}

static HFONT hBitFont = CreateFontA(
        15,                     // height of font
        7,                      // average character width
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
        "Courier New"                   // typeface name
    );

static const int DEMAP_X = 10;
static const int DEMAP_Y = 10 + 300 + 300;
static const int DEMAP_H = 50;
static const int DEMAP_W = 1200;

static void pntDemap(HDC hdc)
{
    int m = getBorderMargin();
    int x = DEMAP_X + m + 3;
    int y = DEMAP_Y + m + 15;
    const int row = 15;
    const int col = 7;

    HFONT hOldFont = (HFONT) SelectObject(hdc, hBitFont); 
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 144; j++)
        {
            int index = i * 144 + j;
            if (softBits[index] == (unsigned char)(-1))
                continue;
            int px = x + j * col;
            int py = y + i * row;
            char c; int conf;
            if (softBits[index] < 3) 
            {
                c = '0'; 
                conf = softBits[index] - 0;
            }
            else
            {
                c = '1';
                conf = 7 - softBits[index];
            }
            
            if (conf == 0)
                SetTextColor(hdc, CS(COLOR_BIT_CONF));
            else if (conf == 1)
                SetTextColor(hdc, CS(COLOR_BIT_OKAY));
            else if (conf == 2)
                SetTextColor(hdc, CS(COLOR_BIT_SUSP));
            else
                SetTextColor(hdc, CS(COLOR_BIT_AMBI));
            TextOutA(hdc, px, py, &c, 1);
        }
    }
    SetTextColor(hdc, CS(COLOR_TEXT));
    SelectObject(hdc, hOldFont);

    pntBorder(hdc, DEMAP_X, DEMAP_Y, DEMAP_H, DEMAP_W, "Demapped bits");
}

static const int DEINTERLEAVE_X = 10;
static const int DEINTERLEAVE_Y = 10 + 300 + 300 + 50;
static const int DEINTERLEAVE_H = 50;
static const int DEINTERLEAVE_W = 1200;

static void pntDeinterleave(HDC hdc)
{
    int m = getBorderMargin();
    int x = DEINTERLEAVE_X + m + 3;
    int y = DEINTERLEAVE_Y + m + 15;
    const int row = 15;
    const int col = 7;

    HFONT hOldFont = (HFONT) SelectObject(hdc, hBitFont); 
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 144; j++)
        {
            int index = i * 144 + j;
            if (diSoftBits[index] == (unsigned char)(-1))
                continue;
            int px = x + j * col;
            int py = y + i * row;
            char c; int conf;
            if (diSoftBits[index] < 3) 
            {
                c = '0'; 
                conf = diSoftBits[index] - 0;
            }
            else
            {
                c = '1';
                conf = 7 - diSoftBits[index];
            }
            
            if (conf == 0)
                SetTextColor(hdc, CS(COLOR_BIT_CONF));
            else if (conf == 1)
                SetTextColor(hdc, CS(COLOR_BIT_OKAY));
            else if (conf == 2)
                SetTextColor(hdc, CS(COLOR_BIT_SUSP));
            else
                SetTextColor(hdc, CS(COLOR_BIT_AMBI));
            TextOutA(hdc, px, py, &c, 1);
        }
    }
    SetTextColor(hdc, CS(COLOR_TEXT));
    SelectObject(hdc, hOldFont);

    pntBorder(hdc, DEINTERLEAVE_X, DEINTERLEAVE_Y, DEINTERLEAVE_H, DEINTERLEAVE_W, 
            "Deinterleaved bits");
}

static const int HEADER_X = 10 + 300 + 300 ;
static const int HEADER_Y = 10 + 300  ;
static const int HEADER_H = 100;
static const int HEADER_W = 200;

void pntHeaderInfo(HDC hdc)
{
    if (dataRate >= 0)
    {
        int m = getBorderMargin();
        int x = HEADER_X + m;
        int y = HEADER_Y + 2;
        
        char buf[1024];
        int i = 0;
        sprintf(buf, "Frame data rate: %d Mb/s", dataRate);
        pntText(hdc, x, y + 20 * i, buf); i++;
        sprintf(buf, "Frame length: %d bytes", usLength);
        pntText(hdc, x, y + 20 * i, buf); i++;
        sprintf(buf, "Symbol left: %d", symbolLeft);
        pntText(hdc, x, y + 20 * i, buf); i++;
    }

    pntBorder(hdc, HEADER_X, HEADER_Y, HEADER_H, HEADER_W, "");
}

void pntDecode(HDC hdc)
{
    pntDemap(hdc);
    pntDeinterleave(hdc);
}

void clearDecode()
{
    memset(softBits, -1, 288);
    memset(diSoftBits, -1, 288);
    dataRate = -1;
    symbolLeft = 0;
    clearViterbi();
}

void resetDecode()
{
    resetViterbi();
    clearDecode();
}

int getDataRate()
{
    return dataRate;
}
