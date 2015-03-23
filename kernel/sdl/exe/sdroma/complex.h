#ifndef COMPLEX_H
#define COMPLEX_H

struct Complex
{
    double re, im;
};

inline void
cmod ( double &res, const Complex & c) 
{
   res = c.im * c.im + c.re * c.re;
   res = sqrt (res);
}

inline void 
cadd(Complex & res, const Complex & c1, const Complex & c2)
{
    res.re = c1.re + c2.re;
    res.im = c1.im + c2.im;
}

inline void 
cadd(Complex & src, const Complex & dest)
{
    cadd(src, src, dest);
}

inline void 
csub(Complex & res, const Complex & c1, const Complex & c2)
{
    res.re = c1.re - c2.re;
    res.im = c1.im - c2.im;
}

inline void 
csub(Complex & src, const Complex & dest)
{
    csub(src, src, dest);
}

inline void 
cmul(Complex & res, const Complex & c1, const Complex & c2)
{
    double re = c1.re * c2.re - c1.im * c2.im;
    double im = c1.im * c2.re + c1.re * c2.im;

    res.re = re;
    res.im = im;
}

inline void 
cmul(Complex & src, const Complex & dest)
{
    cmul(src, src, dest);
}

inline void 
cmulf(Complex & res, const Complex & c1, const Complex & c2)
{
    double re = c1.re * c2.re + c1.im * c2.im;
    double im = c1.im * c2.re - c1.re * c2.im;

    res.re = re;
    res.im = im;
}

inline void 
cmulf(Complex & src, const Complex & dest)
{
    cmul(src, src, dest);
}

inline void 
cdiv(Complex & res, const Complex & c1, const Complex & c2)
{
    double str = c2.re * c2.re + c2.im * c2.im;
    double re = (c1.re * c2.re + c1.im * c2.im) / str;
    double im = (c1.im * c2.re - c1.re * c2.im) / str;

    res.re = re;
    res.im = im;
}

inline void
cdiv(Complex & res, const Complex & c1, const double d)
{
    Complex t = {d, 0};
    cdiv(res, c1, t);
}

inline void 
cdiv(Complex & src, const Complex & dest)
{
    cdiv(src, src, dest);
}

inline void
cdiv(Complex & src, const double d)
{
    Complex t = {d, 0};
    cdiv(src, t);
}

inline void 
cflip(Complex & res, const Complex & c1)
{
    res.re = c1.re;
    res.im = -c1.im;
}

inline void 
cflip(Complex & src)
{
    src.im = -src.im;
}

inline void 
cclr(Complex & src)
{
    src.re = src.im = 0;
}

#endif//COMPLEX_H
