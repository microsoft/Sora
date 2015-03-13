#ifndef BB_LUT_H
#define BB_LUT_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <soratypes.h>

#define IFFT_LUT_FACTOR16 335

#define IFFT_MAG_SHIFT 0
#define IFFT_MAG (0x1 << IFFT_MAG_SHIFT)

#ifndef STATIC_LUT
void GenerateAllLUT();
#endif

/*
    CONV_ENCODER_1_2 
        index range: 2^14
            bRef    6 bits
            bInput  8 bits
        output type:
            1 unsigned short
        prototype:
            unsigned short * CONV_ENCODER_1_2(unsigned short usInput);
        size: 32K
*/

#define CONV_ENCODER_1_2_INPUT_SIZE (0x1 << 14)
#define CONV_ENCODER_1_2_UNIT_SIZE (1)
#define CONV_ENCODER_1_2_LUT_SIZE \
    (CONV_ENCODER_1_2_INPUT_SIZE * CONV_ENCODER_1_2_UNIT_SIZE)
extern const unsigned short CONV_ENCODER_1_2_LUT[CONV_ENCODER_1_2_LUT_SIZE];
#define CONV_ENCODER_1_2(usInput) (CONV_ENCODER_1_2_LUT \
        + ((usInput) * CONV_ENCODER_1_2_UNIT_SIZE))

#ifndef STATIC_LUT
void CONV_ENCODER_1_2_generate();
void CONV_ENCODER_1_2_set(unsigned short usInput, unsigned short * pusOutput);
#endif

/*
    CONV_ENCODER_3_4
        input range: 2^12
            bInput  6 bits
            bRef    6 bits
        output type:
            1 unsigned char
        prototype:
            unsigned short * CONV_ENCODER_3_4(unsigned short usInput);
        size: 4K
*/

#define CONV_ENCODER_3_4_INPUT_SIZE (0x1 << 12)
#define CONV_ENCODER_3_4_UNIT_SIZE (1)
#define CONV_ENCODER_3_4_LUT_SIZE \
    (CONV_ENCODER_3_4_INPUT_SIZE * CONV_ENCODER_3_4_UNIT_SIZE)
extern const unsigned char CONV_ENCODER_3_4_LUT[CONV_ENCODER_3_4_LUT_SIZE];
#define CONV_ENCODER_3_4(usInput) (CONV_ENCODER_3_4_LUT \
        + ((usInput) * CONV_ENCODER_3_4_UNIT_SIZE))

#ifndef STATIC_LUT
void CONV_ENCODER_3_4_generate();
void CONV_ENCODER_3_4_set(unsigned short usInput, unsigned char * pbOutput);
#endif

/*
    CONV_ENCODER_2_3
        input range: 2^14
            bInput  8 bits
            bRef    6 bits
        output type:
            1 unsigned short (12 bits)
        prototype:
            unsigned short * CONV_ENCODER_2_3(unsigned short usInput);
        size: 32K
*/

#define CONV_ENCODER_2_3_INPUT_SIZE (0x1 << 14)
#define CONV_ENCODER_2_3_UNIT_SIZE (1)
#define CONV_ENCODER_2_3_LUT_SIZE \
    (CONV_ENCODER_2_3_INPUT_SIZE * CONV_ENCODER_2_3_UNIT_SIZE)
extern const unsigned short CONV_ENCODER_2_3_LUT[CONV_ENCODER_2_3_LUT_SIZE];
#define CONV_ENCODER_2_3(usInput) (CONV_ENCODER_2_3_LUT \
        + ((usInput) * CONV_ENCODER_2_3_UNIT_SIZE))

#ifndef STATIC_LUT
void CONV_ENCODER_2_3_generate();
void CONV_ENCODER_2_3_set(unsigned short usInput, unsigned short * pusOutput);
#endif

/*
    INTERLEAVE_6M
        index range:    6 * 2^8
            position    0-5
            bInput      8 bits
        output type:
            3 unsigned short
        prototype:
            unsigned short * INTERLEAVE_6M(unsigned short usInput);
        size: 9K
*/

#define INTERLEAVE_6M_LUT_INPUT_SIZE (6 * (0x1 << 8))
#define INTERLEAVE_6M_LUT_UNIT_SIZE (6)
#define INTERLEAVE_6M_LUT_SIZE \
    (INTERLEAVE_6M_LUT_INPUT_SIZE * INTERLEAVE_6M_LUT_UNIT_SIZE)
extern const unsigned short INTERLEAVE_6M_LUT[INTERLEAVE_6M_LUT_SIZE];
#define INTERLEAVE_6M(usInput) (INTERLEAVE_6M_LUT \
        + ((usInput) * INTERLEAVE_6M_LUT_UNIT_SIZE))

#ifndef STATIC_LUT
void INTERLEAVE_6M_generate();
void INTERLEAVE_6M_set(unsigned short usInput, unsigned short * pusOutput);
#endif

/*
    INTERLEAVE_12M
        index range:    12 * 2^8
            position    0-12
            bInput      8 bits
        output type:
            3 unsigned int
        prototype:
            unsigned int * INTERLEAVE_12M(unsigned short usInput);
        size: 36KB
*/

#define INTERLEAVE_12M_LUT_INPUT_SIZE (12 * (0x1 << 8))
#define INTERLEAVE_12M_LUT_UNIT_SIZE (3)
#define INTERLEAVE_12M_LUT_SIZE \
    (INTERLEAVE_12M_LUT_INPUT_SIZE * INTERLEAVE_12M_LUT_UNIT_SIZE)
extern const unsigned int INTERLEAVE_12M_LUT[INTERLEAVE_12M_LUT_SIZE];
#define INTERLEAVE_12M(usInput) (INTERLEAVE_12M_LUT \
        + ((usInput) * INTERLEAVE_12M_LUT_UNIT_SIZE))

#ifndef STATIC_LUT
void INTERLEAVE_12M_generate();
void INTERLEAVE_12M_set(unsigned short usInput, unsigned int * puiOutput);
#endif

/*
    INTERLEAVE_24M
        index range:    24 * 2^8
            position    0-23
            bInput      8 bits
        output type:
            6 unsigned int
        prototype:
            unsigned int * INTERLEAVE_24M(unsigned short usInput);
        size: 144KB
*/

#define INTERLEAVE_24M_LUT_INPUT_SIZE (24 * (0x1 << 8))
#define INTERLEAVE_24M_LUT_UNIT_SIZE (6)
#define INTERLEAVE_24M_LUT_SIZE \
    (INTERLEAVE_24M_LUT_INPUT_SIZE * INTERLEAVE_24M_LUT_UNIT_SIZE)
extern const unsigned int INTERLEAVE_24M_LUT[INTERLEAVE_24M_LUT_SIZE];
#define INTERLEAVE_24M(usInput) (INTERLEAVE_24M_LUT \
        + ((usInput) * INTERLEAVE_24M_LUT_UNIT_SIZE))

#ifndef STATIC_LUT
void INTERLEAVE_24M_generate();
void INTERLEAVE_24M_set(unsigned short usInput, unsigned int * puiOutput);
#endif

/*
    INTERLEAVE_48M
        index range:    36 * 2^8
            position    0-23
            bInput      8 bits
        output type:
            9 unsigned int
        prototype:
            unsigned int * INTERLEAVE_48M(unsigned short usInput);
        size: 324KB
*/

#define INTERLEAVE_48M_LUT_INPUT_SIZE (36 * (0x1 << 8))
#define INTERLEAVE_48M_LUT_UNIT_SIZE (9)
#define INTERLEAVE_48M_LUT_SIZE \
    (INTERLEAVE_48M_LUT_INPUT_SIZE * INTERLEAVE_48M_LUT_UNIT_SIZE)
extern const unsigned int INTERLEAVE_48M_LUT[INTERLEAVE_48M_LUT_SIZE];
#define INTERLEAVE_48M(usInput) (INTERLEAVE_48M_LUT \
        + ((usInput) * INTERLEAVE_48M_LUT_UNIT_SIZE))

#ifndef STATIC_LUT
void INTERLEAVE_48M_generate();
void INTERLEAVE_48M_set(unsigned short usInput, unsigned int * puiOutput);
#endif

/*
    SCRAMBLE_11A
        index range:    2^7
            bReg        7 bits
        output type:
            unsigned char
        prototype:
            unsigned char * SCRAMBLE_11A(unsigned char ubInput);
        size: 128B
*/

#define SCRAMBLE_11A_LUT_INPUT_SIZE (0x1 << 7)
#define SCRAMBLE_11A_LUT_UNIT_SIZE 1
#define SCRAMBLE_11A_LUT_SIZE \
    (SCRAMBLE_11A_LUT_INPUT_SIZE * SCRAMBLE_11A_LUT_UNIT_SIZE)
extern const unsigned char SCRAMBLE_11A_LUT[SCRAMBLE_11A_LUT_SIZE];
#define SCRAMBLE_11A(ubInput) (SCRAMBLE_11A_LUT + (ubInput) * \
        SCRAMBLE_11A_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void SCRAMBLE_11A_generate();
void SCRAMBLE_11A_set(unsigned char ubInput, unsigned char * pbOutput);
#endif

/*
    PREAMBLE_11A
        input range: 1 (nothing)
        output type:
            320 COMPLEX8
        prototype:
            COMPLEX8 * PREAMBLE_11A();
        size: 640B
*/           

#define PREAMBLE_11A_LUT_INPUT_SIZE 1
#define PREAMBLE_11A_LUT_UNIT_SIZE (4 * 80)
#define PREAMBLE_11A_LUT_SIZE \
    (PREAMBLE_11A_LUT_INPUT_SIZE * PREAMBLE_11A_LUT_UNIT_SIZE)
extern const A16 COMPLEX8 PREAMBLE_11A_LUT[PREAMBLE_11A_LUT_SIZE];
#define PREAMBLE_11A() (PREAMBLE_11A_LUT)

#ifndef STATIC_LUT
void PREAMBLE_11A_generate();
void PREAMBLE_11A_set(COMPLEX8 * pcOutput);
#endif

/*
    PREAMBLE16_11A
        input range: 1 (nothing)
        output type:
            320 COMPLEX16
        prototype:
            COMPLEX16 * PREAMBLE16_11A();
        size: 1280B
*/           

#define PREAMBLE16_11A_LUT_INPUT_SIZE 1
#define PREAMBLE16_11A_LUT_UNIT_SIZE (4 * 80)
#define PREAMBLE16_11A_LUT_SIZE \
    (PREAMBLE16_11A_LUT_INPUT_SIZE * PREAMBLE16_11A_LUT_UNIT_SIZE)
extern const A16 COMPLEX16 PREAMBLE16_11A_LUT[PREAMBLE16_11A_LUT_SIZE];
#define PREAMBLE16_11A() (PREAMBLE16_11A_LUT)

#ifndef STATIC_LUT
void PREAMBLE16_11A_generate();
void PREAMBLE16_11A_set(COMPLEX16 * pcOutput);
#endif

/* 
    PREAMBLE40_11A
        input range: 1 (nothing)
        output type:
            640 COMPLEX16
        prototype:
            COMPLEX16 * PREAMBLE40_11A();
        SIZE: 2360B
*/

#define PREAMBLE40_11A_LUT_INPUT_SIZE 1
#define PREAMBLE40_11A_LUT_UNIT_SIZE 640
#define PREAMBLE40_11A_LUT_SIZE \
    (PREAMBLE40_11A_LUT_INPUT_SIZE * PREAMBLE40_11A_LUT_UNIT_SIZE)
extern const A16 COMPLEX16 PREAMBLE40_11A_LUT[PREAMBLE40_11A_LUT_SIZE];
#define PREAMBLE40_11A() (PREAMBLE40_11A_LUT)

#ifndef STATIC_LUT
void PREAMBLE40_11A_generate();
void PREAMBLE40_11A_set(COMPLEX16 * pcOutput);
#endif

/*
    ATAN64
        input range: 1024
        output type:
            1 unsigned int
        prototype:
            unsigned short * ATAN64();
        size: 2048B
*/

#define ATAN64_LUT_INPUT_SIZE (1024 * 2)
#define ATAN64_LUT_UNIT_SIZE 1
#define ATAN64_LUT_SIZE \
    (ATAN64_LUT_INPUT_SIZE * ATAN64_LUT_UNIT_SIZE)
extern const unsigned short ATAN64_LUT[ATAN64_LUT_SIZE];
#define ATAN64(uiInput) (ATAN64_LUT + (uiInput) * \
        ATAN64_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void ATAN64_generate();
void ATAN64_set(unsigned int uiInput, unsigned short * puiOutput);
#endif

/*
    SIN0xFFFF
        input range: 65536
        output type:
            1 short:
        prototype
            short * SIN0xFFFF(usInput);
        size: 128kB
*/

#define SIN0xFFFF_LUT_INPUT_SIZE 65536
#define SIN0xFFFF_LUT_UNIT_SIZE 1
#define SIN0xFFFF_LUT_SIZE \
    (SIN0xFFFF_LUT_INPUT_SIZE * SIN0xFFFF_LUT_UNIT_SIZE)
extern const short SIN0xFFFF_LUT[SIN0xFFFF_LUT_SIZE];
#define SIN0xFFFF(usInput) (SIN0xFFFF_LUT + (usInput) * \
        SIN0xFFFF_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void SIN0xFFFF_generate();
void SIN0xFFFF_set(unsigned short usInput, short * psOutput);
#endif

/*
    COS0xFFFF
        input range: 65536
        output type:
            1 short
        prototype:
            short * COS0xFFFF(usInput);
        size: 128kB
*/

#define COS0xFFFF_LUT_INPUT_SIZE 65536
#define COS0xFFFF_LUT_UNIT_SIZE 1
#define COS0xFFFF_LUT_SIZE \
    (COS0xFFFF_LUT_INPUT_SIZE * COS0xFFFF_LUT_UNIT_SIZE)
extern const short COS0xFFFF_LUT[COS0xFFFF_LUT_SIZE];
#define COS0xFFFF(usInput) (COS0xFFFF_LUT + (usInput) * \
        COS0xFFFF_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void COS0xFFFF_generate();
void COS0xFFFF_set(unsigned short usInput, short * psOutput);
#endif

/*
    ARG
        input range: 2^16
        output type:
            1 short
        prototype:
            short * ARG(usInput);
        size 256kB
*/

#define ARG_LUT_INPUT_SIZE 65536
#define ARG_LUT_UNIT_SIZE 1
#define ARG_LUT_SIZE \
    (ARG_LUT_INPUT_SIZE * ARG_LUT_UNIT_SIZE)
extern const short ARG_LUT[ARG_LUT_SIZE];
#define ARG(usInput) (ARG_LUT + (usInput) * \
        ARG_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void ARG_generate();
void ARG_set(unsigned short usInput, short * psOutput);
#endif

/*
    CRC32
        input range: 256
        output type:
            1 unsigned int
        prototype:
            unsigned int * CRC32(bInput);
        size 1kB
*/

#ifndef STATIC_LUT
void CRC32_generate();
void CRC32_set(unsigned char bInput, unsigned int * puiOutput);
#endif

/*
    PILOTSGN
        input range: 127
        output type:
            1 byte
        prototype:
            char * PILOTSGN(bInput);
        size 127B
*/

#define PILOTSGN_LUT_INPUT_SIZE 127
#define PILOTSGN_LUT_UNIT_SIZE 1
#define PILOTSGN_LUT_SIZE \
    (PILOTSGN_LUT_INPUT_SIZE * PILOTSGN_LUT_UNIT_SIZE)
extern const char PILOTSGN_LUT[PILOTSGN_LUT_SIZE];
#define PILOTSGN(bInput) (PILOTSGN_LUT + (bInput) * \
        PILOTSGN_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void PILOTSGN_generate();
void PILOTSGN_set(char bInput, char * pbOutput);
#endif

/*
    MAPA_BPSK
        input range: 256
        output type:
            8 COMPLEX16
        prototype:
            COMPLEX16 * MAPA_BPSK(bInput);
        size 8kB
*/

#define MAPA_BPSK_LUT_INPUT_SIZE 256
#define MAPA_BPSK_LUT_UNIT_SIZE 8
#define MAPA_BPSK_LUT_SIZE \
    (MAPA_BPSK_LUT_INPUT_SIZE * MAPA_BPSK_LUT_UNIT_SIZE)
extern A16 const COMPLEX16 MAPA_BPSK_LUT[MAPA_BPSK_LUT_SIZE];
#define MAPA_BPSK(bInput) (MAPA_BPSK_LUT + (bInput) * \
        MAPA_BPSK_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void MAPA_BPSK_generate();
void MAPA_BPSK_set(unsigned char bInput, COMPLEX16 * pcOutput);
#endif

/*
    MAPA_QPSK
        input range: 256
        output type:
            4 COMPLEX16
        prototype:
            COMPLEX16 * MAPA_QPSK(bInput);
        size 4kB
*/

#define MAPA_QPSK_LUT_INPUT_SIZE 256
#define MAPA_QPSK_LUT_UNIT_SIZE 4
#define MAPA_QPSK_LUT_SIZE \
    (MAPA_QPSK_LUT_INPUT_SIZE * MAPA_QPSK_LUT_UNIT_SIZE)
extern const COMPLEX16 MAPA_QPSK_LUT[MAPA_QPSK_LUT_SIZE];
#define MAPA_QPSK(bInput) (MAPA_QPSK_LUT + (bInput) * \
        MAPA_QPSK_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void MAPA_QPSK_generate();
void MAPA_QPSK_set(unsigned char bInput, COMPLEX16 * pcOutput);
#endif

/*
    MAPA_16QAM
        input range: 256
        output type:
            2 COMPLEX16
        prototype:
            COMPLEX16 * MAPA_16QAM(bInput);
        size 2kB
*/

#define MAPA_16QAM_LUT_INPUT_SIZE 256
#define MAPA_16QAM_LUT_UNIT_SIZE 2
#define MAPA_16QAM_LUT_SIZE \
    (MAPA_16QAM_LUT_INPUT_SIZE * MAPA_16QAM_LUT_UNIT_SIZE)
extern const COMPLEX16 MAPA_16QAM_LUT[MAPA_16QAM_LUT_SIZE];
#define MAPA_16QAM(bInput) (MAPA_16QAM_LUT + (bInput) * \
        MAPA_16QAM_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void MAPA_16QAM_generate();
void MAPA_16QAM_set(unsigned char bInput, COMPLEX16 * pcOutput);
#endif

/*
    MAPA_64QAM
        input range: 4096
        output type:
            2 COMPLEX16
        prototype:
            COMPLEX16 * MAPA_64QAM(usInput);
        size 32kB
*/

#define MAPA_64QAM_LUT_INPUT_SIZE 4096
#define MAPA_64QAM_LUT_UNIT_SIZE 2
#define MAPA_64QAM_LUT_SIZE \
    (MAPA_64QAM_LUT_INPUT_SIZE * MAPA_64QAM_LUT_UNIT_SIZE)
extern const COMPLEX16 MAPA_64QAM_LUT[MAPA_64QAM_LUT_SIZE];
#define MAPA_64QAM(usInput) (MAPA_64QAM_LUT + (usInput) * \
        MAPA_64QAM_LUT_UNIT_SIZE)

#ifndef STATIC_LUT
void MAPA_64QAM_generate();
void MAPA_64QAM_set(unsigned short usInput, COMPLEX16 * pcOutput);
#endif

#ifdef __cplusplus
}
#endif

#endif // BB_LUT_H
