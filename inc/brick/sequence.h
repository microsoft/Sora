#pragma once

static const int ONE_MOD = 10000;

template<int FFT_N>
bool Generate80211aSTS ( COMPLEX16* pbuf ) {
	if ( (FFT_N % 64 == 0) // FFT_N must be a multiple of 64
	     && !(IsPower2<FFT_N>::IsFalse) // FFT_N must be a power of 2
	)
	{
		A16 COMPLEX16 temp[FFT_N];
		memset (temp, 0, sizeof(temp));

		temp[4].re	= temp[4].im  = - ONE_MOD;
		temp[8].re	= temp[8].im  = - ONE_MOD;
		temp[12].re = temp[12].im =   ONE_MOD;
		temp[16].re = temp[16].im =   ONE_MOD;
		temp[20].re = temp[20].im =   ONE_MOD;
		temp[24].re = temp[24].im =   ONE_MOD;
		
		temp[FFT_N-24].re = temp[FFT_N-24].im =   ONE_MOD;
		temp[FFT_N-20].re = temp[FFT_N-20].im = - ONE_MOD;
		temp[FFT_N-16].re = temp[FFT_N-16].im =   ONE_MOD;
		temp[FFT_N-12].re = temp[FFT_N-12].im = - ONE_MOD;
		temp[FFT_N-8].re  = temp[FFT_N-8].im =  - ONE_MOD;
		temp[FFT_N-4].re  = temp[FFT_N-4].im =    ONE_MOD;

		IFFT<FFT_N>((vcs*)temp, (vcs*)pbuf);
		return true;
	}
	else return false;
}

