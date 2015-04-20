#pragma once

#include "vector128.h"
#include "soradsp.h"
#include "tpltrick.h"
#include "brick.h"
#include "ieee80211facade.hpp"

DEFINE_LOCAL_CONTEXT(TCCK5P5Decoder, CF_DifferentialDemap);
template<TFILTER_ARGS>
class TCCK5P5Decoder : public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW (COMPLEX16, last_symbol);
public:
    REFERENCE_LOCAL_CONTEXT(TCCK5P5Decoder);
    DEFINE_IPORT(COMPLEX16, 16);
    DEFINE_OPORT(unsigned char, 1);
    STD_TFILTER_CONSTRUCTOR(TCCK5P5Decoder)
        BIND_CONTEXT(CF_DifferentialDemap::last_symbol, last_symbol)
    {
    }

	STD_TFILTER_RESET() { }
	STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            BOOLEAN b_isEven = 0;
            unsigned char b_byteTemp5M;

            const COMPLEX16 *pc = ipin.peek();
            BB11BCCK5p5DecodeHalfByte(pc, &b_byteTemp5M,
                &last_symbol, &b_isEven);
            BB11BCCK5p5DecodeHalfByte(pc + 8, &b_byteTemp5M,
                &last_symbol, &b_isEven);
            ipin.pop();

            *opin().append() = b_byteTemp5M;
            bool rc = Next()->Process(opin());
            if (!rc) return false;
        }
        return true;
    }

private:
    FINL int BB11BCCK5p5DecodeHalfByte(
                const COMPLEX16     *input, 
                UCHAR               *output, 
                COMPLEX16           *ref, 
                IN OUT BOOLEAN      *is_even)
    {
        if (*is_even == 0)
        {
            *output = 0;
            CCK5P5_EVEN_DECODER(output, input[0], input[1], input[2], input[3],
                    input[4], input[5], input[6], input[7], ref);
            *is_even = TRUE;
            return 0;
        }
        else
        {
            CCK5P5_ODD_DECODER(output, input[0], input[1], input[2], input[3],
                    input[4], input[5], input[6], input[7], ref);
            *is_even = FALSE;
            return 1;
        }
    }

    FINL void CCK5P5_EVEN_DECODER(UCHAR *pbDecodedValue, const COMPLEX16& P0, const COMPLEX16& P1, const COMPLEX16& P2, const COMPLEX16& P3, const COMPLEX16& P4, const COMPLEX16& P5, const COMPLEX16& P6, const COMPLEX16& P7, COMPLEX16 *pLastItem)
    {
        vci A0, A1;
        vci B;
        A32 COMPLEX32 L1;
    
        long Max1 = 0, Max2 = 0;
        unsigned char Value1 = 0, Value2 = 0;
    
        A0[0].re = P0.im + P1.re;
        A0[0].im = P1.im - P0.re;
        A0[1].re = P2.im - P3.re;
        A0[1].im = -(P2.re + P3.im);
        A1[0].re = P4.im + P5.re;
        A1[0].im = P5.im - P4.re;
        A1[1].re = P7.re - P6.im;
        A1[1].im = P6.re + P7.im;
    
        B[0].re = A0[0].re + A0[1].re; B[0].im = A0[0].im + A0[1].im;
        B[1].re = A1[0].re + A1[1].re; B[1].im = A1[0].im + A1[1].im;
    
        B[0].im = -B[0].im;
        B = shift_right(B, 2);
    
        L1.re = B[0].re * B[1].re - B[0].im * B[1].im;
        L1.im = B[0].im * B[1].re + B[0].re * B[1].im;
    
        if (L1.re > 0) {
            Max1 = L1.re; Value1 = 0x00;
        } else {
            Max1 = -L1.re; Value1 = 0x08;
        }
        A0[0].re = P1.re - P0.im;
        A0[0].im = P0.re + P1.im;
        A0[1].re = -(P2.im + P3.re);
        A0[1].im = P2.re - P3.im;
        A1[0].re = P5.re - P4.im;
        A1[0].im = P4.re + P5.im;
        A1[1].re = P6.im + P7.re;
        A1[1].im = P7.im - P6.re;
    
        B[0].re = A0[0].re + A0[1].re; B[0].im = A0[0].im + A0[1].im;
        B[1].re = A1[0].re + A1[1].re; B[1].im = A1[0].im + A1[1].im;
    
        B[0].im = -B[0].im;
        B = shift_right(B, 2);
    
        L1.re = B[0].re * B[1].re - B[0].im * B[1].im;
        L1.im = B[0].im * B[1].re + B[0].re * B[1].im;
    
    
        if (L1.re > 0) {
            Max2 = L1.re; Value2 = 0x04;
        } else {
            Max2 = -L1.re; Value2 = 0x0c;
        }
    
        if(Max1 > Max2) {
            *pbDecodedValue = Value1;
        } else {
            *pbDecodedValue = Value2;
        }
        demap_dqpsk_bits((*pbDecodedValue), 0, (*pLastItem), (P7));
        pLastItem->re = P7.re;
        pLastItem->im = P7.im;
    }

    FINL void CCK5P5_ODD_DECODER(UCHAR *pbDecodedValue, const COMPLEX16& P0, const COMPLEX16& P1, const COMPLEX16& P2, const COMPLEX16& P3, const COMPLEX16& P4, const COMPLEX16& P5, const COMPLEX16& P6, const COMPLEX16& P7, COMPLEX16 *pLastItem)
    {
        vci A0, A1;
        vci B;
        A32 COMPLEX32 L1;
    
        long Max1 = 0, Max2 = 0;
        unsigned char Value1 = 0, Value2 = 0;
    
        A0[0].re = P0.im + P1.re;
        A0[0].im = P1.im - P0.re;
        A0[1].re = P2.im - P3.re;
        A0[1].im = -(P2.re + P3.im);
        A1[0].re = P4.im + P5.re;
        A1[0].im = P5.im - P4.re;
        A1[1].re = P7.re - P6.im;
        A1[1].im = P6.re + P7.im;
    
        B[0].re = A0[0].re + A0[1].re; B[0].im = A0[0].im + A0[1].im;
        B[1].re = A1[0].re + A1[1].re; B[1].im = A1[0].im + A1[1].im;
    
        B[0].im = -B[0].im;
        B = shift_right(B, 2);
    
        L1.re = B[0].re * B[1].re - B[0].im * B[1].im;
        L1.im = B[0].im * B[1].re + B[0].re * B[1].im;
    
        if (L1.re > 0) {
            Max1 = L1.re; Value1 = 0x00;
        } else {
            Max1 = -L1.re; Value1 = 0x80;
        }
    
        A0[0].re = P1.re - P0.im;
        A0[0].im = P0.re + P1.im;
        A0[1].re = -(P2.im + P3.re);
        A0[1].im = P2.re - P3.im;
        A1[0].re = P5.re - P4.im;
        A1[0].im = P4.re + P5.im;
        A1[1].re = P6.im + P7.re;
        A1[1].im = P7.im - P6.re;
    
        B[0].re = A0[0].re + A0[1].re; B[0].im = A0[0].im + A0[1].im;
        B[1].re = A1[0].re + A1[1].re; B[1].im = A1[0].im + A1[1].im;
    
        B[0].im = -B[0].im;
        B = shift_right(B, 2);
    
        L1.re = B[0].re * B[1].re - B[0].im * B[1].im;
        L1.im = B[0].im * B[1].re + B[0].re * B[1].im;
    
    
        if (L1.re > 0) {
            Max2 = L1.re; Value2 = 0x40;
        } else {
            Max2 = -L1.re; Value2 = 0xc0;
        }
    
        if(Max1 > Max2) {
            *pbDecodedValue |= Value1;
        } else {
            *pbDecodedValue |= Value2;
        }
        demap_dqpsk_bits((*pbDecodedValue), 4, (*pLastItem), (P7));
        *pbDecodedValue ^= 0x30; 
        pLastItem->re = P7.re;
        pLastItem->im = P7.im;
    }
};

DEFINE_LOCAL_CONTEXT(TCCK11Decoder, CF_DifferentialDemap);
template<TFILTER_ARGS>
class TCCK11Decoder : public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW (COMPLEX16, last_symbol);
protected:
	// internal states
    BOOLEAN                 is_even;

    void _init()
    {
        is_even = 0;
    }

public:
    REFERENCE_LOCAL_CONTEXT(TCCK11Decoder);
    DEFINE_IPORT(COMPLEX16, 8);
    DEFINE_OPORT(UCHAR, 1);
    STD_TFILTER_CONSTRUCTOR(TCCK11Decoder)
        BIND_CONTEXT(CF_DifferentialDemap::last_symbol, last_symbol)
    {
        _init();
    }

	STD_TFILTER_RESET()
    {
        _init();
    }
	STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
        {
            const COMPLEX16 *input = ipin.peek();

            UCHAR b = BB11BCCK11DecodeByte(input, &last_symbol, &is_even);
            ipin.pop();

            *opin().append() = b;
            bool rc = Next()->Process(opin());
            if (!rc) return false;
        }
        return true;
    }

private:
    FINL UCHAR BB11BCCK11DecodeByte(const COMPLEX16 *input, COMPLEX16 *ref, BOOLEAN *is_even)
    {
        UCHAR ret;

        CCK11_DECODER(&ret, input[0], input[1], input[2], input[3],
                input[4], input[5], input[6], input[7], ref, (*is_even));

        return ret;
    }

    FINL void CCK11_DECODER(UCHAR *pbDecodedValue, const COMPLEX16& P0, const COMPLEX16& P1, const COMPLEX16& P2, const COMPLEX16& P3, const COMPLEX16& P4, const COMPLEX16& P5, const COMPLEX16& P6, const COMPLEX16& P7, COMPLEX16 *pLastItem, BOOLEAN& bEven)
    {
        A32 COMPLEX32 A1, A2, A3, A4; 
        vci B0, B1, B2, B3; 
        A32 COMPLEX32 L1, L2, L3, L4; 
    
        long Module1Max = 0, Module2Max = 0, Module34Max = 0; 
        long Max1 = 0, Max2 = 0, Max3 = 0, Max4 = 0; 
        unsigned char Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0; 
        unsigned char Module1DecodedValue = 0, Module2DecodedValue = 0, Module34DecodedValue = 0; 
    
        long ABS1 = 0, ABS2 = 0; 
        A1.re = P0.re + P1.re;
        A1.im = P0.im + P1.im;
        A2.re = P2.re - P3.re;
        A2.im = P2.im - P3.im;
        A3.re = P4.re + P5.re;
        A3.im = P4.im + P5.im;
        A4.re = P7.re - P6.re;
        A4.im = P7.im - P6.im;
    
        B0[0].re = A2.re + A1.re; B0[0].im = A2.im + A1.im;
        B1[0].re = A2.re - A1.re; B1[0].im = A2.im - A1.im;
        B0[1].re = A2.re - A1.im; B0[1].im = A2.im + A1.re;
        B1[1].re = A2.re + A1.im; B1[1].im = A2.im - A1.re;
    
        B2[0].re = A4.re + A3.re; B2[0].im = A4.im + A3.im;
        B3[0].re = A4.re - A3.re; B3[0].im = A4.im - A3.im;
        B2[1].re = A4.re - A3.im; B2[1].im = A4.im + A3.re;
        B3[1].re = A4.re + A3.im; B3[1].im = A4.im - A3.re;
        B0 = shift_right(B0, 2);
        B1 = shift_right(B1, 2);
        B2 = shift_right(B2, 2);
        B3 = shift_right(B3, 2);
    
        L1.re = B0[0].re * B2[0].re + B0[0].im * B2[0].im;
        L1.im = B0[0].re * B2[0].im - B0[0].im * B2[0].re;
        L2.re = B0[1].re * B2[1].re + B0[1].im * B2[1].im;
        L2.im = B0[1].re * B2[1].im - B0[1].im * B2[1].re;
        L3.re = B1[0].re * B3[0].re + B1[0].im * B3[0].im;
        L3.im = B1[0].re * B3[0].im - B1[0].im * B3[0].re;
        L4.re = B1[1].re * B3[1].re + B1[1].im * B3[1].im;
        L4.im = B1[1].re * B3[1].im - B1[1].im * B3[1].re;
    
        if(L1.re < 0) ABS1 = -L1.re; else ABS1 = L1.re;
        if(L1.im < 0) ABS2 = -L1.im; else ABS2 = L1.im;
        if (ABS1 > ABS2) {
            if (L1.re > 0) {
                Max1 = L1.re; Value1 = 0x00;
            } else {
                Max1 = -L1.re; Value1 = 0x40;
            }
        } else {
            if (L1.im > 0) {
                Max1 = L1.im; Value1 = 0xC0;
            } else {
                Max1 = -L1.im; Value1 = 0x80;
            }
        }
    
        if(L2.re < 0) ABS1 = -L2.re; else ABS1 = L2.re;
        if(L2.im < 0) ABS2 = -L2.im; else ABS2 = L2.im;
        if (ABS1 > ABS2) {
            if (L2.re > 0) {
                Max2 = L2.re; Value2 = 0x30;
            } else {
                Max2 = -L2.re; Value2 = 0x70;
            }
        } else {
            if (L2.im > 0) {
                Max2 = L2.im; Value2 = 0xF0;
            } else {
                Max2 = -L2.im; Value2 = 0xB0;
            }
        }
    
        if(L3.re < 0) ABS1 = -L3.re; else ABS1 = L3.re;
        if(L3.im < 0) ABS2 = -L3.im; else ABS2 = L3.im;
        if (ABS1 > ABS2) {
            if (L3.re > 0) {
                Max3 = L3.re; Value3 = 0x10; /*0001*/
            } else {
                Max3 = -L3.re; Value3 = 0x50; /*1001*/
            }
        } else {
            if (L3.im > 0) {
                Max3 = L3.im; Value3 = 0xD0; /*1101*/
            } else {
                Max3 = -L3.im; Value3 = 0x90;/*1001*/
            }
        }
    
        if(L4.re < 0) ABS1 = -L4.re; else ABS1 = L4.re;
        if(L4.im < 0) ABS2 = -L4.im; else ABS2 = L4.im;
        if (ABS1 > ABS2) {
            if (L4.re > 0) {
                Max4 = L4.re; Value4 = 0x20; /*0010*/
            } else {
                Max4 = -L4.re; Value4 = 0x60; /*0110*/
            }
        } else {
            if (L4.im > 0) {
                Max4 = L4.im; Value4 = 0xE0; /*1110*/
            } else {
                Max4 = -L4.im; Value4 = 0xA0; /*1010*/
            }
        }
    
    
        if (Max1 > Max2) {
            Module1Max = Max1; Module1DecodedValue = Value1;
        } else {
            Module1Max = Max2; Module1DecodedValue = Value2;
        }
    
        if (Max3 > Max4) {
            if(Max3 > Module1Max) {
                Module1Max = Max3; Module1DecodedValue = Value3;
            }
        } else {
            if(Max4 > Module1Max) {
                Module1Max = Max4; Module1DecodedValue = Value4;
            }
        }
    
        A1.re = P0.im + P1.re;
        A1.im = P1.im - P0.re;
        A2.re = P2.im - P3.re;
        A2.im = -(P2.re + P3.im);
        A3.re = P4.im + P5.re;
        A3.im = P5.im - P4.re;
        A4.re = P7.re - P6.im;
        A4.im = P6.re + P7.im;
    
        B0[0].re = A1.re + A2.re; B0[0].im = A1.im + A2.im;
        B1[0].re = A2.re - A1.re; B1[0].im = A2.im - A1.im;
        B0[1].re = A2.re - A1.im; B0[1].im = A2.im + A1.re;
        B1[1].re = A2.re + A1.im; B1[1].im = A2.im - A1.re;
    
        B2[0].re = A3.re + A4.re; B2[0].im = A3.im + A4.im;
        B3[0].re = A4.re - A3.re; B3[0].im = A4.im - A3.im;
        B2[1].re = A4.re - A3.im; B2[1].im = A4.im + A3.re;
        B3[1].re = A4.re + A3.im; B3[1].im = A4.im - A3.re;
        B0 = shift_right(B0, 2);
        B1 = shift_right(B1, 2);
        B2 = shift_right(B2, 2);
        B3 = shift_right(B3, 2);
    
        L1.re = B0[0].re * B2[0].re + B0[0].im * B2[0].im;
        L1.im = B0[0].re * B2[0].im - B0[0].im * B2[0].re;
        L2.re = B0[1].re * B2[1].re + B0[1].im * B2[1].im;
        L2.im = B0[1].re * B2[1].im - B0[1].im * B2[1].re;
        L3.re = B1[0].re * B3[0].re + B1[0].im * B3[0].im;
        L3.im = B1[0].re * B3[0].im - B1[0].im * B3[0].re;
        L4.re = B1[1].re * B3[1].re + B1[1].im * B3[1].im;
        L4.im = B1[1].re * B3[1].im - B1[1].im * B3[1].re;
    
        if(L1.re < 0) ABS1 = -L1.re; else ABS1 = L1.re;
        if(L1.im < 0) ABS2 = -L1.im; else ABS2 = L1.im;
        if (ABS1 > ABS2) {
            if (L1.re > 0) {
                Max1 = L1.re; Value1 = 0x00;
            } else {
                Max1 = -L1.re; Value1 = 0x40;
            }
        } else {
            if (L1.im > 0) {
                Max1 = L1.im; Value1 = 0xC0;
            } else {
                Max1 = -L1.im; Value1 = 0x80;
            }
        }
    
        if(L2.re < 0) ABS1 = -L2.re; else ABS1 = L2.re;
        if(L2.im < 0) ABS2 = -L2.im; else ABS2 = L2.im;
        if (ABS1 > ABS2) {
            if (L2.re > 0) {
                Max2 = L2.re; Value2 = 0x30;
            } else {
                Max2 = -L2.re; Value2 = 0x70;
            }
        } else {
            if (L2.im > 0) {
                Max2 = L2.im; Value2 = 0xF0;
            } else {
                Max2 = -L2.im; Value2 = 0xB0;
            }
        }
    
        if(L3.re < 0) ABS1 = -L3.re; else ABS1 = L3.re;
        if(L3.im < 0) ABS2 = -L3.im; else ABS2 = L3.im;
        if (ABS1 > ABS2) {
            if (L3.re > 0) {
                Max3 = L3.re; Value3 = 0x10;
            } else {
                Max3 = -L3.re; Value3 = 0x50;
            }
        } else {
            if (L3.im > 0) {
                Max3 = L3.im; Value3 = 0xD0;
            } else {
                Max3 = -L3.im; Value3 = 0x90;
            }
        }
    
        if(L4.re < 0) ABS1 = -L4.re; else ABS1 = L4.re;
        if(L4.im < 0) ABS2 = -L4.im; else ABS2 = L4.im;
        if (ABS1 > ABS2) {
            if (L4.re > 0) {
                Max4 = L4.re; Value4 = 0x20;
            } else {
                Max4 = -L4.re; Value4 = 0x60;
            }
        } else {
            if (L4.im > 0) {
                Max4 = L4.im; Value4 = 0xE0;
            } else {
                Max4 = -L4.im; Value4 = 0xA0;
            }
        }
    
        if (Max1 > Max2) {
            Module2Max = Max1; Module2DecodedValue = Value1;
        } else {
            Module2Max = Max2; Module2DecodedValue = Value2;
        }
        if (Max3 > Max4) {
            if(Max3 > Module2Max) {
                Module2Max = Max3; Module2DecodedValue = Value3;
            }
        } else {
            if(Max4 > Module2Max) {
                Module2Max = Max4; Module2DecodedValue = Value4;
            }
        }
    
        Module2DecodedValue |= 0x08;
    
        if(Module1Max > Module2Max) {
            goto lable4;
        }
    
        A1.re = P1.re - P0.re;
        A1.im = P1.im - P0.im;
        A2.re = -(P2.re + P3.re);
        A2.im = -(P2.im + P3.im);
        A3.re = P5.re - P4.re;
        A3.im = P5.im - P4.im;
        A4.re = P6.re + P7.re;
        A4.im = P6.im + P7.im;
    
        B0[0].re = A1.re + A2.re; B0[0].im = A1.im + A2.im;
        B1[0].re = A2.re - A1.re; B1[0].im = A2.im - A1.im;
        B0[1].re = A2.re - A1.im; B0[1].im = A2.im + A1.re;
        B1[1].re = A2.re + A1.im; B1[1].im = A2.im - A1.re;
    
        B2[0].re = A3.re + A4.re; B2[0].im = A3.im + A4.im;
        B3[0].re = A4.re - A3.re; B3[0].im = A4.im - A3.im;
        B2[1].re = A4.re - A3.im; B2[1].im = A4.im + A3.re;
        B3[1].re = A4.re + A3.im; B3[1].im = A4.im - A3.re;
        B0 = shift_right(B0, 2);
        B1 = shift_right(B1, 2);
        B2 = shift_right(B2, 2);
        B3 = shift_right(B3, 2);
    
        L1.re = B0[0].re * B2[0].re + B0[0].im * B2[0].im;
        L1.im = B0[0].re * B2[0].im - B0[0].im * B2[0].re;
        L2.re = B0[1].re * B2[1].re + B0[1].im * B2[1].im;
        L2.im = B0[1].re * B2[1].im - B0[1].im * B2[1].re;
        L3.re = B1[0].re * B3[0].re + B1[0].im * B3[0].im;
        L3.im = B1[0].re * B3[0].im - B1[0].im * B3[0].re;
        L4.re = B1[1].re * B3[1].re + B1[1].im * B3[1].im;
        L4.im = B1[1].re * B3[1].im - B1[1].im * B3[1].re;
    
        if(L1.re < 0) ABS1 = -L1.re; else ABS1 = L1.re;
        if(L1.im < 0) ABS2 = -L1.im; else ABS2 = L1.im;
        if (ABS1 > ABS2) {
            if (L1.re > 0) {
                Max1 = L1.re; Value1 = 0x00;
            } else {
                Max1 = -L1.re; Value1 = 0x40;
            }
        } else {
            if (L1.im > 0)
            {
                Max1 = L1.im; Value1 = 0xC0;
            } else {
                Max1 = -L1.im; Value1 = 0x80;
            }
        }
    
        if(L2.re < 0) ABS1 = -L2.re; else ABS1 = L2.re;
        if(L2.im < 0) ABS2 = -L2.im; else ABS2 = L2.im;
        if (ABS1 > ABS2) {
            if (L2.re > 0) {
                Max2 = L2.re; Value2 = 0x30;
            } else {
                Max2 = -L2.re; Value2 = 0x70;
            }
        } else {
            if (L2.im > 0) {
                Max2 = L2.im; Value2 = 0xF0;
            } else {
                Max2 = -L2.im; Value2 = 0xB0;
            }
        }
    
        if(L3.re < 0) ABS1 = -L3.re; else ABS1 = L3.re;
        if(L3.im < 0) ABS2 = -L3.im; else ABS2 = L3.im;
        if (ABS1 > ABS2) {
            if (L3.re > 0) {
                Max3 = L3.re; Value3 = 0x10;
            } else {
                Max3 = -L3.re; Value3 = 0x50;
            }
        } else {
            if (L3.im > 0) {
                Max3 = L3.im; Value3 = 0xD0;
            } else {
                Max3 = -L3.im; Value3 = 0x90;
            }
        }
    
        if(L4.re < 0) ABS1 = -L4.re; else ABS1 = L4.re;
        if(L4.im < 0) ABS2 = -L4.im; else ABS2 = L4.im;
    
        if (ABS1 > ABS2) {
            if (L4.re > 0) {
                Max4 = L4.re; Value4 = 0x20;
            } else {
                Max4 = -L4.re; Value4 = 0x60;
            }
        } else {
            if (L4.im > 0) {
                Max4 = L4.im; Value4 = 0xE0;
            } else {
                Max4 = -L4.im; Value4 = 0xA0;
            }
        }
    
        if (Max1 > Max2) {
            Module34Max = Max1; Module34DecodedValue = Value1;
        } else {
            Module34Max = Max2; Module34DecodedValue = Value2;
        }
    
        if (Max3 > Max4) {
            if(Max3 > Module34Max) {
                Module34Max = Max3; Module34DecodedValue = Value3;
            }
        } else {
            if(Max4 > Module34Max) {
                Module34Max = Max4; Module34DecodedValue = Value4;
            }
        }
    
        Module34DecodedValue |= 0x04;
    
    
        if(Module2Max > Module34Max) {
            *pbDecodedValue = Module2DecodedValue;
        } else {
            *pbDecodedValue = Module34DecodedValue;
        }
    
        goto lableend;
    
    lable4:
        A1.re = P1.re - P0.im;
        A1.im = P0.re + P1.im;
        A2.re = -(P2.im + P3.re);
        A2.im = P2.re - P3.im;
        A3.re = P5.re - P4.im;
        A3.im = P4.re + P5.im;
        A4.re = P6.im + P7.re;
        A4.im = P7.im - P6.re;
    
        B0[0].re = A1.re + A2.re; B0[0].im = A1.im + A2.im;
        B1[0].re = A2.re - A1.re; B1[0].im = A2.im - A1.im;
        B0[1].re = A2.re - A1.im; B0[1].im = A2.im + A1.re;
        B1[1].re = A2.re + A1.im; B1[1].im = A2.im - A1.re;
    
        B2[0].re = A3.re + A4.re; B2[0].im = A3.im + A4.im;
        B3[0].re = A4.re - A3.re; B3[0].im = A4.im - A3.im;
        B2[1].re = A4.re - A3.im; B2[1].im = A4.im + A3.re;
        B3[1].re = A4.re + A3.im; B3[1].im = A4.im - A3.re;
        B0 = shift_right(B0, 2);
        B1 = shift_right(B1, 2);
        B2 = shift_right(B2, 2);
        B3 = shift_right(B3, 2);
    
        L1.re = B0[0].re * B2[0].re + B0[0].im * B2[0].im;
        L1.im = B0[0].re * B2[0].im - B0[0].im * B2[0].re;
        L2.re = B0[1].re * B2[1].re + B0[1].im * B2[1].im;
        L2.im = B0[1].re * B2[1].im - B0[1].im * B2[1].re;
        L3.re = B1[0].re * B3[0].re + B1[0].im * B3[0].im;
        L3.im = B1[0].re * B3[0].im - B1[0].im * B3[0].re;
        L4.re = B1[1].re * B3[1].re + B1[1].im * B3[1].im;
        L4.im = B1[1].re * B3[1].im - B1[1].im * B3[1].re;
    
        if(L1.re < 0) ABS1 = -L1.re; else ABS1 = L1.re;
        if(L1.im < 0) ABS2 = -L1.im; else ABS2 = L1.im;
    
        if (ABS1 > ABS2) {
            if (L1.re >0) {
                Max1 = L1.re; Value1 = 0x00;
            } else {
                Max1 = -L1.re; Value1 = 0x40;
            }
        } else {
            if (L1.im > 0) {
                Max1 = L1.im; Value1 = 0xC0;
            } else {
                Max1 = -L1.im; Value1 = 0x80;
            }
        }
    
        if(L2.re < 0) ABS1 = -L2.re; else ABS1 = L2.re;
        if(L2.im < 0) ABS2 = -L2.im; else ABS2 = L2.im;
    
        if (ABS1 > ABS2) {
            if (L2.re > 0) {
                Max2 = L2.re; Value2 = 0x30;
            } else {
                Max2 = -L2.re; Value2 = 0x70;
            }
        } else {
            if (L2.im > 0) {
                Max2 = L2.im; Value2 = 0xF0;
            } else {
                Max2 = -L2.im; Value2 = 0xB0;
            }
        }
    
        if(L3.re < 0) ABS1 = -L3.re; else ABS1 = L3.re;
        if(L3.im < 0) ABS2 = -L3.im; else ABS2 = L3.im;
    
        if (ABS1 > ABS2) {
            if (L3.re > 0) {
                Max3 = L3.re; Value3 = 0x10;
            } else {
                Max3 = -L3.re;Value3 = 0x50;
            }
        } else {
            if (L3.im > 0) {
                Max3 = L3.im; Value3 = 0xD0;
            } else {
                Max3 = -L3.im; Value3 = 0x90;
            }
        }
    
        if(L4.re < 0) ABS1 = -L4.re; else ABS1 = L4.re;
        if(L4.im < 0) ABS2 = -L4.im; else ABS2 = L4.im;
    
        if (ABS1 > ABS2) {
            if (L4.re > 0) {
                Max4 = L4.re; Value4 = 0x20;
            } else {
                Max4 = -L4.re; Value4 = 0x60;
            }
        } else {
            if (L4.im > 0) {
                Max4 = L4.im; Value4 = 0xE0;
            } else {
                Max4 = -L4.im; Value4 = 0xA0;
            }
        }
    
        if (Max1 > Max2) {
            Module34Max = Max1; Module34DecodedValue = Value1;
        } else {
            Module34Max = Max2; Module34DecodedValue = Value2;
        }
    
        if (Max3 > Max4) {
            if(Max3 > Module34Max) {
                Module34Max = Max3; Module34DecodedValue = Value3;
            }
        } else {
            if(Max4 > Module34Max) {
                Module34Max = Max4; Module34DecodedValue = Value4;
            }
        }
    
        Module34DecodedValue |= 0x0C;
    
        if(Module1Max > Module34Max) {
            *pbDecodedValue = Module1DecodedValue;
        } else {
            *pbDecodedValue = Module34DecodedValue;
        }
    
    lableend:
        demap_dqpsk_bits((*pbDecodedValue), 0, (*pLastItem), (P7));
        (*pbDecodedValue) ^= ((bEven << 1) | bEven); 
        bEven ^= 0x1;
        pLastItem->re = P7.re;
        pLastItem->im = P7.im;
    }
};

const COMPLEX8 DQPSKEncode[] = { {1, 0}, {0, -1}, {0, 1}, {-1, 0} };
const COMPLEX8 CCK11D3D2[] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
const COMPLEX8 CCK5D3D2[4][8] = {
    {{0, 1}, {1}, {0, 1}, {-1}, {0, 1}, {1}, {0, -1}, {1}},
    {{0, -1}, {1}, {0, -1}, {-1}, {0, -1}, {1}, {0, 1}, {1}},
    {{0, -1}, {-1}, {0, -1}, {1}, {0, 1}, {1}, {0, -1}, {1}},
    {{0, 1}, {-1}, {0, 1}, {1}, {0, -1}, {1}, {0, 1}, {1}},
};

#define CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR 8
typedef struct _LUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX
{
    COMPLEX8     Values[CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR]; // 8 bits are spreaded into 8 complexes, we process a UCHAR(8bits) a time, produce 8 complexes
    UCHAR        bNewRef; // new ref for next step
}LUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX, *PLUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX;

DEFINE_LOCAL_CONTEXT(TCCK11Encode, CF_DifferentialMap);
template<TFILTER_ARGS>
class TCCK11Encode : public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW (uint, last_phase);

    UCHAR                   bEven;

    struct _init_cck11_lut
    {
        void operator()(LUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX (&lut)[256][4][2])
        {
            unsigned int i, d, prev, d7d6, d5d4, d3d2, d1d0;
            COMPLEX8 m4, m3, m2, m1, m0;

            for (d = 0; d < 256; d++)
            for (prev = 0; prev < 4; prev++)
            {
                d1d0 = d % 4;
                d3d2 = (d >> 2) % 4;
                d5d4 = (d >> 4) % 4;
                d7d6 = (d >> 6) % 4;

                // The table for m0 depends on the definition of prev constellation
                m0 = DQPSKEncode[prev];
                m1 = DQPSKEncode[d1d0];
                m2 = CCK11D3D2[d3d2];
                m3 = CCK11D3D2[d5d4];
                m4 = CCK11D3D2[d7d6];

                lut[d][prev][0].Values[0] =   m0 * m1 * m2 * m3 * m4;
                lut[d][prev][0].Values[1] =   m0 * m1 *      m3 * m4;
                lut[d][prev][0].Values[2] =   m0 * m1 * m2 *      m4;
                lut[d][prev][0].Values[3] = - m0 * m1 *           m4;
                lut[d][prev][0].Values[4] =   m0 * m1 * m2 * m3     ;
                lut[d][prev][0].Values[5] =   m0 * m1 *      m3     ;
                lut[d][prev][0].Values[6] = - m0 * m1 * m2          ;
                lut[d][prev][0].Values[7] =   m0 * m1               ;

                // All odd-numbered symbols of the PSDU are given an extra 180 degree (pi) rotation
                for (i = 0; i < 8; i++)
                    lut[d][prev][1].Values[i] = - lut[d][prev][0].Values[i];

                lut[d][prev][0].bNewRef = (UCHAR)find(DQPSKEncode, lut[d][prev][0].Values[7]);
                lut[d][prev][1].bNewRef = (UCHAR)find(DQPSKEncode, lut[d][prev][1].Values[7]);
            }
        }
    };
    const static_wrapper<LUT_ELEMENT_CCK11_UCHAR_SPREADED_COMPLEX [256][4][2], _init_cck11_lut> CCK11Lut;

    FINL void _init()
    {
        bEven = DOT11B_PLCP_EVEN_SYMBOL;
    }
public:
    REFERENCE_LOCAL_CONTEXT(TCCK11Encode);
    DEFINE_IPORT(unsigned char, 1);
    DEFINE_OPORT(COMPLEX8, 1);
    STD_TFILTER_CONSTRUCTOR(TCCK11Encode)
        BIND_CONTEXT(CF_DifferentialMap::last_phase, last_phase)
    {
        _init();
    }


	STD_TFILTER_RESET() { _init(); }
	STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        int j;
        while(ipin.check_read())
        {
            unsigned char b = *ipin.peek();
            ipin.pop();

            const COMPLEX8 * pcSrc = CCK11Lut[b][last_phase][bEven].Values;
            for (j = 0; j < CCK11_OUTPUT_COMPLEX_COUNT_PER_UCHAR; j++)
            {
                *opin().append() = pcSrc[j];
                Next()->Process(opin());
            }

            last_phase = CCK11Lut[b][last_phase][bEven].bNewRef;
            bEven = ((bEven + 1) & 0x1);
        }
        return true;
    }
};

#define CCK5_SPREAD_LENGTH          16
#define CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR 16
typedef struct _LUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX
{
    COMPLEX8  Values[CCK5_OUTPUT_COMPLEX_COUNT_PER_UCHAR]; // 4 bits are spreaded into 8 complexes, we process a UCHAR(8bits) a time, produce 16 complexes
    UCHAR         bNewRef; // new ref for next step
}LUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX, *PLUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX;

DEFINE_LOCAL_CONTEXT(TCCK5Encode, CF_DifferentialMap);
template<TFILTER_ARGS>
class TCCK5Encode : public TFilter<TFILTER_PARAMS>
{
    CTX_VAR_RW (uint, last_phase);

    struct _init_cck5_lut
    {
        void operator()(LUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX (&lut)[256][4])
        {
            unsigned int i, d, prev, halfd, d3d2, d1d0;
            COMPLEX8 m1, m0;

            for (d = 0; d < 256; d++)
            for (prev = 0; prev < 4; prev++)
            {
                halfd = d % 16;
                d1d0 = halfd % 4;
                d3d2 = halfd / 4;
                m1 = DQPSKEncode[d1d0];
                // The table for m0 depends on the definition of prev constellation
                m0 = DQPSKEncode[prev];
                for (i = 0; i < 8; i++)
                {
                    lut[d][prev].Values[i] = m0 * m1 * CCK5D3D2[d3d2][i];
                }

                halfd = d / 16;
                d1d0 = halfd % 4;
                d3d2 = halfd / 4;
                m1 = DQPSKEncode[d1d0];
                m0 = lut[d][prev].Values[7];
                for (i = 0; i < 8; i++)
                {
                    lut[d][prev].Values[i+8] = - m0 * m1 * CCK5D3D2[d3d2][i];
                }
                lut[d][prev].bNewRef = (UCHAR)find(DQPSKEncode, lut[d][prev].Values[15]);
            }
        }
    };
    const static_wrapper<LUT_ELEMENT_CCK5_UCHAR_SPREADED_COMPLEX [256][4], _init_cck5_lut> CCK5Lut;

public:
    REFERENCE_LOCAL_CONTEXT(TCCK5Encode);
    DEFINE_IPORT(unsigned char, 1);
    DEFINE_OPORT(COMPLEX8, 1);
    STD_TFILTER_CONSTRUCTOR(TCCK5Encode)
        BIND_CONTEXT(CF_DifferentialMap::last_phase, last_phase)
    {
    }

	STD_TFILTER_RESET() { }
	STD_TFILTER_FLUSH() { }

    BOOL_FUNC_PROCESS(ipin)
    {
        int j;
        while(ipin.check_read())
        {
            unsigned char b = *ipin.peek();
            ipin.pop();

            const COMPLEX8 * pcSrc = CCK5Lut[b][last_phase].Values;
            for (j = 0; j < CCK5_SPREAD_LENGTH; j++)
            {
                *opin().append() = pcSrc[j];
                Next()->Process(opin());
            }

            last_phase = CCK5Lut[b][last_phase].bNewRef;
        }
        return true;
    }
};
