#include <stdio.h>
#include "dsplib.h"

#define MAX_SHORT 32767

// any point FFT
static const int M1 = 0x21, M2 = 0x12, M3 = 0x0C;
// note: the bit reordering only for 64 and less...
void FFT  ( int Size, COMPLEX * pInput, COMPLEX * pOutput )
{
	for (int i=0; i<Size; i++)
	{
		int j = i;
		if (((i & M1) ^ M1) && (i & M1))
			j ^= M1;
		if (((i & M2) ^ M2) && (i & M2))
			j ^= M2;
		if (((i & M3) ^ M3) && (i & M3))
			j ^= M3;
		pOutput[j] = pInput[i];
	}
	
	int deltatheta = (Size >> 1);
		
	for (int i=1; i<Size; i<<=1)
	{
		int t = i << 1;
		for (int j=0; j<Size; j+=t)
		{
			int theta = 0;
			for (int k=0; k<i; k++)
			{
				int e1 = j + k;
				int e2 = j + k + i;
				COMPLEX w;
				w.re = cos(2 * PI * theta / Size);
				w.im = -sin(2 * PI * theta / Size);
				
				COMPLEX temp = pOutput[e2] * w;
				pOutput[e2] = pOutput[e1] - temp;
				pOutput[e1] = pOutput[e1] + temp;
				
				theta += deltatheta;
			}
		}
		deltatheta >>= 1;
	}
}



// any point DFT - Slow version
void DFT ( int Size, COMPLEX * pInput, COMPLEX * pOutput ) {
	double dfm = - 2*PI / Size;
	
	for ( int i=0; i<Size; i++) {
		// y[i] = \sum_0_{M-1} x[i] * dfm (i, k );
		COMPLEX c, w, y;
		c.clear ();
		
		for (int j=0; j<Size; j++) {
			w.re = cos ( dfm * j * i ); w.im = sin (dfm * j * i );
			y = w * pInput[j];
			c = c + y;
		}
		pOutput [i] = c;
	}
}

void iDFT ( int Size, COMPLEX * pInput, COMPLEX * pOutput ) {
	double dfm = 2*PI / Size;
	for ( int i=0; i<Size; i++) {
		// y[i] = \sum_0_{M-1} x[i] * dfm (i, k );
		COMPLEX c, w, y;
		c.clear ();
		
		for (int j=0; j<Size; j++) {
			w.re = cos ( dfm * j * i ); w.im = sin ( dfm * j * i );
			y = w * pInput[j];
			c = c + y ;
		}
		pOutput [i] = c / Size ;
	}
}

// Dump file operations

int LoadDumpFile(const char *fileName, COMPLEX* buf, int maxSize)
{
	COMPLEX16 blk[28];

    //FILE *fin = fopen(fileName, "rb");
    //if (!fin) return -1;

	FILE * fin;
	errno_t ret = fopen_s(&fin, fileName, "rb");
	if (ret)
		return -1;

	COMPLEX* pBuf = buf;
	int count = 0;	
	while (!feof(fin) ) {
		fseek ( fin, 16, SEEK_CUR );

		fread ( blk, sizeof(COMPLEX16), 28, fin);
		for ( int i=0; i<28; i++) {
			pBuf [i] = blk[i];
		}
		pBuf += 28;
		count += 28;
	}

    fclose(fin);
    return count;
}


void Normalize ( COMPLEX * buf, int Size ) {
	double mod_max = 0;
	for ( int i=0; i < Size; i++) {
		if ( buf[i].norm () > mod_max ) 
			mod_max = buf[i].norm ();
	}

	if ( mod_max == 0 ) return;
	double factor = MAX_SHORT / sqrt(mod_max);

	for ( int i=0; i < Size; i++) {
		buf[i].scale (factor);
	}
}

static char hdr[16] = { 0x1, 0, 7, 0,  0, 0, 0, 0, 
	             0, 0, 0, 0,  0, 0, 0, 0};

void SaveDumpFile ( const char *filename, COMPLEX* buf, int Size ) {
	Normalize ( buf, Size );

	COMPLEX16 blk[28];
	
    FILE *fout;// = fopen(filename, "wb");

	errno_t ret = fopen_s(&fout, filename, "wb");
    //if (!fout) return;
	if (ret != 0)
		return;

	COMPLEX* pBuf = buf;

	while ( Size >= 28) {
		fwrite ( hdr, sizeof(char), 16, fout); // header

		for (int i=0; i < 28; i++) {
			blk[i].re = (short)pBuf[i].re;
			blk[i].im = (short)pBuf[i].im;			
		}
		
		fwrite ( blk, sizeof(COMPLEX16), 28, fout);
		
		pBuf += 28;
		Size -= 28;
	}

    fclose(fout);
}
