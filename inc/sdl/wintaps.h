#pragma once

#define TYPE_LOWPASS  0
#define TYPE_BANDPASS 1
#define TYPE_HIGHPASS 2
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
	case TYPE_BANDPASS:
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
	case TYPE_HIGHPASS:
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

