#ifndef _TYPES_HELPER_H
#define _TYPES_HELPER_H

#include <math.h>
#include "OscilloTypes.h"

inline void EncodeOscilloData(BYTE *pData, const SORA_OSCILLO_DATA &data)
{
    if (pData == NULL) return;

    *((int *)pData) = (int)data.DataType;
    pData += sizeof(int);
    
    switch (data.DataType)
    {
    case DATA_TYPE_COMMAND:
        *((int *)pData) = (int)data.cmd.CmdType;
        pData += sizeof(int);
        
        *((int *)pData) = (int)data.cmd.nCmdParam;
        pData += sizeof(int);
        break;
    case DATA_TYPE_COMPLEX8:
        *pData = data.comp8.re;
        pData += sizeof(BYTE);
        
        *pData = data.comp8.im;
        pData += sizeof(BYTE);
        break;
    case DATA_TYPE_COMPLEX16:
        *((short *)pData) = data.comp16.re;
        pData += sizeof(short);
        
        *((short *)pData) = data.comp16.im;
        pData += sizeof(short);
        break;
    case DATA_TYPE_COMPLEX32:
        *((int *)pData) = data.comp32.re;
        pData += sizeof(int);
        
        *((int *)pData) = data.comp32.im;
        pData += sizeof(int);
        break;
    case DATA_TYPE_REAL:
        *((double *)pData) = data.dbl;
        pData += sizeof(double);
        break;
    case DATA_TYPE_BYTE:
        *pData = data.bt;
        pData += sizeof(BYTE);
        break;
    case DATA_TYPE_TEXT:
        *((unsigned int *)pData) = (unsigned int)data.text;
        pData += sizeof(unsigned int);
        break;
    default:
        break;
    }

    *((int *)pData) = (int)data.nTime;
}

inline void DecodeOscilloData(BYTE *pData, SORA_OSCILLO_DATA &data)
{
    if (pData == NULL) return;

    data.DataType = (SORA_OSCILLO_DATA_TYPE)(*((int *)pData));
    pData += sizeof(int);
    
    switch (data.DataType)
    {
    case DATA_TYPE_COMMAND:
        data.cmd.CmdType = (SORA_OSCILLO_COMMAND_TYPE)(*((int *)pData));
        pData += sizeof(int);
        
        data.cmd.nCmdParam = *((int *)pData);
        pData += sizeof(int);
        break;
    case DATA_TYPE_COMPLEX8:
        data.comp8.re = *pData;
        pData += sizeof(BYTE);
        
        data.comp8.im = *pData;
        pData += sizeof(BYTE);
        break;
    case DATA_TYPE_COMPLEX16:
        data.comp16.re = *((short *)pData);
        pData += sizeof(short);
        
        data.comp16.im = *((short *)pData);
        pData += sizeof(short);
        break;
    case DATA_TYPE_COMPLEX32:
        data.comp32.re = *((int *)pData);
        pData += sizeof(int);
        
        data.comp32.im = *((int *)pData);
        pData += sizeof(int);
        break;
    case DATA_TYPE_REAL:
        data.dbl = *((double *)pData);
        pData += sizeof(double);
        break;
    case DATA_TYPE_BYTE:
        data.bt = *pData;
        pData += sizeof(BYTE);
        break;
    case DATA_TYPE_TEXT:
        data.text = (char *)(*((unsigned int *)pData));
        pData += sizeof(unsigned int);
        break;
    default:
        break;
    }

    data.nTime = *((int *)pData);
}

inline void ConvertComplex16ToReal(SORA_OSCILLO_DATA &symbol)
{
    if (symbol.DataType == DATA_TYPE_COMPLEX16)
    {
        symbol.DataType = DATA_TYPE_REAL;
        double re = (double)symbol.comp16.re;
        double im = (double)symbol.comp16.im;
        symbol.dbl = sqrt(re * re + im * im);
    }
}

inline short NormalizeComplex16(short w, short shift)
{
#ifdef OLD_HARDWARE
    w >>= 2;
#endif

    if (shift > 0)
    {
        w >>= shift;
    }
    else
    {
        w <<= -shift;
    }
    //if (w > 127) w = 127;
    //if (w < -128) w = -128;
    return (short)w;
}

#endif//_TYPES_HELPER_H