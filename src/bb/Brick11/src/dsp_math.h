#pragma once

#include "vector128.h"
#include "complex.h"

#define _USE_MATH_DEFINES
#include "math.h"

struct dsp_math
{
private:
    static const unsigned int sincos_lookup_table_size = 65536;
    COMPLEX16 sincos_lookup_table[sincos_lookup_table_size];

    static const unsigned int atan_lookup_table_size = 4096 + 1;
    short atan_lookup_table[atan_lookup_table_size];

#define dsp_2pi  65535
#define dsp_pi   32767
#define dsp_pi_2 16384
#define dsp_pi_4 8192

    // fixed point mapping
    // 0 => 0
    // PI => 32767
    // 2PI => 65535
    // -PI => -32768

    dsp_math()
    {
        generate_sincos_lookup_table();
        generate_atan_lookup_table();
    }

    dsp_math(const dsp_math&); // Forbid default
    dsp_math& operator=(const dsp_math&); // Forbid default

public:
    static const dsp_math& singleton()
    {
        static dsp_math instance;
        return instance;
    }

    __forceinline static char abs(char x)
    {
        char out   = x;
        char temp  = x;
        temp       = temp >> 7;
        out        = out ^ temp;
        out        = out - temp;
        return out;
    }

    __forceinline static short abs(short x)
    {
        short out  = x;
        short temp = x;
        temp       = temp >> 15;
        out        = out ^ temp;
        out        = out - temp;
        return out;
    }

    __forceinline static int abs(int x)
    {
        int out  = x;
        int temp = x;
        temp     = temp >> 31;
        out      = out ^ temp;
        out      = out - temp;
        return out;
    }

    __forceinline static char imax(char x, char y)
    {
        return ( ( (x + y) + dsp_math::abs(x - y) ) >> 1 );
    }

    __forceinline static short imax(short x, short y)
    {
        return ( ( (x + y) + dsp_math::abs(x - y) ) >> 1 );
    }

    __forceinline static int imax(int x, int y)
    {
        return ( ( (x + y) + dsp_math::abs(x - y) ) >> 1 );
    }

    __forceinline COMPLEX16 sincos(short x) const
    {
        return sincos_lookup_table[(unsigned short)x];
    }

    // return the arctangent of x
    // resolution: atan(1/4096)*180/pi = 0.014 degree
    // return value lies in -PI/2 ~~ +PI/2 in radius
    // trick: atan(y/x)=PI/2 - atan(x/y)
    // verify: retvalue * 32767 / PI =?= atan(y/x)
    __forceinline short atan(short x, short y) const
    {
        short absx, absy;
        short srad;
        short sign = (x ^ y) >> 15;
        int idx;

        absx = dsp_math::abs(x);
        absy = dsp_math::abs(y);

#if 1
        short tsign = ((absx - absy) >> 15);
        int tmax    = absx;
        int tmin    = absy;
        int tsum    = tmax + tmin;
        tmax        = dsp_math::imax(tmax, tmin);
        tmin        = tsum - tmax;

        if (tmax == 0)
        {
            return 0;
        }


        idx = ((tmin << 16) + (tmax >> 1)) / tmax;
        idx >>= 4;

        if (idx < 0 || idx >= atan_lookup_table_size) return 0; // just to prevent index-out-of-range bug
        srad = atan_lookup_table[idx];

        srad = (dsp_pi_2 & tsign) + ( (srad ^ tsign) - tsign );
#else
        int ix = absx;
        int iy = absy;

        if (iy <= ix)
        {
            idx = ((iy << 16) + (ix >> 1)) / ix;
            idx >>= 4;

            srad = atan_lookup_table[idx];
        }
        else // x < y
        {
            idx = ((ix << 16) + (iy >> 1)) / iy;
            idx >>= 4;

            srad = atan_lookup_table[idx];
            srad = dsp_pi_2 - srad; // PI/2-rad
        }
#endif
        srad ^= sign;
        srad -= sign;

        return srad;
    }

    __forceinline short atan(int x, int y) const
    {
        short srad;
        short sign = (short)(((x ^ y) >> 31) & 0x0000FFFF);

        int absx = dsp_math::abs(x);
        int absy = dsp_math::abs(y);
        int idx;
#if 1
        short tsign = (short)(((absx - absy) >> 31) & 0x0000FFFF);
        int tmax    = absx;
        int tmin    = absy;
        int tsum    = tmax + tmin;
        tmax        = dsp_math::imax(tmax, tmin);
        tmin        = tsum - tmax;

        __int64 i64x = tmin;
        __int64 i64y = tmax;
        i64x <<= 16;

        if (i64y == 0)
        {
            i64y = 1;
        }
        idx   = (int)((i64x + (i64y >> 1)) / i64y);
        idx >>= 4;

        if (idx < 0 || idx >= atan_lookup_table_size) return 0; // just to prevent index-out-of-range bug
        srad = atan_lookup_table[idx];

        srad = (dsp_pi_2 & tsign) + ( (srad ^ tsign) - tsign );
#else
        int ix = absx;
        int iy = absy;
        if (iy <= ix)
        {
            idx = (iy + (ix >> 17)) / (ix >> 16);
            idx >>= 4;

            srad = atan_lookup_table[idx];
        }
        else // x < y
        {
            idx = (ix + (iy >> 17)) / (iy >> 16);
            idx >>= 4;

            srad = atan_lookup_table[idx];
            srad = dsp_pi_2 - srad; // PI/2-rad
        }
#endif
        srad ^= sign;
        srad -= sign;

        return srad;
    }

private:
    void generate_sincos_lookup_table()
    {
        double drad, dsin, dcos;
        short  ssin, scos;
        for (unsigned int i = 0; i < 65536; i++)
        {
            drad = (double)i * 2.0 * M_PI / 65535.0;
            dcos = cos(drad);
            dsin = sin(drad);

            scos = (short)(dcos * 32767.5);
            ssin = (short)(dsin * 32767.5);

            sincos_lookup_table[i].re = scos;
            sincos_lookup_table[i].im = ssin;
            //printf("[sincos]%d: dcos=%f, dsin=%f, scos=%d, ssin=%d\n", i, dcos, dsin, scos, ssin);
        }
    }

    void generate_atan_lookup_table()
    {
        double dinput, drad;
        short  srad;

        //  input range: 0<=x<=4096
        for (int i = 0; i <= 4096; i++)
        {
            dinput               = (double)i;
            drad                 = ::atan(dinput / 4096.0);
            srad                 = (short)(drad / ( M_PI / 4.0) * dsp_pi_4);
            atan_lookup_table[i] = srad;
            //printf("[atan]%d:    dinput=%f, drad=%f, srad=%d\n", i, dinput, drad, srad);
        }
    }
};
