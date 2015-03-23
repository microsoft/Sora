/*++
Copyright (c) Microsoft Corporation

Module Name: soradsp.h

Abstract: Sora digital signal processing functions.

--*/
#pragma once

#include <stdlib.h>
#include <const.h>
#include <vector128.h>
#include <soratypes.h>
#include <sora.h>
#include "unroll.h"

#include <rxstream.h>

// Note: force compiler inline this function to minimize memory usage
DSP_INLINE void RemoveDC(SignalBlock& block, const vcs& dc)
{
    // Helper functor class for RemoveDC(), used together with unroll_for to unroll for-loop to achieve better
    // code generation.
    class VcsRemoveDC
    {
        const vcs& dc;
        SignalBlock& block;
        //VcsRemoveDC& operator=(const VcsRemoveDC&) { return *this; } // make compiler happy, calling forbidden
    public:
        VcsRemoveDC(const vcs& dc, SignalBlock& block)
            : dc(dc), block(block)
        { }

        void operator()(size_t j)
        {
            block[j] = sub(block[j], dc);
        }
    };

	VcsRemoveDC removeDC(dc, block);
	unroll_for<0, SignalBlock::size, 1>(removeDC);
}

// Check all COMPLEX16 items in a SignalBlock, and find the smallest square to cover them all
// Returns:
//   pLeast: The 1/2 of the edge width of the sqaure 
FINL __int16 SoraEstimateEnergyScope(const SignalBlock& block)
{
    vcs hi, lo;
    hi = smax(
            smax(smax(block[0], block[1]), smax(block[2], block[3])),
            smax(smax(block[4], block[5]), block[6]));
    lo = smin(
            smin(smin(block[0], block[1]), smin(block[2], block[3])),
            smin(smin(block[4], block[5]), block[6]));

    lo = sub(vector128_consts::zero<vcs>(), lo);
    hi = smax(hi, lo);

    return hmax((vs&)hi)[0];
}

// Calculates Direct-Current(DC) offset from a SignalBlock
FINL vcs SoraCalcDC(const SignalBlock& block)
{
    SignalBlock b;
    b[0] = shift_right(block[0], 2);
    b[1] = shift_right(block[1], 3);
    b[2] = shift_right(block[2], 3);
    b[3] = shift_right(block[3], 3);
    b[4] = shift_right(block[4], 3);
    b[5] = shift_right(block[5], 3);
    b[6] = shift_right(block[6], 3);

    vcs sum = add (add (add(b[0], b[1]), add(b[2], b[3])), add(add(b[4], b[5]), b[6]));
    sum = shift_right(sum, 2);
    sum = hadd(sum);
    return sum;
}

// Updates Direct-Current(DC) offset from new a SignalBlock
// Parameters:
//   oldDC: old DC offset.
FINL vcs SoraUpdateDC (const SignalBlock& block, const vcs& oldDC)
{
    vcs newDC = SoraCalcDC(block);

    // Multiply oldDC by 15/16
    vcs oldDC_15_16 = sub (oldDC, shift_right(oldDC, 4));

    // Add 1/16 newDC to oldDC
    return add(oldDC_15_16, shift_right(newDC, 4));
}

// Decreases energy of sample block saved in SignalBlock linearly.
// Parameters:
//   shift: decrease level. Result sample will be equal to original sample >> shift.
FINL void SoraRightShiftSignalBlock(SignalBlock& block, unsigned int shift)
{
    ASSERT(shift < 16);     // ref: sdr_phy_rx.c __GetDagcShiftRightBits()
    block[0] = shift_right(block[0], shift);
    block[1] = shift_right(block[1], shift);
    block[2] = shift_right(block[2], shift);
    block[3] = shift_right(block[3], shift);
    block[4] = shift_right(block[4], shift);
    block[5] = shift_right(block[5], shift);
    block[6] = shift_right(block[6], shift);
}

// Increases energy of sample block saved in SignalBlock linearly.
// Parameters:
//   shift: increase level. Result sample will be equal to original sample << shift.
FINL void SoraLeftShiftSignalBlock(SignalBlock& block, unsigned int shift)
{
    ASSERT(shift < 16);     // ref: sdr_phy_rx.c __GetDagcShiftRightBits()
    block[0] = shift_left(block[0], shift);
    block[1] = shift_left(block[1], shift);
    block[2] = shift_left(block[2], shift);
    block[3] = shift_left(block[3], shift);
    block[4] = shift_left(block[4], shift);
    block[5] = shift_left(block[5], shift);
    block[6] = shift_left(block[6], shift);
}

// Calculates 1/16 of sum of 1-norm of complex numbers in one SignalBlock
FINL vcs SoraPackNorm1SignalBlock(const SignalBlock& block)
{
    vs b0, b1, b2, b3, b4, b5, b6;
    b0 = abs0((vs&)block[0]);
    b1 = abs0((vs&)block[1]);
    b2 = abs0((vs&)block[2]);
    b3 = abs0((vs&)block[3]);
    b4 = abs0((vs&)block[4]);
    b5 = abs0((vs&)block[5]);
    b6 = abs0((vs&)block[6]);

    vs x0, x1, x2, sum;
    x0 = average((vus&)b1, (vus&)b2);
    x1 = average((vus&)b3, (vus&)b4);
    x2 = average((vus&)b5, (vus&)b6);
    x1 = average((vus&)x1, (vus&)x2);

    sum = add( shift_right(b0, 3), shift_right(x0, 2) );
    sum = add( sum, shift_right(x1, 1) );

    vcs pack;
    pack = average((vcus&)sum, flip((vcus&)sum));
    return pack;
}

// Calculates 1/4 of sum of 1-norm of complex numbers in one SignalBlock
FINL vui SoraGetNorm(const SignalBlock& block)
{
    vs b0, b1, b2, b3, b4, b5, b6;
    b0 = abs0((vs&)block[0]);
    b1 = abs0((vs&)block[1]);
    b2 = abs0((vs&)block[2]);
    b3 = abs0((vs&)block[3]);
    b4 = abs0((vs&)block[4]);
    b5 = abs0((vs&)block[5]);
    b6 = abs0((vs&)block[6]);

    vs x0, x1, x2, x3;
    x0 = average((vus&)b0, (vus&)b1);
    x1 = average((vus&)b2, (vus&)b3);
    x2 = average((vus&)b4, (vus&)b5);
    x3 = shift_right(b6, 1);

    vs zero;
    set_zero(zero);

    vui a0, a1, a2, a3;
    a0 = add (interleave_low(x0, zero), interleave_high(x0, zero));
    a1 = add (interleave_low(x1, zero), interleave_high(x1, zero));
    a2 = add (interleave_low(x2, zero), interleave_high(x2, zero));
    a3 = add (interleave_low(x3, zero), interleave_high(x3, zero));

    vui sum = add (add(a0, a1), add(a2, a3));
    sum = hadd(sum);
    return shift_right(sum, 1);
}

template<class TPOINTER>
size_t buffer_span(TPOINTER beg, TPOINTER end, size_t size)
{
    return (end - beg + size) % size;
}

FINL void demap_dqpsk_bits (UCHAR& r, int pos, const COMPLEX16 & ref, const COMPLEX16 & s )
{
	long re = (ref.re*s.re + ref.im * s.im );
	long im = (ref.re*s.im - ref.im * s.re );
    re >>= 1;
    im >>= 1;
	r |= (UCHAR) ((unsigned long)(re+im) >> 31) << pos;
	r |= (UCHAR) ((unsigned long)(re-im) >> 31) << (pos+1);
}
