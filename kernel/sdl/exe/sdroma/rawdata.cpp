#include "common.h"

static int nSample = 0;
static DConstel dRawData;

static const int RAWDATA_X = 10;
static const int RAWDATA_Y = 10;
static const int RAWDATA_SIZE = 200;

void
pntRawData(HDC hdc)
{
    double r = 2000;
    int index = (dRawData.current - dRawData.count + DCONSTEL_HISLEN) % DCONSTEL_HISLEN;
    for (int i = 0; i < dRawData.count; i++)
    {
        Complex & c = dRawData.data[(index + i) % DCONSTEL_HISLEN];
        double str = c.re * c.re + c.im * c.im;
        while (str > r * r)
            r *= 2;
    }
    constelSetNormalOne(dRawData, r);

    dRawData.lightColor = CS(COLOR_RAW);
    dRawData.darkColor = CS(COLOR_RAWBG);
    pntConstelB(hdc, dRawData, RAWDATA_X, RAWDATA_Y, RAWDATA_SIZE, "Raw data");
}

inline static void 
pushRawDataRecord(Complex & c)
{
    // dprintf("%f %f", c.re, c.im);
    constelPush(dRawData, c);   
}

inline static void 
resetRawDataRecord()
{
    constelClear(dRawData);
}

void
rawDataPaintInit()
{
    constelSetNormalOne(dRawData, 10000);
}

void 
resetData(int point)
{
    nSample = point;
    resetRawDataRecord();
    resetSynchronizer();
}

void autoGetNewDump();
void pushRemoveDC (Complex & c);

extern bool carrierSensing;
extern Complex symbol[];
extern Complex symbolFreq[];

// Stream starts here
static int g_freqindex = 0;
void 
pushData(int count)
{
    for (int i = 0; i < count; i++)
    {
        if (nSample >= getDumpTotalCount())
        {
            //autoGetNewDump();
            //resetData();
			if ( bEndBatch ) {
				// try to execute the end batch file
				system ( EndBatchFile );
				resetData(0);
			}

			if ( bMonitFile ) {
				// Check if file has been changed -- if 
				loadDumpFile ( MonitFile );
			} else {
				if ( flagMonitorMode ) {
					OnlineDump ();
				}
			}

            break;
        }
        Complex * pc = getDumpData(nSample);
        pushRawDataRecord(*pc);

		pushRemoveDC (*pc);

        if ( !carrierSensing ) {
            // push raw FFT
            symbol[g_freqindex+8] = *pc; 
            g_freqindex ++;
            if ( g_freqindex >= 64 ) {
                fft(&symbol[8], symbolFreq);
                g_freqindex = 0;
            }
            
        }
        
    	pushSynchronizer(*pc);
        nSample++;
    }
    // dprintf("current: %d", nSample);
}

int 
getSampleCounter()
{
    return nSample;
}

int 
getDescCounter()
{
    return nSample / 28;
}


static const int GENERAL_X = 10 + 200 + 200 + 200;
static const int GENERAL_Y = 10 + 200 ;
static const int GENERAL_H = 100;
static const int GENERAL_W = 200;

extern int dumpSampleCount;
extern double freq;
extern int LT_boundary;
extern Complex dcOffset;

void pntGeneralInfo(HDC hdc)
{
  int m = getBorderMargin();
  int x = GENERAL_X + m;
  int y = GENERAL_Y + 1;
        
  char buf[1024];
  int i = 0;
  sprintf(buf, "Total Sample Count: %d", dumpSampleCount);
  pntText(hdc, x, y + 20 * i, buf); i++;

  sprintf(buf, "Current Sample: %d", getSampleCounter () );
  pntText(hdc, x, y + 20 * i, buf); i++;

  sprintf ( buf, "Freq offset %lf", freq );
  pntText (hdc, x, y + 20*i, buf); i++;

  sprintf ( buf, "LT Boundary %d", LT_boundary );
  pntText (hdc, x, y + 20*i, buf); i++;

  sprintf ( buf, "DC offset {%.1lf, %.1lf}", dcOffset.re, dcOffset.im );
  pntText (hdc, x, y + 20*i, buf); i++;



  pntBorder(hdc, GENERAL_X, GENERAL_Y, GENERAL_H, GENERAL_W, "");
}
