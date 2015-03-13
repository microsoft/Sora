#pragma once

#include "vector128.h"
#include "bb/mod/vb.h"
#include "bb/mod/lbuf1.h"

#define VB1_DCSIZE 48
#define VB1_DCCOUNT 128
#define VB2_DCSIZE 96
#define VB2_DCCOUNT 128
#define VB3_DCSIZE 192
#define VB3_DCCOUNT 128
#define VB4_DCSIZE 288
#define VB4_DCCOUNT 128

typedef struct __BB11A_RX_FIFOS
{
    union
    {
        typedef VB<VB1_DCSIZE, VB1_DCCOUNT> VB1;
        typedef VB<VB2_DCSIZE, VB2_DCCOUNT> VB2;
        typedef VB<VB3_DCSIZE, VB3_DCCOUNT> VB3;
        typedef VB<VB4_DCSIZE, VB4_DCCOUNT> VB4;
        VB1 vb1;
        VB2 vb2;
        VB3 vb3;
        VB4 vb4;
    };

    SoraLinearFifo1<vcs> g11a_lbSync;
} BB11A_RX_FIFOS, *PBB11A_RX_FIFOS;
