#pragma once

#include "vector128.h"

#define NORM_AC_SHIFT 5
DSP_INLINE1 int GetAutoCorrelation(const SignalBlock& inBlock, vcs * const histAutoCorrelation)
{
#if 0
    vi re[7], im;

	conj_mul(re[0], im, inBlock[0], histAutoCorrelation[0]);
    conj_mul(re[1], im, inBlock[1], histAutoCorrelation[1]);
    conj_mul(re[2], im, inBlock[2], histAutoCorrelation[2]);
    conj_mul(re[3], im, inBlock[3], histAutoCorrelation[3]);
    conj_mul(re[4], im, inBlock[4], inBlock[0]);
    conj_mul(re[5], im, inBlock[5], inBlock[1]);
    conj_mul(re[6], im, inBlock[6], inBlock[2]);

    vi s1 = shift_right ( add(re[0], re[1]), NORM_AC_SHIFT );
    vi s2 = shift_right ( add(re[2], re[3]), NORM_AC_SHIFT );
    vi s3 = shift_right ( add(re[4], re[5]), NORM_AC_SHIFT );
    vi s4 = shift_right ( re[6], NORM_AC_SHIFT );

    vi sum1 = add (s1, s2);
    vi sum2 = add (s3, s4);
    
	vi sum = add(sum1, sum2);
	sum = hadd(sum);  

    // Save history
    histAutoCorrelation[0] = inBlock[3];
    histAutoCorrelation[1] = inBlock[4];
    histAutoCorrelation[2] = inBlock[5];
    histAutoCorrelation[3] = inBlock[6];

    return sum[0];
#else
	vi re[7], im[7], sum_re, sum_im;
	conj_mul(re[0], im[0], inBlock[0], histAutoCorrelation[0]);
	conj_mul(re[1], im[1], inBlock[1], histAutoCorrelation[1]);
	conj_mul(re[2], im[2], inBlock[2], histAutoCorrelation[2]);
	conj_mul(re[3], im[3], inBlock[3], histAutoCorrelation[3]);
    conj_mul(re[4], im[4], inBlock[4], inBlock[0]);
    conj_mul(re[5], im[5], inBlock[5], inBlock[1]);
    conj_mul(re[6], im[6], inBlock[6], inBlock[2]);

	set_zero (sum_re);
	rep<7>::vshift_right (re, NORM_AC_SHIFT);
	rep<7>::vsum (sum_re, re);
	sum_re = hadd (sum_re );

	set_zero (sum_im);
	rep<7>::vshift_right (im, NORM_AC_SHIFT);
	rep<7>::vsum (sum_im, im);
	sum_im = hadd (sum_im );
	
    // We use 1-norm of complex number as an approximation of Euclidean norm,
    // because it is fast and enough for carrier sensing
	int acorr = (sum_re[0] + sum_im[0]);
	
    histAutoCorrelation[0] = inBlock[3];
    histAutoCorrelation[1] = inBlock[4];
    histAutoCorrelation[2] = inBlock[5];
    histAutoCorrelation[3] = inBlock[6];
	return (acorr);	
#endif
}

DSP_INLINE1 int GetEnergy(const SignalBlock& inBlock)
{
#if 0
    vi re[7], im;

	conj_mul(re[0], im, inBlock[0], inBlock[0]);
	conj_mul(re[1], im, inBlock[1], inBlock[1]);
	conj_mul(re[2], im, inBlock[2], inBlock[2]);
	conj_mul(re[3], im, inBlock[3], inBlock[3]);
	conj_mul(re[4], im, inBlock[4], inBlock[4]);
	conj_mul(re[5], im, inBlock[5], inBlock[5]);
	conj_mul(re[6], im, inBlock[6], inBlock[6]);

    vi s1 = shift_right ( add(re[0], re[1]), NORM_AC_SHIFT );
    vi s2 = shift_right ( add(re[2], re[3]), NORM_AC_SHIFT );
    vi s3 = shift_right ( add(re[4], re[5]), NORM_AC_SHIFT );
    vi s4 = shift_right ( re[6], NORM_AC_SHIFT );

    vi sum1 = add (s1, s2);
    vi sum2 = add (s3, s4);
    vi sum = add ( sum1, sum2 );
    
    sum = hadd(sum);
    return sum[0];
#else
	vi sum;

	sum = shift_right ( SquaredNorm (inBlock[0]), NORM_AC_SHIFT);
	sum = add (sum, shift_right ( SquaredNorm (inBlock[1]), NORM_AC_SHIFT));
	sum = add (sum, shift_right ( SquaredNorm (inBlock[2]), NORM_AC_SHIFT));
	sum = add (sum, shift_right ( SquaredNorm (inBlock[3]), NORM_AC_SHIFT));
	sum = add (sum, shift_right ( SquaredNorm (inBlock[4]), NORM_AC_SHIFT));
	sum = add (sum, shift_right ( SquaredNorm (inBlock[5]), NORM_AC_SHIFT));
	sum = add (sum, shift_right ( SquaredNorm (inBlock[6]), NORM_AC_SHIFT));
	sum = hadd (sum );
	return sum[0];
#endif
}
