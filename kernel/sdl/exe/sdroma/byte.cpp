#include "common.h"

static unsigned char descrambleSeed = 0;
static int bytesTotal = -1;
static int bytesCount = -2;
static int countStore = -1;

static const unsigned char SCRAMBLE_11A_LUT[128] = {
    0x00, 0x91, 0x22, 0xb3, 0x44, 0xd5, 0x66, 0xf7, 
    0x19, 0x88, 0x3b, 0xaa, 0x5d, 0xcc, 0x7f, 0xee, 
    0x32, 0xa3, 0x10, 0x81, 0x76, 0xe7, 0x54, 0xc5, 
    0x2b, 0xba, 0x09, 0x98, 0x6f, 0xfe, 0x4d, 0xdc, 
    0x64, 0xf5, 0x46, 0xd7, 0x20, 0xb1, 0x02, 0x93, 
    0x7d, 0xec, 0x5f, 0xce, 0x39, 0xa8, 0x1b, 0x8a, 
    0x56, 0xc7, 0x74, 0xe5, 0x12, 0x83, 0x30, 0xa1, 
    0x4f, 0xde, 0x6d, 0xfc, 0x0b, 0x9a, 0x29, 0xb8, 
    0xc8, 0x59, 0xea, 0x7b, 0x8c, 0x1d, 0xae, 0x3f, 
    0xd1, 0x40, 0xf3, 0x62, 0x95, 0x04, 0xb7, 0x26, 
    0xfa, 0x6b, 0xd8, 0x49, 0xbe, 0x2f, 0x9c, 0x0d, 
    0xe3, 0x72, 0xc1, 0x50, 0xa7, 0x36, 0x85, 0x14, 
    0xac, 0x3d, 0x8e, 0x1f, 0xe8, 0x79, 0xca, 0x5b, 
    0xb5, 0x24, 0x97, 0x06, 0xf1, 0x60, 0xd3, 0x42, 
    0x9e, 0x0f, 0xbc, 0x2d, 0xda, 0x4b, 0xf8, 0x69, 
    0x87, 0x16, 0xa5, 0x34, 0xc3, 0x52, 0xe1, 0x70, 
};

const unsigned long LUTCRC32[256] = 
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,

    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,

    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,

    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

static char store[8000];
unsigned int CRCcurrent = 0xFFFFFFFF;

static void
updateCRC(char c)
{
    CRCcurrent = (CRCcurrent >> 8)
        ^ LUTCRC32[((unsigned char)(c)) ^ (CRCcurrent & 0xFF)];
}

static void
resetCRC()
{
    CRCcurrent = 0xFFFFFFFF;
}

void
pushRawByte(unsigned char c)
{
    if (bytesTotal < 0)
        return;

    if (bytesCount == -1)
    {
        descrambleSeed = SCRAMBLE_11A_LUT[c >> 1];
    }
    else if (bytesCount >= 0)
    {
        c ^= descrambleSeed;
        descrambleSeed = SCRAMBLE_11A_LUT[descrambleSeed >> 1];
        store[bytesCount] = (char)(c);
        if (bytesCount < bytesTotal - 4)
            updateCRC(c);
    }

    bytesCount++;
    if (bytesCount == bytesTotal)
    {
        bytesTotal = -1;
        bytesCount = -2;

		
		int exp_crc = *((int*)(store + countStore - 4));
		
			
		if (exp_crc == (CRCcurrent ^ 0xFFFFFFFF)) {
			printf ( "Good frame decoded! " );
		}	
		else {
			printf ( "Bad frame decoded! " );
		}	
		
		printf ( "Expected CRC %08x - Actual CRC %08x\n\n", 
			exp_crc, (CRCcurrent ^ 0xFFFFFFFF));


    }
}

void 
clearByte()
{
    bytesTotal = -1;
    bytesCount = -2;
}

void
resetByte()
{
    countStore = -1;
    clearByte();
}

void
setByteTotal(int t)
{ 
    resetCRC();
    countStore = t;
    bytesTotal = t;
    bytesCount = -2;
}

static char buf[20000];

static const int BYTE_X = 10;
static const int BYTE_Y = 10 + 200 + 200;
static const int BYTE_H = 100;
static const int BYTE_W = 800;

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

static const int LINES = 4;
static const int PER_LINE = 24; // 48;
static const int SHOW = LINES * PER_LINE;

extern void pntHeaderInfo(HDC hdc);
void
pntByte(HDC hdc)
{
    pntHeaderInfo(hdc);

    int current = bytesCount;
    if (current < 0)
        current = countStore;
    if (current >= 0)
    {
        int start = 0;
        int count = current;

        if (current > SHOW)
        {
            start = current - SHOW;
            count = SHOW;
        }

        HFONT hOldFont = (HFONT) SelectObject(hdc, hBitFont);
        
        char * pt;
		char chbuf[PER_LINE+1];
		int  chcnt; 
        for (int i = 0; i < LINES; i++)
        {
            pt = buf;
			chcnt = 0;
		
            for (int j = i * PER_LINE; j < count && 
                    j < (i + 1) * PER_LINE; j++)
            {
				unsigned char ch = (unsigned char) store[start + j];
				if ( (ch >= 32 && ch <= 126) )
					chbuf[chcnt++] = ch;
				else 
					chbuf[chcnt++] = '.';

				sprintf(pt, "%02X ", (unsigned char)(store[start + j]));
                pt += 3;
            }

			chbuf [chcnt] = 0;
			for ( ; chcnt < PER_LINE; chcnt ++ ) {
				sprintf ( pt, "   " );
				pt += 3;
			}

			sprintf ( pt, " %s", chbuf );

            if (i * PER_LINE < count)
                TextOutA(hdc, BYTE_X + 10, BYTE_Y + 17 + 16 * i, buf, strlen(buf));
        }

        pt = buf;
        char * pcrc = (char *)(&CRCcurrent);
        for (int i = 0; i < 4; i++)
        {
            sprintf(pt, "%02X ", 0xFF ^ (unsigned char)(pcrc[i]));
            pt += 3;
        }

        COLORREF color = CS(COLOR_CRC_NORMAL);
        
        if (bytesTotal < 0 && countStore >= 4)
        {
            char * pcrc = (char *)(&CRCcurrent);
            char * pcrc2 = (char *)(store + countStore - 4);
            bool allSame = true;

            for (int i = 0; i < 4; i++)
            {
                if ((unsigned char)(pcrc2[i]) != (unsigned char)((0xFF ^ pcrc[i])))
                    allSame = false;
            }

            if (allSame) {
                color = CS(COLOR_CRC_CORRECT);
            }	
            else {
                color = CS(COLOR_CRC_INCORRECT);
            }	

        }

        SetTextColor(hdc, color);
        TextOutA(hdc, BYTE_X + 10 + 800 - 100, BYTE_Y + 17 + 16 * (LINES - 1), 
                buf, strlen(buf));
        SetTextColor(hdc, CS(COLOR_TEXT));

        SelectObject(hdc, hOldFont);
    }

    pntBorder(hdc, BYTE_X, BYTE_Y, BYTE_H, BYTE_W, "Decoded bytes");
}
