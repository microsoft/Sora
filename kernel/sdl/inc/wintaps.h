#pragma once

#include "Bessel_lut.h" 

#define TYPE_LOWPASS  0
#define TYPE_HIGHPASS 1
#define TYPE_BANDPASS 2
#define TYPE_BANDSTOP 3

static const int power_level = 20000;

//the hanning window
FINL bool HanCompTap(
						uint  Filter_type,
						uint* Cutoff_freq, // The second element is just for BPF&BSF: [0]=Wl,[1]=Wh
						uint  Sample_rate, // The unit is MHz
						uint Tap_num,                       
						COMPLEX16* Taps    //The computed taps
    )
{    
	uint Dream = Filter_type;
	uint fc1=*Cutoff_freq;
	double Wc1=2*PI*fc1/Sample_rate;
	uint half = (Tap_num-1)>>1;
	switch(Dream){
 	case TYPE_LOWPASS:                          //Low Pass Filter
	{	
		for(uint n=0; n<Tap_num; n++)
		{
			double h;
			if(n==half) h = 0.5*Wc1/PI*(1-cos(PI*n/half));
			else if(n<half) h = 0.5*sin(Wc1*(half-n))*(1-cos(PI*n/half))/PI/(half-n);
			else h = 0.5*sin(Wc1*(n-half))*(1-cos(PI*n/half))/PI/(n-half);
			short hn=(short)(power_level*h);
			Taps[n].re = Taps[n].im = hn;
		}
	}
	break;
	case TYPE_HIGHPASS:
	{
		for(uint n=0; n<Tap_num; n++)
		{
			double h;
			if(n==half) h = 0.5*(1-Wc1/PI)*(1-cos(PI*n/half));
			else if(n<half) h = 0.5*(sin(PI*(half-n))-sin(Wc1*(half-n)))*(1-cos(PI*n/half))/PI/(half-n);
			else h = 0.5*(sin(PI*(n-half))-sin(Wc1*(n-half)))*(1-cos(PI*n/half))/PI/(n-half);
			short hn=(short)(power_level*h);
			Taps[n].re = Taps[n].im = hn;
		}
	}
	break;
	case TYPE_BANDPASS:
	{    
		uint fc2=*(++Cutoff_freq);
		double Wc2=2*PI*fc2/Sample_rate; 
		for(uint n=0; n<Tap_num; n++)
		{
			double h;
			if(n==half) h = 0.5*(Wc2-Wc1)/PI*(1-cos(PI*n/half));
			else if(n<half) h = 0.5*(sin(Wc2*(half-n))-sin(Wc1*(half-n)))*(1-cos(PI*n/half))/PI/(half-n);
			else h = 0.5*(sin(Wc2*(n-half))-sin(Wc1*(n-half)))*(1-cos(PI*n/half))/PI/(n-half);
			short hn=(short)(power_level*h);
			Taps[n].re = Taps[n].im = hn;
		}
	}
	break;
	case TYPE_BANDSTOP: 
	{
		uint fc2=*(++Cutoff_freq);
		double Wc2=2*PI*fc2/Sample_rate; 
		for(uint n=0; n<Tap_num; n++)
		{
			double h;
			if(n==half) h = 0.5*(1-Wc2/PI+Wc1/PI)*(1-cos(PI*n/half));
			else if(n<half) h = 0.5*(sin(PI*(half-n))-sin(Wc2*(half-n))+sin(Wc1*(half-n)))*(1-cos(PI*n/half))/PI/(half-n);
			else h = 0.5*(sin(PI*(n-half))-sin(Wc2*(n-half))+sin(Wc1*(n-half)))*(1-cos(PI*n/half))/PI/(n-half);
			short hn=(short)(power_level*h);
			Taps[n].re = Taps[n].im = hn;
		}
	}
	break;
	default: 
		break;
	}
	
	return true;
}


//the kaiser window
FINL bool KSCompTap(
						uint  Filter_type,
						uint* Cutoff_freq, // The second element is just for BPF&BSF: [0]=Wl,[1]=Wh
						uint  Sample_rate, // The unit is MHz
						uint Tap_num,                       
						COMPLEX16* Taps,    //The computed taps: 7, 11, 15, 19, 23, 27, 31, 35
						int beta            //beta: 0,1,2,3,4,5,6,7
    )
{    
	uint Dream = Filter_type;
	uint fc1=*Cutoff_freq;
	double Wc1=2*PI*fc1/Sample_rate;
	uint half = (Tap_num-1)>>1;
	uint index = (Tap_num-7)/4*8+beta;

	

	switch(Dream){
 	case TYPE_LOWPASS:                          //Low Pass Filter
	{	
		for(uint n=0; n<Tap_num; n++)
		{
			double h;
			double wn;
			wn = Bessel_lut2[index][n]/Bessel_lut1[beta];
			if(n==half) h = wn*Wc1/PI;
			else if(n<half) h = wn*sin(Wc1*(half-n))/PI/(half-n);
			else h = wn*sin(Wc1*(n-half))/PI/(n-half);
			short hn=(short)(power_level*h);
			Taps[n].re = Taps[n].im = hn;
		}
	}
	break;
	case TYPE_HIGHPASS:
	{
		for(uint n=0; n<Tap_num; n++)
		{
			double h;
			double wn;
			wn = Bessel_lut2[index][n]/Bessel_lut1[beta];
			if(n==half) h = wn*(1-Wc1/PI);
			else if(n<half) h = wn*(sin(PI*(half-n))-sin(Wc1*(half-n)))/PI/(half-n);
			else h = wn*(sin(PI*(n-half))-sin(Wc1*(n-half)))/PI/(n-half);
			short hn=(short)(power_level*h);
			Taps[n].re = Taps[n].im = hn;
		}
	}
	break;
	case TYPE_BANDPASS:
	{    
		uint fc2=*(++Cutoff_freq);
		double Wc2=2*PI*fc2/Sample_rate; 
		for(uint n=0; n<Tap_num; n++)
		{
			double h;
			double wn;
			wn = Bessel_lut2[index][n]/Bessel_lut1[beta];
			if(n==half) h = wn*(Wc2-Wc1)/PI;
			else if(n<half) h = wn*(sin(Wc2*(half-n))-sin(Wc1*(half-n)))/PI/(half-n);
			else h = wn*(sin(Wc2*(n-half))-sin(Wc1*(n-half)))/PI/(n-half);
			short hn=(short)(power_level*h);
			Taps[n].re = Taps[n].im = hn;
		}
	}
	break;
	case TYPE_BANDSTOP: 
	{
		uint fc2=*(++Cutoff_freq);
		double Wc2=2*PI*fc2/Sample_rate;
		for(uint n=0; n<Tap_num; n++)
		{
			double h;
			double wn;
			wn = Bessel_lut2[index][n]/Bessel_lut1[beta];
			if(n==half) h = wn*(1-Wc2/PI+Wc1/PI);
			else if(n<half) h = wn*(sin(PI*(half-n))-sin(Wc2*(half-n))+sin(Wc1*(half-n)))/PI/(half-n);
			else h = wn*(sin(PI*(n-half))-sin(Wc2*(n-half))+sin(Wc1*(n-half)))/PI/(n-half);
			short hn=(short)(power_level*h);
			Taps[n].re = Taps[n].im = hn;
		}
	}
	break;
	default: 
		break;
	}
	
	return true;
}