#pragma once

#include <stdio.h>
#include "const.h"

SORA_EXTERN_C
FINL unsigned int FindLeftMostSetPosition(int Set)
{
    static const char FindLeftMostSetPositionInChar[256] = {
            0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

    unsigned int _Mask;
    unsigned int _Offset = 16;
    if ((_Mask = Set >> 16) == 0)
    {
        _Offset = 0;
        _Mask = Set;
    }
    if (_Mask >> 8)
    {
        _Offset += 8;
    }
    return FindLeftMostSetPositionInChar[Set >> _Offset] + _Offset;
}

SORA_EXTERN_C
FINL int ceil_pad(int x, int mod)
{
    int rem = x % mod;
    return rem ? mod - rem : 0;
}

SORA_EXTERN_C
FINL int ceil_div(int x, int mod)
{
    return (x + mod - 1) / mod;
}

SORA_EXTERN_C
FINL FILE *fopen_try(const char * _Filename, const char * _Mode)
{
    FILE *fin;
    if (!_Filename) return NULL;
#pragma warning (push)
#pragma warning (disable:4996)
    fin = fopen(_Filename, _Mode);
#pragma warning (pop)
    return fin;
}

SORA_EXTERN_C
FINL int fclose_try(FILE * _File)
{
    if (!_File) return 0;
    return fclose(_File);
}

SORA_EXTERN_C
FINL unsigned int BinaryToGray(unsigned int num)
{
    return (num >> 1) ^ num;
}

SORA_EXTERN_C
FINL unsigned int BitReverseBinaryToGray(unsigned int rnum)
{
    return (rnum << 1) ^ rnum;
}

SORA_EXTERN_C
FINL unsigned int GrayToBinary(unsigned int num)
{
    unsigned int numBits = 8 * sizeof(num);
    unsigned int shift;
    for (shift = 1; shift < numBits; shift *= 2)
    {
        num ^= num >> shift;
    }
    return num;
}

SORA_EXTERN_C
FINL unsigned int BitReverse(unsigned int v)
{
    // swap odd and even bits
    v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
    // swap consecutive pairs
    v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
    // swap nibbles ... 
    v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
    // swap bytes
    v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
    // swap 2-byte long pairs
    v = ( v >> 16             ) | ( v               << 16);
    return v;
}

SORA_EXTERN_C
FINL unsigned int BitReverseN(unsigned int v, int nbits)
{
    v = BitReverse(v);
    return v >> (sizeof(v) * 8 - nbits);
}

SORA_EXTERN_C
FINL int LoadDumpFile(const char *fileName, unsigned char *buf, size_t maxSize)
{
    int len;
#pragma warning (push)
#pragma warning (disable:4996)
    FILE *fin = fopen(fileName, "rb");
#pragma warning (pop)
    if (!fin) return -1;

    len = fread(buf, sizeof(char), maxSize, fin);
    fclose(fin);
    return len;
}
