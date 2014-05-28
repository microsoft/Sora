#pragma once
/*
Copyright (c) Microsoft Corporation

Abstract:   Class definition for software resample from 44MHz to 40MHz sampling rate, 
            and vice versus

*/
#include <bb/bba.h>
#include "vector128.h"

class CDown44MTo40M
{
   MEM_ALIGN(16) COMPLEX16 OutStream[84];
   COMPLEX32        LastIQRight;
   int              LastIQIndex;

   int              start;
   int              length;

private:
    void Move();
    
public :

    COMPLEX16 * GetOutStream(int nIQs);
    CDown44MTo40M();
    void Resample(const SignalBlock& block);
};

class CUp40MTo44M
{
public:
    static ULONG Resample(
        OUT COMPLEX8 *pIQBuf44M, 
        IN ULONG n44MBufIQNum, 
        IN COMPLEX8 *pIQBuf40M, 
        IN ULONG n40BufIQNum);
};

