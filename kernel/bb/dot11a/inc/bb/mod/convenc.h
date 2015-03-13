#ifndef BB_MOD_CONVENC_H
#define BB_MOD_CONVENC_H

#include "lut.h"

__forceinline
void ConvEncode_1_2(char *pbInput, char *pbOutput, unsigned int uiSize, unsigned char& bConvEncoderReg)
{
    unsigned int i;
    unsigned char c;
    for (i = 0; i < uiSize; i++)
    {
        c = (unsigned char)(pbInput[i]);

        *(unsigned short *)(pbOutput) = 
            *(CONV_ENCODER_1_2( (bConvEncoderReg << 8) | c ));

        /*
        KdPrint(("%02x %02x : %02x %02x \n", 
        bConvEncoderReg,
        c,
        (unsigned char)(pbOutput[0]), 
        (unsigned char)(pbOutput[1])));
        */

        bConvEncoderReg = c >> 2;
        pbOutput += 2;
    }
}

__forceinline
void ConvEncode_2_3(char *pbInput, char *pbOutput, unsigned int uiSize, unsigned char& bConvEncoderReg)
{
    unsigned int i;
    unsigned char c1, c2;
    unsigned short b1, b2;
    unsigned short l1, l2;

    for (i = 0; i < uiSize; i += 2)
    {
        c1 = (unsigned char)(pbInput[i]);
        c2 = (unsigned char)(pbInput[i+1]);

        b1 = (c1 << 6) | (bConvEncoderReg);
        b2 = (c2 << 6) | (c1 >> 2);

        l1 = *CONV_ENCODER_2_3(b1);
        l2 = *CONV_ENCODER_2_3(b2);

        pbOutput[0] = l1 & 0xff;
        pbOutput[1] = (l1 >> 8) | (l2 << 4);
        pbOutput[2] = (l2 >> 4);

        pbOutput += 3;

        bConvEncoderReg = (c2 >> 2);
    }
}

__forceinline
void ConvEncode_3_4(char *pbInput, char *pbOutput, unsigned int uiSize, unsigned char& bConvEncoderReg)
{
    unsigned int i;
    unsigned char c1, c2, c3;
    unsigned short b1, b2, b3, b4;

    for (i = 0; i < uiSize; i += 3)
    {
        c1 = (unsigned char)(pbInput[i]);
        c2 = (unsigned char)(pbInput[i+1]);
        c3 = (unsigned char)(pbInput[i+2]);

        b1 = ((c1 & 0x3F) << 6) | bConvEncoderReg;
        b2 = ((c2 & 0xF) << 8) | c1;
        b3 = ((c3 & 0x3) << 10) | (c2 << 2) | (c1 >> 6);
        b4 = (c3 << 4) | (c2 >> 4);

        bConvEncoderReg = (c3 >> 2);

        pbOutput[0] = *(CONV_ENCODER_3_4(b1));
        pbOutput[1] = *(CONV_ENCODER_3_4(b2));
        pbOutput[2] = *(CONV_ENCODER_3_4(b3));
        pbOutput[3] = *(CONV_ENCODER_3_4(b4));
        pbOutput += 4;
    }
}

__forceinline
void ConvEncode_3_4_9MSpecial1(char *pbInput, char *pbOutput, unsigned char& bConvEncoderReg)
{
    unsigned char c1, c2, c3, c4, c5;
    unsigned short b1, b2, b3, b4, b5, b6;

    c1 = (unsigned char)(pbInput[0]);
    c2 = (unsigned char)(pbInput[1]);
    c3 = (unsigned char)(pbInput[2]);
    c4 = (unsigned char)(pbInput[3]);
    c5 = (unsigned char)(pbInput[4]);

    b1 = ((c1 & 0x3F) << 6) | bConvEncoderReg;
    b2 = ((c2 & 0xF) << 8) | c1;
    b3 = ((c3 & 0x3) << 10) | (c2 << 2) | (c1 >> 6);
    b4 = (c3 << 4) | (c2 >> 4);
    b5 = ((c4 & 0x3F) << 6) | (c3 >> 2);
    b6 = ((c5 & 0xF) << 8) | c4;

    pbOutput[0] = *(CONV_ENCODER_3_4(b1));
    pbOutput[1] = *(CONV_ENCODER_3_4(b2));
    pbOutput[2] = *(CONV_ENCODER_3_4(b3));
    pbOutput[3] = *(CONV_ENCODER_3_4(b4));
    pbOutput[4] = *(CONV_ENCODER_3_4(b5));
    pbOutput[5] = *(CONV_ENCODER_3_4(b6));

    bConvEncoderReg = ((c5 & 0xF) << 2) | ((c4 >> 6) & 0x3);
}

__forceinline
void ConvEncode_3_4_9MSpecial2(char *pbInput, char *pbOutput, unsigned char& bConvEncoderReg)
{
    unsigned char c1, c2, c3, c4, c5;
    unsigned short b1, b2, b3, b4, b5, b6;

    c1 = (unsigned char)(pbInput[0]);
    c2 = (unsigned char)(pbInput[1]);
    c3 = (unsigned char)(pbInput[2]);
    c4 = (unsigned char)(pbInput[3]);
    c5 = (unsigned char)(pbInput[4]);

    b1 = ((c2 & 0x3) << 10) | ((c1 & 0xF0) << 2) | bConvEncoderReg;
    b2 = (c2 << 4) | (c1 >> 4);
    b3 = ((c3 & 0x3F) << 6) | (c2 >> 2);
    b4 = ((c4 & 0xF) << 8) | c3;
    b5 = ((c5 & 0x3) << 10) | (c4 << 2) | (c3 >> 6);
    b6 = (c5 << 4) | (c4 >> 4);

    pbOutput[0] = *(CONV_ENCODER_3_4(b1));
    pbOutput[1] = *(CONV_ENCODER_3_4(b2));
    pbOutput[2] = *(CONV_ENCODER_3_4(b3));
    pbOutput[3] = *(CONV_ENCODER_3_4(b4));
    pbOutput[4] = *(CONV_ENCODER_3_4(b5));
    pbOutput[5] = *(CONV_ENCODER_3_4(b6));

    bConvEncoderReg = (c5 >> 2);
}

__forceinline
void ConvEncodeReset(unsigned char& bConvEncoderReg)
{
    bConvEncoderReg = 0;
}

#endif//BB_MOD_CONVENC_H
