#include "common.h"

#include <vector128.h>
#include <rxstream.h>

#ifdef IGNORE_VALID_BIT
#define CHECK_VALID_BIT 0
#else
#define CHECK_VALID_BIT 1
#endif

static const int DUMP_SAMPLE_MAX = 1024 * 1024 * 16;
static const int COMPLEX_PER_DESC = 28;

COMPLEX16 dumpData_raw[DUMP_SAMPLE_MAX]; // raw data
int dumpSampleCount_raw;

Complex dumpData[DUMP_SAMPLE_MAX];     // data after proper sampling rate adjustment
int dumpSampleCount;

static Complex blank = { 0, 0 };

struct Int16Pair { short i1, i2; };


struct DumpDescriptor
{
    char validBit;
    char paddings[15];
    Int16Pair data[COMPLEX_PER_DESC];
};

inline static short 
parseLittle14(short s)
{
    return s;
    //return ((short)(((unsigned short)(s)) << 2)) >> 2;
}

Complex dcOffset;
static void
removeDCOffset()
{
    const int DC_TRAIN_LEN = 100000;
    Complex sum = { 0, 0 };

    int count = DC_TRAIN_LEN;
    if (count > dumpSampleCount)
        count = dumpSampleCount;

    for (int i = 0; i < count; i++)
        cadd(sum, dumpData[i]);

    cdiv(dcOffset, sum, count);

    for (int i = 0; i < dumpSampleCount; i++)
        csub(dumpData[i], dcOffset);
}

static const int OVERVIEW_X = 10;
static const int OVERVIEW_Y = 10 + 200 + 200 + 100;
static const int OVERVIEW_H = 100;
static const int OVERVIEW_W = 800;

static int overviewH[1200];
static double overviewV[1200];

void DownConvert44Mto40M () {
	int idx40 = 0;
	int idx44 = 0;

	int idx = 0;
	
	while (idx44 < dumpSampleCount_raw ) {
		if (idx == 0) {
			dumpData[idx40++].re = dumpData_raw[idx44++].re;
			dumpData[idx40++].im = dumpData_raw[idx44++].im;
			idx ++; 
		} else {
			dumpData[idx40].re = ((10-idx)*dumpData_raw[idx44].re + idx*dumpData_raw[idx44+1].re) / 10; 	
			dumpData[idx40].im = ((10-idx)*dumpData_raw[idx44].im + idx*dumpData_raw[idx44+1].im) / 10;
			idx40 ++; idx44++;

			idx ++;
			if (idx > 9 ) {
				idx = 0;
				idx44++; // skip one sample
			}
		}
	}
	
	dumpSampleCount = idx40;
}

int 
OnlineDump () {
	resetData ();

	dumpSampleCount_raw = dumpSampleCount = 0;

	// read to the end of the buffer
	SignalBlock sblk;
	HRESULT hr;
	FLAG    bEnd = false;
	while ( !bEnd ) {
		hr = SoraRadioReadRxStream ( &RxStream, &bEnd, sblk );
		if ( FAILED (hr)) {
			printf ( "Warning: Online dump failed!\n" );

			dumpSampleCount = 0;
			return 0;
		}
	}

	// Start to dump -- 16M
	int num_desc = DUMP_SAMPLE_MAX / sizeof (DumpDescriptor);
	for ( int i=0; i< num_desc; i++) {
		hr = SoraRadioReadRxStream ( &RxStream, &bEnd, sblk );
		if ( FAILED (hr)) {
			printf ( "Warning: Online dump failed!\n" );
			dumpSampleCount_raw = dumpSampleCount = 0;
			return 0;
		}
		COMPLEX16* pSample = (COMPLEX16*) sblk._data;
		for (int i = 0; i < COMPLEX_PER_DESC; i++)
        {
            dumpData_raw[dumpSampleCount_raw].re = pSample[i].re;
            dumpData_raw[dumpSampleCount_raw].im = pSample[i].im;
			dumpSampleCount_raw ++;
        }
	
	}
	
	printf ( "Total %d samples are dumped.\n\n", dumpSampleCount_raw );

	// Sampling rate adjustment
	if ( g_resample44_40 ) {
	    DownConvert44Mto40M ();		
		printf ( "44->40 convertion \n" );
	}

	// decimation
	for (int i = 0; i < dumpSampleCount_raw / g_decimation; i++)
	{
	    dumpData[i].re = dumpData_raw[i*g_decimation].re;
		dumpData[i].im = dumpData_raw[i*g_decimation].im;
	}
	dumpSampleCount = dumpSampleCount_raw / g_decimation;

    removeDCOffset();

	// always show the entire trace
    setOverviewRange(0, dumpSampleCount );

    return dumpSampleCount;

}

unsigned char fakehdr[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

int 
writeDumpFile(char * fn) {

	FILE* fout = fopen (fn, "wb" );
	if ( !fout ) return -1;

	int index = 0;
	while ( index < dumpSampleCount_raw )
	{
		// write a block
		fwrite ( fakehdr, 1, 16, fout );
		for ( int i=0; i<COMPLEX_PER_DESC; i++) {
			fwrite ( &dumpData_raw[index], sizeof(COMPLEX16), 1, fout );
			index ++;
		}
	}

	fclose (fout );
	return dumpSampleCount_raw;
}

int 
loadDumpFile(char * fn)
{
    resetData();

    FILE * fin = fopen(fn, "rb");
    if (!fin)
        return -1; // open file error;   
 
    if (sizeof(DumpDescriptor) != 128)
        dprintf("WARNING: length DumpDescriptor not equal to 128");

    dumpSampleCount_raw = dumpSampleCount = 0;

    DumpDescriptor desc;
    while (1)
    {
        if (!fread(&desc, sizeof(desc), 1, fin))
            break;
        if (CHECK_VALID_BIT && !desc.validBit)
            break;
       
       
       for (int i = 0; i < COMPLEX_PER_DESC; i++)
        {
            dumpData_raw[dumpSampleCount_raw].re = desc.data[i].i1;
            dumpData_raw[dumpSampleCount_raw].im = desc.data[i].i2;

            dumpSampleCount_raw ++;
        }
    }

    fclose(fin);
	
	printf ( "Load dump file %s\n""%d samples are loaded.\n\n",
		fn, dumpSampleCount_raw );

	
	// Sampling rate adjustment
	if ( g_resample44_40 ) {
	    DownConvert44Mto40M ();		
	}
	
	// decimation

	for (int i = 0; i < dumpSampleCount_raw / g_decimation; i++)
	{
	    dumpData[i].re = dumpData_raw[i*g_decimation].re;
		dumpData[i].im = dumpData_raw[i*g_decimation].im;
	}
	dumpSampleCount = dumpSampleCount_raw / g_decimation;


    removeDCOffset();

	// always show the entire trace
    setOverviewRange(0, dumpSampleCount );

    return dumpSampleCount;
}

Complex * 
getDumpData(int index)
{
    if (index >= dumpSampleCount)
        return &blank;

    return &dumpData[index];
}

int
getDumpTotalCount()
{
    return dumpSampleCount;
}

static int overviewStart;
static int overviewEnd;

void pntOverview(HDC hdc)
{
    HPEN hpen = CreatePen(PS_SOLID, 1, CS(COLOR_ENERGY));
    int m = getBorderMargin();
    int x = OVERVIEW_X + m;
    int y = OVERVIEW_Y + 13;
    int w_ = OVERVIEW_W - 2 * m;
    int h_ = (OVERVIEW_H - 15);

    int pt = getSampleCounter();
    if (isPlaying() && pt >= overviewStart + (overviewEnd - overviewStart) * 9 / 10)
    {
        overviewPageDown();
    }
    if (isPlaying() && (pt >= overviewEnd || pt < overviewStart))
    {
        overviewCenter();
    }
    
    SelectObject(hdc, hpen);
    for (int i = 0; i < w_; i++)
    {
        if (overviewH[i] < 0)
            break;
        int px = x + i;
        int py = y + h_ - overviewH[i];
        if (i == 0)
            MoveToEx(hdc, px, py, NULL);
        else
            LineTo(hdc, px, py);
    }
    DeleteObject(hpen);

    SelectObject(hdc, getGreyPen());
    int count = overviewEnd - overviewStart;

    if (pt >= overviewStart && pt < overviewEnd)
    {
        pt = (int)((double)(pt - overviewStart) * w_ / count);
        if (pt > w_) pt = w_;
        MoveToEx(hdc, x + pt, y, NULL);
        LineTo(hdc, x + pt, y + h_);
    }

    pntBorder(hdc, OVERVIEW_X, OVERVIEW_Y, OVERVIEW_H, OVERVIEW_W, "Overview"); 
}

int
getPointerTime(int px, int py)
{
    int m = getBorderMargin();
    int x = OVERVIEW_X + m;
    int y = OVERVIEW_Y + 13;
    int w_ = OVERVIEW_W - 2 * m;
    int h_ = (OVERVIEW_H - 15);

    px -= x;
    py -= y;
    if (px < 0) return -1;
    if (px > w_) return -1;
    if (py < 0) return -1;
    if (py > h_) return -1;
    
    int count = overviewEnd - overviewStart;

    if (count > w_)
    {
        return overviewStart + int(double(px) / w_ * count);
    }
    else
    {
        return px;
    }
}


void
setOverviewRange(int start, int end)
{
    int m = getBorderMargin();
    int w_ = OVERVIEW_W - 2 * m; // this is overview valid point count
    if (start > dumpSampleCount)
        start = dumpSampleCount;
    if (end < start)
        end = dumpSampleCount;
    int showCount = end - start;
    if (showCount < w_ * 32)
    {
        showCount = w_ * 32;
        end = start + showCount;
    }
    overviewStart = start;
    overviewEnd = end;

    memset(overviewH, 0, sizeof(int) * w_);
    for (int i = 0; i < w_; i++)
        overviewV[i] = -1;

    // dprintf("start: %d  end: %d", overviewStart, overviewEnd);
	double max_v = -1;

    if (showCount > w_)
    {
        for (int i = start; i < end && i < dumpSampleCount; i++)
        {
            double str = dumpData[i].re * dumpData[i].re 
                + dumpData[i].im * dumpData[i].im;
            int ind = int((double)(i - start) * w_ / showCount);
            if (str > overviewV[ind])
                overviewV[ind] = str;

			if ( str > max_v ) max_v = str;
        }
    }
    else 
    {
        for (int i = start; i < end && i < dumpSampleCount; i++)
        {
            double str = dumpData[i].re * dumpData[i].re 
                + dumpData[i].im * dumpData[i].im;
            int ind = i - start;
            if (str > overviewV[ind])
                overviewV[ind] = str;

			if ( str > max_v ) max_v = str;
        }
    }
    
	double max_h = 10*log10(max_v);
	printf ( "Overview max: %.2f dB\n", max_h );

    for (int i = 0; i < w_; i++)
    {
        if (overviewV[i] < 0)
            overviewH[i] = -1;
        else
        {
//          double l = log(overviewV[i]) - 10;
//			if (l < 0)
//				l = 0;

            double l = 10 * log10(overviewV[i]);
            overviewH[i] = (int)(l * (OVERVIEW_H - 15) / max_h );
        }
    }
}

void overviewPageUp()
{
    int cnt = overviewEnd - overviewStart;
    int newStart = overviewStart - cnt * 8 / 10;
    if (newStart < 0)
        newStart = 0;
    setOverviewRange(newStart, newStart + cnt);
}

void overviewPageDown()
{
    int cnt = overviewEnd - overviewStart;
    int newStart = overviewStart + cnt * 8 / 10;
    if (newStart < 0)
        newStart = 0;
    if (newStart > dumpSampleCount)
        newStart = dumpSampleCount;
    setOverviewRange(newStart, newStart + cnt);
}

void overviewCenter()
{
    int cnt = overviewEnd - overviewStart;
    int pt = getSampleCounter();
    int newStart = pt - cnt / 2;
    if (newStart < 0)
        newStart = 0;
    setOverviewRange(newStart, newStart + cnt);
}

void overviewZoomIn()
{
    int cnt = overviewEnd - overviewStart;
    int newStart = overviewStart + cnt / 4;
    setOverviewRange(newStart, newStart + cnt / 2);
}

void overviewZoomOut()
{
    int cnt = overviewEnd - overviewStart;
    int newStart = overviewStart - cnt / 2;
    if (newStart < 0)
        newStart = 0;
    cnt *= 2;
    if (cnt > dumpSampleCount)
        cnt = dumpSampleCount;
    setOverviewRange(newStart, newStart + cnt);
}
