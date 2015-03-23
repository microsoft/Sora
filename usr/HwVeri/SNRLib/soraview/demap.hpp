#pragma once

#define MAP_MOD_MAX 10000


extern unsigned char g11a_rgbDemapBPSK[256];
extern unsigned char g11a_rgbDemap16QAM2[256];

FINL void Saturate ( COMPLEX& s ) {
	if ( s.re > MAP_MOD_MAX ) 
		s.re = MAP_MOD_MAX;
	else if ( s.re < - MAP_MOD_MAX )  
		s.re = - MAP_MOD_MAX;

	if ( s.im > MAP_MOD_MAX ) 
		s.im = MAP_MOD_MAX;
	else if ( s.im < - MAP_MOD_MAX )  
		s.im = - MAP_MOD_MAX;
}

FINL void BPSKDemap ( COMPLEX* pSample, unsigned char * pSBits )
{
	int i, jj;
	
	for (i = 64 - 26; i < 64; i++)
	{
		if (i == 64 - 21 || i == 64 - 7)
			continue;

		Saturate ( pSample[i] );		
		jj = (unsigned char) (pSample[i].re * 127 / MAP_MOD_MAX);
		pSBits[0] = g11a_rgbDemapBPSK[jj];
		pSBits++;
	}

	for (i = 1; i <= 26; i++)
	{
		if (i == 7 || i == 21)
			continue;

		Saturate ( pSample[i] );		
		jj = (unsigned char) (pSample[i].re * 127 / MAP_MOD_MAX);
		
		pSBits[0] = g11a_rgbDemapBPSK[jj];
		pSBits++;
	} 
}

static int DemapSpot(unsigned char * pbOutput)
{
    int spot = 0;

    switch(pbOutput[0])
    {
        case 0 :
            spot += 0;
            break;
        case 1 :
            spot += 0;
            break;
        case 2 :
            spot += 0;
            break;
        case 3 :
            spot += 0;
            break;
        case 4 :
            spot += 8;
            break;
        case 5 :
            spot += 8;
            break;
        case 6 :
            spot += 8;
            break;
        case 7 :
            spot += 8;
            break;
        default :
            spot = -1;
            return (spot);
    }		

    switch(pbOutput[1])
    {
        case 0 :
            spot += 0;
            break;
        case 1 :
            spot += 0;
            break;
        case 2 :
            spot += 0;
            break;
        case 3 :
            spot += 4;
            break;
        case 4 :
            spot += 4;
            break;
        case 5 :
            spot += 4;
            break;
        case 6 :
            spot += 4;
            break;
        case 7 :
            spot += 4;
            break;
        default :
            spot = -1;
            return (spot);
    }		

    switch(pbOutput[2])
    {
        case 0 :
            spot += 0;
            break;
        case 1 :
            spot += 0;
            break;
        case 2 :
            spot += 0;
            break;
        case 3 :
            spot += 0;
            break;
        case 4 :
            spot += 2;
            break;
        case 5 :
            spot += 2;
            break;
        case 6 :
            spot += 2;
            break;
        case 7 :
            spot += 2;
            break;
        default :
            spot = -1;
            return (spot);
    }		

    switch(pbOutput[3])
    {
        case 0 :
            spot += 0;
            break;
        case 1 :
            spot += 0;
            break;
        case 2 :
            spot += 0;
            break;
        case 3 :
            spot += 1;
            break;
        case 4 :
            spot += 1;
            break;
        case 5 :
            spot += 1;
            break;
        case 6 :
            spot += 1;
            break;
        case 7 :
            spot += 1;
            break;
        default :
            spot = -1;
            return (spot);
    }		

    return (spot);
}

static void 
demap16QAM_11a(RX_SAMPLE & samples, COMPLEX * pc, unsigned char * pbOutput)
{
    int i;

	int spot;
	
    for (i = 64 - 26; i < 64; i++)
    {
        if (i == 64 - 21 || i == 64 - 7)
            continue;

		COMPLEX sampleBeforeSaturate = pc[i];
		Saturate ( sampleBeforeSaturate );

		unsigned char nRe = (unsigned char) (sampleBeforeSaturate.re * 127 / MAP_MOD_MAX);
		unsigned char nIm = (unsigned char) (sampleBeforeSaturate.im * 127 / MAP_MOD_MAX);	

		Printf("{%d %d} ", nRe, nIm);
		
        pbOutput[0] = g11a_rgbDemapBPSK[nRe];
        pbOutput[1] = g11a_rgbDemap16QAM2[nRe];
        pbOutput[2] = g11a_rgbDemapBPSK[nIm];
        pbOutput[3] = g11a_rgbDemap16QAM2[nIm];
        pbOutput += 4;

		spot = DemapSpot(pbOutput-4);

		if (!samples.IsFull())
			//samples.InsertSample(spot, pc[i]);
			samples.InsertSample(g_spotTable[samples.sampleNum], pc[i]);
		
//		Printf("spot: %d\n", spot);
		
    }


    for (i = 1; i <= 26; i++)
    {
        if (i == 7 || i == 21)
            continue;
		
		COMPLEX sampleBeforeSaturate = pc[i];
		Saturate ( sampleBeforeSaturate );

		unsigned char nRe = (unsigned char) (sampleBeforeSaturate.re * 127 / MAP_MOD_MAX);
		unsigned char nIm = (unsigned char) (sampleBeforeSaturate.im * 127 / MAP_MOD_MAX);		

		Printf("{%d %d} ", nRe, nIm);
		
        pbOutput[0] = g11a_rgbDemapBPSK[nRe];
        pbOutput[1] = g11a_rgbDemap16QAM2[nRe];
        pbOutput[2] = g11a_rgbDemapBPSK[nIm];
        pbOutput[3] = g11a_rgbDemap16QAM2[nIm];
        pbOutput += 4;


		spot = DemapSpot(pbOutput-4);
		if (!samples.IsFull())		
			samples.InsertSample(spot, pc[i]);
		
//		Printf("spot: %d\n", spot);		
    }
}
