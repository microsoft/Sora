/*******************************************************
	Intalg - a collection of integer-based algorithm
********************************************************/

#pragma once

//
// We support Fixed Point Radians in this file.
// FP_RAD maps PI (and -PI) to 0x8000 and it represents only radians [-PI, PI)
//
typedef short FP_RAD;  // fixed point radians

// FP_FRAC fixed point fraction - [-1,1)
//
typedef short FP_FRAC; // fixed point fraction

#define FP_PI  0x8000
#define FP_2PI (65536)
#define FP_ONE (32767)


#include "intalglut.h"

#ifndef PI
#define PI (3.141592654)
#endif

FINL 
FP_FRAC usin ( FP_RAD r ) {
	return usin_lut[(ushort)r];
}

FINL 
FP_FRAC ucos ( FP_RAD r ) {
	return ucos_lut[(ushort)r];
}


//
// conversion
FINL
double fprad2rad ( FP_RAD r ) {
	return (PI * r / FP_PI);
}

FINL
FP_RAD rad2fprad ( double r ) {
	return (FP_RAD) (r * FP_PI / PI);
}

// how many bits are set (one) in a char
FINL 
uchar bit_cnt (uchar b ) {
	return bit_set_lut[b];
}

// bit_scope - find the highest bit position of an integer
FINL
uchar _bit_scope_ub (uchar x) {
	return bit_high_pos_lut[x];
}

FINL
uchar _bit_scope_us (ushort x) {
	uchar tt;
	if ( tt = (x >> 8) ) {
		return _bit_scope_ub (tt) + 8;
	} else {
		return _bit_scope_ub ((uchar)(x));
	}	
}

FINL
uchar _bit_scope_ui (uint x) {
	ushort tt;
	if (( tt = (x >> 16) )) {
		return _bit_scope_us (tt) + 16;
	} else {
		return _bit_scope_us ((ushort)(x));
	}	
}


FINL
uchar bit_scope (int x) {
	if ( x>0 ) {
		// positive value
		return _bit_scope_ui ((uint)x);		
	} else {
		// negative value
		return _bit_scope_ui ((uint)(-x));		
	}
}

FINL
FP_FRAC uatan2 ( int y, int x ) {
	int ys = bit_scope (y);
	int xs = bit_scope (x); 
	
	int shift = ((xs>ys)?xs:ys) - 6;
	
	if ( shift > 0 ) {
		return uatan2_lut[(uchar)(y>>shift)][(uchar)(x>>shift)];
	}	
	else 
		return uatan2_lut[(uchar)(y)][(uchar)(x)];
}

