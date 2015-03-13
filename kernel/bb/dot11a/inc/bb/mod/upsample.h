#pragma once
#include <assert.h>
#include "complex.h"
#include "vector128.h"

// Maximal signed value of short integer
// Note: cannot use 0x8000, which is the minimal negative short integer
#define SONE (0x7fff)

// Macro to represent interpolating paramters
// Note: 0 <= x <= 10
#define S1(x) (short((x) * SONE / 11))

__declspec(selectany) extern const
vs::data_type g_coff1_row0 [] = {
    {SONE,  SONE,  S1(2),  S1(2),  S1(9), S1(9), S1(4), S1(4)},
    {S1(7), S1(7), S1(6),  S1(6),  S1(5), S1(5), S1(8), S1(8)},
    {S1(3), S1(3), S1(10), S1(10), S1(1), S1(1), 0,     0}
};

__declspec(selectany) extern const
vs::data_type g_coff1_row1 [] = {
    {S1(1), S1(1), S1(10), S1(10), S1(3), S1(3), S1(8), S1(8)},
    {S1(5), S1(5), S1(6),  S1(6),  S1(7), S1(7), S1(4), S1(4)}, 
    {S1(9), S1(9), S1(2),  S1(2),  0,     0,     0,     0} 
};

__declspec(selectany) extern const
vcs::data_type _ODD_MASK = {
    __int16(-1),    __int16(-1),  
	__int16(0x0),   __int16(0x0),
    __int16(-1),    __int16(-1),  
	__int16(0x0),   __int16(0x0),
};

__declspec(selectany) extern const
vcs::data_type _EVEN_MASK = {
	__int16(0x0),   __int16(0x0),
    __int16(-1),    __int16(-1),
	__int16(0x0),   __int16(0x0),
    __int16(-1),    __int16(-1),
};

DSP_INLINE static void compute_4 ( vcs & r, vs & vin, vcs & resi,
	                        const vs * c_r0, const vs * c_r1, 
	                        int c_idx )
{
	vcs xx1, xx2, xx3, yy1, yy2;
	
    vs vins = vin;
	xx1 = (vcs) mul_shift<15> ( vins, c_r0[c_idx] );
	xx3 = resi; resi = xx1;
	xx1 = concat_extract<12> ( xx1, xx3 ); // aligned
	xx2 = permutate<1,0,3,2> (xx1);
	yy1 = add (xx1, xx2); // sample 0 and 2
	yy1 = and (yy1, (vcs) _ODD_MASK );	
	xx1 = (vcs) mul_shift<15> ( vins, c_r1[c_idx] );
	xx2 = permutate<1,0,3,2> (xx1);
	yy2 = add (xx1, xx2); // sample 1 and 3
	yy2 = and (yy2, (vcs) _EVEN_MASK );
	r = or (yy1, yy2);
}

// input  = 3 16bit I/Q samples (3 COMPLEX16 <  1 vcs)
// output = 4 16bit I/Q samples (4 COMPLEX16 == 1 vcs)
DSP_INLINE static void Upsample40MTo44M_3 ( const COMPLEX16* pInput, COMPLEX16* pOutput )
{
    const vcs& in = *(vcs*)pInput;
    vcs& out = *(vcs*)pOutput;
    assert(in[3].re == 0 && in[3].im == 0);

	const vs * pcoff1_r0 = (const vs*) g_coff1_row0;
	const vs * pcoff1_r1 = (const vs*) g_coff1_row1;

    vcs resi;
    set_zero (resi);

	vcs rr1;
	compute_4 ( rr1, (vs&)in, resi, pcoff1_r0, pcoff1_r1, 0 );
    out = rr1;
}

// input  = 160 16bit I/Q samples (160 COMPLEX16 == 40 vcs)
// output = 176 16bit I/Q samples (176 COMPLEX16 == 44 vcs)
DSP_INLINE static void Upsample40MTo44M_160 ( const COMPLEX16* pInput, COMPLEX16* pOutput )
{
	const vs *  pvsi = (const vs*) pInput;
	COMPLEX16 * po = (COMPLEX16*) pOutput;
	
	const vs * pcoff1_r0 = (const vs*) g_coff1_row0;
	const vs * pcoff1_r1 = (const vs*) g_coff1_row1;
	
	// residule
	vcs resi;
    set_zero (resi);
	
	vcs rr1;
	vs  vin, vin1;

	int idx = 0;   // in vs
	int cnt = 160; // in samples
	while (true) {
		// compute 20 samples in a batch
		vin = pvsi[idx++];
		compute_4 ( rr1, vin, resi, pcoff1_r0, pcoff1_r1, 0 );
		store ( po, rr1 );
		po += 4;

		vin = pvsi[idx++]; 
		compute_4 ( rr1, vin, resi, pcoff1_r0, pcoff1_r1, 1 );
		store ( po, rr1 );
		po += 4;
	
		vin = pvsi[idx++]; 
		compute_4 ( rr1, vin, resi, pcoff1_r0, pcoff1_r1, 2 );
		store ( po, rr1 );
		po += 3;
	
		vin1 = pvsi[idx++];
		vin  = concat_extract<8> (vin1, vin );
		compute_4 ( rr1, vin, resi, pcoff1_r0, pcoff1_r1, 0 );
		store ( po, rr1 );
		po += 4;

		vin = vin1; vin1 = pvsi[idx++];
		vin  = concat_extract<8> (vin1, vin );
		compute_4 ( rr1, vin, resi, pcoff1_r0, pcoff1_r1, 1 );
		store ( po, rr1 );
		po += 4;
	
		vin = vin1; vin1 = pvsi[idx];
		vin  = concat_extract<8> (vin1, vin );
		compute_4 ( rr1, vin, resi, pcoff1_r0, pcoff1_r1, 2 );

		cnt -= 20;
        if (cnt == 0) break;
        store ( po, rr1 );
		po += 3;
	}

    *po++ = rr1[0];
    *po++ = rr1[1];
    *po++ = rr1[2];
}
