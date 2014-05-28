#include "bb/bba.h"
#include <bb/mod.h>
#include "arx_vdc.h"

#define TB_DEPTH        36
#define TB_OUTPUT       96

#define NOR_MASK        0x7

void VitDesCRC24(PBB11A_RX_CONTEXT pRxContextA)
{
    VitDesCRC<24, NOR_MASK, TB_DEPTH, TB_OUTPUT>
        (pRxContextA, pRxContextA->rxFifos->vb3);
}
