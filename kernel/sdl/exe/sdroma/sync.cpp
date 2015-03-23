#include "common.h"

bool carrierSensing = false;

static const int AUTOCORR_X = 10 + 200;
static const int AUTOCORR_Y = 10;
static const int AUTOCORR_H = 100;
static const int AUTOCORR_W = 600;

static const int CORR_X = 10 + 200;
static const int CORR_Y = 10 + 100;
static const int CORR_H = 100;
static const int CORR_W = 600;

static const int AVG_STEP = 28;

static const int AUTOCORR_STEP = 48;
static Complex acorrHistory[AUTOCORR_STEP];
static int acorrHisIndex = 0;
static int nCount = 0;

static const int CORR_STEP = 16;
static Complex corrHistory[CORR_STEP] = { 0 };
static double corrReHistory[CORR_STEP] = { 0 };
static int corrHisIndex = 0;

static double strSum = 0;
static double corrSum = 0;
static int posCount = 0;

static double THRESHOLD = 100000/2;
static int counter = 0;

static DWave waveAutoCorr;
static DWave waveStrSum;

static DWave waveCorr;
static DWave waveEnergy;

static const int AUTOCORR_WAVE_HIS_LEN = 220;
static const int CORR_WAVE_HIS_LEN = 440;


// Online DC
static Complex g_dc = {0,0};


static void inline
pushCorrelator(double acorr, double sum)
{
    wavePush(waveAutoCorr, acorr);
    wavePush(waveStrSum, sum);
}

static void
resetCorrelator()
{
    waveReset(waveAutoCorr, AUTOCORR_WAVE_HIS_LEN);
    waveReset(waveStrSum, AUTOCORR_WAVE_HIS_LEN);
    waveReset(waveCorr, CORR_WAVE_HIS_LEN);
    waveReset(waveEnergy, CORR_WAVE_HIS_LEN);
}

void
pntCarrierSensor(HDC hdc)
{
    double up = 10000;
    double down = -7000;
    for (int i = 0; i < waveAutoCorr.count; i++)
    {
        double d = waveAutoCorr.data[(waveAutoCorr.start + i) % DWAVE_BUFSIZE];
        while (d > up || d < down)
        {
            up *= 1.2;
            down *= 1.2;
        }
    }

    for (int i = 0; i < waveStrSum.count; i++)
    {
        double d = waveStrSum.data[(waveStrSum.start + i) % DWAVE_BUFSIZE];
        while (d > up || d < down)
        {
            up *= 1.2;
            down *= 1.2;
        }
    }

    waveSetBound(waveAutoCorr, up, down);
    waveSetBound(waveStrSum, up, down);

    int x = AUTOCORR_X;
    int y = AUTOCORR_Y;
    int h = AUTOCORR_H;
    int w = AUTOCORR_W;
    int m = getBorderMargin();
    waveAutoCorr.color = CS(COLOR_CORR);
    waveStrSum.color = CS(COLOR_ENERGY);
    pntWave(hdc, waveAutoCorr, x + m, y + m, h - 2 * m, w - 2 * m);
    pntWave(hdc, waveStrSum, x + m, y + m, h - 2 * m, w - 2 * m);
    pntBorder(hdc, x, y, h, w, "Average energy and Auto correlation");
}

static void
pushCarrierSensor(Complex & c)
{
    double str = c.re * c.re + c.im * c.im;
    double corr = c.re * acorrHistory[acorrHisIndex].re 
        + c.im * acorrHistory[acorrHisIndex].im;
    strSum += str;
    corrSum += corr;

    acorrHistory[acorrHisIndex] = c;
    acorrHisIndex++;
    acorrHisIndex %= AUTOCORR_STEP;
 
    nCount++;
    if (nCount >= AVG_STEP)
    {
        strSum /= AVG_STEP;
        corrSum /= AVG_STEP;

        pushCorrelator(corrSum, strSum);

        // dprintf("%f %f", strSum, corrSum);

		if (  corrSum > strSum *(4.0 / 9.0) && corrSum > THRESHOLD)
//		if ( strSum > THRESHOLD * 2 )
             posCount++; 
        else
            posCount = 0;

        if (posCount >= 2 && !carrierSensing)
        {
            carrierSensing = true;
            waveMark(waveAutoCorr);
            counter++;
        }

        strSum = 0;
        corrSum = 0;
        nCount = 0;
   }
}


static Complex SHORT_PRE[16] = { 
    {   753,    -753     },
    {   -2162,  -32      },
    {   -212,   1294   },
    {   2342,   212    },
    {   1507,   0       },
    {   2342,   212    },
    {   -212,   1294   },
    {   -2162,  -32      },
    {   753,    -753     },
    {   32,     2162   },
    {   -1294,  212    },
    {   -212,   -2342    },
    {   0,      -1507    },
    {   -212,   -2342    },
    {   -1294,  212    },
    {   32,     2162   },
};

/*
static Complex SHORT_PRE[16] = { 
    {   0,      -1507    },
    {   -212,   -2342    },
    {   -1294,  212    },
    {   32,     2162   },

    {   753,    -753     },
    {   -2162,  -32      },
    {   -212,   1294   },
    {   2342,   212    },
    {   1507,   0       },
    {   2342,   212    },
    {   -212,   1294   },
    {   -2162,  -32      },
    {   753,    -753     },
    {   32,     2162   },
    {   -1294,  212    },
    {   -212,   -2342    },

};
*/
static bool
pushCorrelator(Complex & c, double & str)
{
    wavePush(waveEnergy, c.re * c.re + c.im * c.im);

    Complex t = {0, 0};
    Complex sum = {0, 0};
    corrHistory[corrHisIndex] = c;
    for (int i = 0; i < 16; i++)
    {
        cmul(t, corrHistory[(corrHisIndex + i) % CORR_STEP], SHORT_PRE[i]);
        cadd(sum, t);
    }
    cdiv(sum, 16);

    str = (sum.re * sum.re + sum.im * sum.im) / 1000000;
    corrReHistory[corrHisIndex] = str;
//    dprintf("%f", str);
    wavePush(waveCorr, str);

    corrHisIndex = (corrHisIndex + 1) % CORR_STEP;

    double hisSum = 0;
    for (int i = 0; i < 16; i++)
    {
        if (str < corrReHistory[i] * 0.9999)
            return false;
        hisSum += corrReHistory[i];
    }

    if ((hisSum / 16 * 1.3) > str)
        return false;
    return true;
}

static int lastSyncedDistance = 0;
static int peek16Count = 0;
static int collectorCount = -1;
static const int COMPLEX_PER_SYMBOL = 80;
static Complex oneSymbol[COMPLEX_PER_SYMBOL];
static double lastStr;
int LT_boundary = -1;
 //extern int getSampleCounter ();

static void
pushSymbolCollector(Complex & c)
{
    double str;
    bool isBoundary = pushCorrelator(c, str);
    bool isPeak = false;
    
    lastSyncedDistance++;
    if (isBoundary)
    {

//printf ( "found boundary (%d) CS:%d cnt:%d syndis:%d\n",
//	      getSampleCounter (), carrierSensing, peek16Count, lastSyncedDistance);

        if (lastSyncedDistance == 16 && str >= lastStr / 2)
        {
            peek16Count++;
            isPeak = true;
        }
        else
            peek16Count = 0;
            
        lastStr = str;
        lastSyncedDistance = 0;
    }
//if ( getSampleCounter () > 160 )
//	printf ( "found boundary (%d) CS:%d cnt:%d syndis:%d\n",
//			  getSampleCounter (), carrierSensing, peek16Count, lastSyncedDistance);
    
    if (!isPeak && peek16Count > 6 && lastSyncedDistance == 16
            && carrierSensing)
    {
     	//
    	// this is ad hoc - 
    	// So we come to the conclusion that 
    	// short-training symbols finish
    	//
  	
        setEqualizerIndicator(16);
  //      setEqualizerIndicator(12);
        collectorCount = -1;
        waveMarkClear(waveCorr);
        waveMarkHis(waveCorr, 16);
        waveMarkHis(waveCorr, 16 + 160);
        peek16Count = 0;
        lastStr = 0;

        LT_boundary = getSampleCounter ();
		printf ( "Frame detected at sample %d\n", LT_boundary );
		
    }

    if (collectorCount >= 0)
    {
        oneSymbol[collectorCount] = c;
        collectorCount++;
        // dprintf("%d", collectorCount);
        if (collectorCount == COMPLEX_PER_SYMBOL)
        {
            collectorCount = 0;
            waveMark(waveCorr);
            pushSymbol(oneSymbol);
        }
    }
    else
    {
        clearSymbol();
    }

    if (pushEqualizer(c))
    {
        if (collectorCount < 0)
        {
            collectorCount = 0;
            waveMark(waveCorr);
            resetPilotCounter();
        }
    }
}

static void
clearSymbolBoundary()
{
    collectorCount = -1;
    clearSymbol();
}

void
pntCorrelator(HDC hdc)
{
    int m = getBorderMargin();

    double up = 2000000;
    double down = -600000;
    for (int i = 0; i < waveCorr.count; i++)
    {
        double d = waveCorr.data[(waveCorr.start + i) % DWAVE_BUFSIZE];
        while (d > up || d < down)
        {
            up *= 1.2;
            down *= 1.2;
        }
    }

    
    for (int i = 0; i < waveEnergy.count; i++)
    {
        double d = waveEnergy.data[(waveEnergy.start + i) % DWAVE_BUFSIZE];
        while (d > up * 2 || d < down * 2)
        {
            up *= 1.2;
            down *= 1.2;
        }
    }

    waveSetBound(waveCorr, up, down);
    waveSetBound(waveEnergy, up * 2, down * 2);  

    waveCorr.color = CS(COLOR_CORR);
    waveEnergy.color = CS(COLOR_ENERGY);
    pntWave(hdc, waveCorr, CORR_X + m, CORR_Y + m, CORR_H - 2 * m, CORR_W - 2 * m);
    pntWave(hdc, waveEnergy, CORR_X + m, CORR_Y + m, CORR_H - 2 * m, CORR_W - 2 * m);
    pntBorder(hdc, CORR_X, CORR_Y, CORR_H, CORR_W, "Energy and correlation");
}


void 
pushRemoveDC (Complex & c)
{
   double e;
   cmod (e, c); 

   if ( e < 5000 ) {

   }
}

void
pushSynchronizer(Complex & c)
{
    pushCarrierSensor(c);

    pushSymbolCollector(c);
}

void clearConstHisInfo ();
void
clearReceiverState()
{
    clearSymbolBoundary();
    carrierSensing = false;
    
    // kun: clear hisinfo
    clearConstHisInfo ();
    
}

void 
resetSynchronizer()
{
    resetCorrelator();
    resetSymbol();
    clearReceiverState();
}

void
synchronizerInit()
{
    waveSetBound(waveAutoCorr, 15000000, -5000000);
    waveSetBound(waveStrSum, 15000000, -5000000);
    waveSetBound(waveCorr, 12000000, -2000000);
    waveSetBound(waveEnergy, 2 * 12000000, - 2 * 2000000);

    resetCorrelator();
}
