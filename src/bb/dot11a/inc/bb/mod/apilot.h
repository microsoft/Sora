#ifndef BB_MOD_PILOT_H
#define BB_MOD_PILOT_H

#include "lut.h"
#include <math.h>

// Hint
//   Correct the phase using pilot
//
#undef PI
#define PI 3.141592653

#define INT_PI 0x8000
#define TRACK_THRESHOLD 3277 // PI * (3277) / 32767 

//
// Adataptive channel tracking -  
// We slow the tracking down to reduce the precision requirements.
//

__forceinline
void Pilot(IN OUT COMPLEX16 pc[64], 
           IN OUT COMPLEX32 * ChannelFactor,
           char * pbPilotCounter)
{
    short th1 = 
        *(ARG( ((((unsigned short)(pc[64-21].re)) << 5) & 0xFF00)
        ^ ((unsigned char)(pc[64-21].im >> 3)) ));
    short th2 =
        *(ARG( ((((unsigned short)(pc[64-7].re)) << 5) & 0xFF00)
        ^ ((unsigned char)(pc[64-7].im >> 3)) ));
    short th3 = 
        *(ARG( ((((unsigned short)(pc[7].re)) << 5) & 0xFF00)
        ^ ((unsigned char)(pc[7].im >> 3)) ));
    short th4 =
        (*(ARG( ((((unsigned short)(pc[21].re)) << 5) & 0xFF00)
        ^ ((unsigned char)(pc[21].im >> 3)) ))) + INT_PI;

	
	    _dump_text ( "Pilots: <%d,%d> %d <%d,%d> %d <%d,%d> %d <%d,%d> %d \n\n", 
                pc[64-21].re, pc[64-21].im, th1, 
                pc[64-7].re, pc[64-7].im, th2, 
                pc[7].re,    pc[7].im, th3, 
                pc[21].re,   pc[21].im, th4 );

	// Debug
	/*
	printf ( "th1 %d th2 %d th3 %d th4 %d\n", 
				  th1 ,
				  th2 ,
				  th3 ,
				  th4  ); 

	printf ( "energy p1 %d p2 %d p3 %d p4 %d\n", 
			 pc[64-21].re * pc[64-21].re + pc[64-21].im * pc[64-21].im,
			 pc[64-7].re * pc[64-7].re + pc[64-7].im * pc[64-7].im,
			 pc[7].re * pc[7].re + pc[7].im * pc[7].im,
			 pc[21].re * pc[21].re + pc[21].im * pc[21].im ); 	

	printf ( "energy ch1 %d ch2 %d ch3 %d ch4 %d\n", 
			 ChannelFactor[64-21].re * ChannelFactor[64-21].re 
			 	+ ChannelFactor[64-21].im * ChannelFactor[64-21].im,
			 ChannelFactor[64-7].re * ChannelFactor[64-7].re 
			 	+ ChannelFactor[64-7].im * ChannelFactor[64-7].im,
			 ChannelFactor[7].re * ChannelFactor[7].re 
			 	+ ChannelFactor[7].im * ChannelFactor[7].im,
			 ChannelFactor[21].re * ChannelFactor[21].re 
			 	+ ChannelFactor[21].im * ChannelFactor[21].im ); 	
	*/
	
	//
	// subcarrier rotation = const_rotate + i * delta_rotate
    // estimate the const part
    //
    // N.B. This is a compiler issue that I don't know how to work around
    // SHORT is presented as 32-bit integer inside
    //
	if (*PILOTSGN(*pbPilotCounter)) {
			th1 += INT_PI;
			th2 += INT_PI;
			th3 += INT_PI;
			th4 += INT_PI;
	}
    if ((++(*pbPilotCounter)) == 127)
	   *pbPilotCounter = 0;

    // subcarrier rotation = const_rotate + i * delta_rotate
    // estimate the const part
	short avgTheta = (th1 + th2 + th3 + th4) / 4;

    // estimate the delta part
    short delTheta = ((((short)(th3 - th1)) / (21 + 7) 
            + ((short)(th4 - th2)) / (21 + 7)) >> 1);

	// Debug
	/*
	printf ( "ave %lf delta %lf \n\n", 
			  avgTheta * PI / 0x7FFF,
			  delTheta * PI / 0x7FFF );	
	*/
    short th = avgTheta - delTheta * 26;
    int i;
    int s, c;
    short re, im;

   	int ire, iim;

	int bTrack = (avgTheta > TRACK_THRESHOLD || 
		          avgTheta < - TRACK_THRESHOLD);
	
    // subcarrier -26 - -1
    for (i = 64-26; i < 64; i++)
    {
        s = *(SIN0xFFFF((unsigned short)(th)));
        c = *(COS0xFFFF((unsigned short)(th)));
        
        re = (short)((pc[i].re * c + pc[i].im * s) >> 16);
        im = (short)((pc[i].im * c - pc[i].re * s) >> 16);
        
        pc[i].re = re;
        pc[i].im = im;

		// Track the channel rotation
		// adaptive track
		if ( bTrack ) {
	        ire = (short)((ChannelFactor[i].re * c 
							+ ChannelFactor[i].im * s)>>15);
		    iim = (short)((ChannelFactor[i].im * c 
							- ChannelFactor[i].re * s)>>15);
			ChannelFactor[i].re = ire;
			ChannelFactor[i].im = iim;
		}

		// Debug
		/*
		if ( i == (64-7) || i == (64-21) || i == (7) || i == (21) ) {

			ffc = 1.0 * c / 32767;
			ffs = 1.0 * s / 32767;
			printf ( "ch %d <%lf, %lf> <%lf, %lf> %lf\n <%d, %d>\n", 
					  i, 
					  fc, fs,
					  ffc, ffs,
					  ffc*ffc + ffs*ffs,
					  ire, iim );
		}
		*/

        th += delTheta;
    }

    th += delTheta; // 0 subcarrier, dc

    // subcarrier 1 - 26
    for (i = 1; i <= 26; i++)
    {

        s = *(SIN0xFFFF((unsigned short)(th)));
        c = *(COS0xFFFF((unsigned short)(th)));

        re = (short)((pc[i].re * c + pc[i].im * s) >> 16);
        im = (short)((pc[i].im * c - pc[i].re * s) >> 16);
        pc[i].re = re;
        pc[i].im = im;

		// Track the channel rotation
		if ( bTrack ) {
	        ire = (short)((ChannelFactor[i].re * c 
							+ ChannelFactor[i].im * s)>>15);
		    iim = (short)((ChannelFactor[i].im * c 
							- ChannelFactor[i].re * s)>>15);
			ChannelFactor[i].re = ire;
			ChannelFactor[i].im = iim;
		}

		// Debug
		/*
		if ( i == (64-7) || i == (64-21) || i == (7) || i == (21) ) {

			ffc = 1.0 * c / 32767;
			ffs = 1.0 * s / 32767;
			printf ( "ch %d <%lf, %lf> <%lf, %lf> %lf\n <%d, %d>\n", 
					  i, 
					  fc, fs,
					  ffc, ffs,
					  ffc*ffc + ffs*ffs,
					  ire, iim );
		}
		*/
        th += delTheta;
    }
	
}

#endif//BB_MOD_PILOT_H
